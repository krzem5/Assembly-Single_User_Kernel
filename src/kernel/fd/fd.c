#include <kernel/fd/fd.h>
#include <kernel/vfs/allocator.h>
#include <kernel/vfs/vfs.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "fd"



static lock_t _fd_lock=LOCK_INIT_STRUCT;
static u64 _fd_bitmap[FD_MAX_COUNT>>6];

fd_data_t* fd_data;
u16 fd_count;



static inline _Bool _is_invalid_fd(fd_t fd){
	return (!fd||fd>FD_MAX_COUNT||(_fd_bitmap[(fd-1)>>6]&(1ull<<((fd-1)&63))));
}



static inline fd_data_t* _get_fd_data(fd_t fd){
	return fd_data+fd-1;
}



static int _node_to_fd(vfs_node_t* node,u8 flags){
	lock_acquire(&_fd_lock);
	if (fd_count>=FD_MAX_COUNT){
		return FD_ERROR_OUT_OF_FDS;
	}
	fd_t out=0;
	while (!_fd_bitmap[out]){
		out++;
	}
	fd_t idx=__builtin_ctzll(_fd_bitmap[out]);
	_fd_bitmap[out]&=_fd_bitmap[out]-1;
	out=(out<<6)|idx;
	fd_data_t* data=fd_data+out;
	data->node_id=node->id;
	data->offset=((flags&FD_FLAG_APPEND)?vfs_get_size(node):0);
	data->flags=flags&(FD_FLAG_READ|FD_FLAG_WRITE);
	lock_release(&_fd_lock);
	return idx+1;
}



void fd_init(void){
	LOG("Initializing file descriptor list...");
	fd_data=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(FD_MAX_COUNT*sizeof(fd_data_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_FD));
}



void fd_clear(void){
	LOG("Clearing file descriptor list...");
	lock_acquire(&_fd_lock);
	fd_count=0;
	for (u16 i=0;i<((FD_MAX_COUNT+63)>>6);i++){
		_fd_bitmap[i]=0xffffffffffffffffull;
	}
	lock_release(&_fd_lock);
}



int fd_open(fd_t root,const char* path,u32 length,u8 flags){
	if (flags&(~(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_APPEND|FD_FLAG_CREATE|FD_FLAG_DIRECTORY))){
		return FD_ERROR_INVALID_FLAGS;
	}
	char buffer[4096];
	if (length>4095){
		return FD_ERROR_INVALID_POINTER;
	}
	memcpy(buffer,path,length);
	lock_acquire(&_fd_lock);
	vfs_node_t* root_node=NULL;
	if (root){
		if (_is_invalid_fd(root)){
			lock_release(&_fd_lock);
			return FD_ERROR_INVALID_FD;
		}
		root_node=vfs_get_by_id(_get_fd_data(root)->node_id);
		if (!root_node){
			lock_release(&_fd_lock);
			return FD_ERROR_NOT_FOUND;
		}
	}
	buffer[length]=0;
	vfs_node_t* node=vfs_get_by_path(root_node,buffer,((flags&FD_FLAG_CREATE)?((flags&FD_FLAG_DIRECTORY)?VFS_NODE_TYPE_DIRECTORY:VFS_NODE_TYPE_FILE):0));
	lock_release(&_fd_lock);
	if (!node){
		return FD_ERROR_NOT_FOUND;
	}
	return _node_to_fd(node,flags);
}



int fd_close(fd_t fd){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	_fd_bitmap[(fd-1)>>6]|=1ull<<((fd-1)&63);
	lock_release(&_fd_lock);
	return 0;
}



int fd_delete(fd_t fd){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	if (node->type==VFS_NODE_TYPE_DIRECTORY&&vfs_get_relative(node,VFS_RELATIVE_FIRST_CHILD)){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_EMPTY;
	}
	_Bool out=vfs_delete(node);
	if (out){
		_fd_bitmap[(fd-1)>>6]|=1ull<<((fd-1)&63);
	}
	lock_release(&_fd_lock);
	return (out?0:FD_ERROR_NOT_EMPTY);
}



s64 fd_read(fd_t fd,void* buffer,u64 count){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	if (!(data->flags&FD_FLAG_READ)){
		lock_release(&_fd_lock);
		return FD_ERROR_UNSUPPORTED_OPERATION;
	}
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	count=vfs_read(node,data->offset,buffer,count);
	data->offset+=count;
	lock_release(&_fd_lock);
	return count;
}



s64 fd_write(fd_t fd,const void* buffer,u64 count){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	if (!(data->flags&FD_FLAG_WRITE)){
		lock_release(&_fd_lock);
		return FD_ERROR_UNSUPPORTED_OPERATION;
	}
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	count=vfs_write(node,data->offset,buffer,count);
	data->offset+=count;
	lock_release(&_fd_lock);
	return count;
}



s64 fd_seek(fd_t fd,u64 offset,u8 flags){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	switch (flags){
		case FD_SEEK_SET:
			data->offset=offset;
			break;
		case FD_SEEK_ADD:
			data->offset+=offset;
			break;
		case FD_SEEK_END:
			vfs_node_t* node=vfs_get_by_id(data->node_id);
			if (!node){
				lock_release(&_fd_lock);
				return FD_ERROR_NOT_FOUND;
			}
			data->offset=vfs_get_size(node);
			break;
		default:
			lock_release(&_fd_lock);
			return FD_ERROR_INVALID_FLAGS;
	}
	lock_release(&_fd_lock);
	return data->offset;
}



int fd_resize(fd_t fd,u64 size){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	int out=(vfs_set_size(node,size)?0:FD_ERROR_NO_SPACE);
	if (!out&&data->offset>size){
		data->offset=size;
	}
	lock_release(&_fd_lock);
	return out;
}



int fd_absolute_path(fd_t fd,char* buffer,u32 buffer_length){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	int out=vfs_get_full_path(node,buffer,buffer_length);
	lock_release(&_fd_lock);
	return out;
}



int fd_stat(fd_t fd,fd_stat_t* out){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	out->node_id=node->id;
	out->type=node->type;
	out->vfs_index=node->vfs_index;
	out->name_length=node->name_length;
	memcpy(out->name,node->name,64);
	out->size=vfs_get_size(node);
	lock_release(&_fd_lock);
	return 0;
}



int fd_get_relative(fd_t fd,u8 relative,u8 flags){
	if (flags&(~(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_APPEND))){
		return FD_ERROR_INVALID_FLAGS;
	}
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	vfs_node_t* other=vfs_get_relative(node,relative);
	lock_release(&_fd_lock);
	if (!other){
		return FD_ERROR_NO_RELATIVE;
	}
	return _node_to_fd(other,flags);
}



int fd_move(fd_t fd,fd_t dst_fd){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	if (_is_invalid_fd(dst_fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	fd_data_t* dst_data=_get_fd_data(dst_fd);
	vfs_node_t* dst_node=vfs_get_by_id(dst_data->node_id);
	if (!dst_node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	if (node->vfs_index!=dst_node->vfs_index){
		lock_release(&_fd_lock);
		return FD_ERROR_DIFFERENT_FS;
	}
	if (node->type!=dst_node->type){
		lock_release(&_fd_lock);
		return FD_ERROR_DIFFERENT_TYPE;
	}
	if (node->flags&VFS_NODE_TYPE_DIRECTORY){
		if (vfs_get_relative(dst_node,VFS_RELATIVE_FIRST_CHILD)){
			lock_release(&_fd_lock);
			return FD_ERROR_NOT_EMPTY;
		}
	}
	else{
		if (vfs_get_size(dst_node)){
			lock_release(&_fd_lock);
			return FD_ERROR_NOT_EMPTY;
		}
	}
	_Bool out=vfs_move(node,dst_node);
	lock_release(&_fd_lock);
	return (out?0:FD_ERROR_NOT_EMPTY);
}

#include <kernel/fs/fs.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/permissions.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "vfs"



static vfs_node_t* _vfs_root_node=NULL;



static _Bool _has_read_permissions(vfs_node_t* node,u32 flags,uid_t uid,gid_t gid){
	if (!(flags&VFS_LOOKUP_FLAG_CHECK_PERMISSIONS)){
		return 1;
	}
	return !!(vfs_permissions_get(node,uid,gid)&VFS_PERMISSION_READ);
}



KERNEL_PUBLIC void vfs_mount(filesystem_t* fs,const char* path){
	if (!path){
		_vfs_root_node=fs->root;
		spinlock_acquire_exclusive(&(_vfs_root_node->lock));
		_vfs_root_node->relatives.parent=NULL;
		spinlock_release_exclusive(&(_vfs_root_node->lock));
		return;
	}
	vfs_node_t* parent;
	const char* child_name;
	if (vfs_lookup_for_creation(NULL,path,0,0,0,&parent,&child_name)){
		panic("vfs_mount: node already exists");
	}
	spinlock_acquire_exclusive(&(fs->root->lock));
	smm_dealloc(fs->root->name);
	fs->root->name=smm_alloc(child_name,0);
	spinlock_release_exclusive(&(fs->root->lock));
	vfs_node_attach_external_child(parent,fs->root);
}



KERNEL_PUBLIC vfs_node_t* vfs_lookup(vfs_node_t* root,const char* path,u32 flags,uid_t uid,gid_t gid){
	return vfs_lookup_for_creation(root,path,flags,uid,gid,NULL,NULL);
}



KERNEL_PUBLIC vfs_node_t* vfs_lookup_for_creation(vfs_node_t* root,const char* path,u32 flags,uid_t uid,gid_t gid,vfs_node_t** parent,const char** child_name){
	if (!root||path[0]=='/'){
		root=_vfs_root_node;
	}
	if (parent){
		*parent=NULL;
		*child_name=NULL;
	}
	while (root&&path[0]){
		if ((flags&VFS_LOOKUP_FLAG_FOLLOW_LINKS)&&(root->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
			if (!_has_read_permissions(root,flags,uid,gid)){
				return NULL;
			}
			char buffer[4096];
			buffer[vfs_node_read(root,0,buffer,4095,0)]=0;
			if (!buffer[0]){
				return NULL;
			}
			root=vfs_lookup_for_creation(root->relatives.parent,buffer,flags,uid,gid,NULL,NULL);
			if (!root){
				return NULL;
			}
		}
		if (path[0]=='/'){
			path++;
			continue;
		}
		u64 i=0;
		for (;path[i]&&path[i]!='/';i++){
			if (i>=SMM_MAX_LENGTH){
				return NULL;
			}
		}
		if (i==1&&path[0]=='.'){
			path+=1;
			continue;
		}
		if (i==2&&path[0]=='.'&&path[1]=='.'){
			root=root->relatives.parent;
			if (!root){
				root=_vfs_root_node;
			}
			path+=2;
			continue;
		}
		if (!_has_read_permissions(root,flags,uid,gid)){
			return NULL;
		}
		SMM_TEMPORARY_STRING name=smm_alloc(path,i);
		vfs_node_t* child=vfs_node_lookup(root,name);
		path+=i;
		if (!child&&parent){
			if (!path[0]){
				*parent=root;
				*child_name=path-i;
				return NULL;
			}
			panic("vfs_lookup_for_creation: alloc virtual node");
		}
		root=child;
	}
	if (root&&(flags&VFS_LOOKUP_FLAG_FOLLOW_LINKS)&&(root->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
		if (!_has_read_permissions(root,flags,uid,gid)){
			return NULL;
		}
		char buffer[4096];
		buffer[vfs_node_read(root,0,buffer,4095,0)]=0;
		if (!buffer[0]){
			return NULL;
		}
		root=vfs_lookup_for_creation(root->relatives.parent,buffer,flags,uid,gid,NULL,NULL);
		if (!root){
			return NULL;
		}
	}
	return root;
}



KERNEL_PUBLIC u32 vfs_path(vfs_node_t* node,char* buffer,u32 buffer_length){
	u32 i=buffer_length;
	for (;node;node=node->relatives.parent){
		if (i<node->name->length+1){
			return 0;
		}
		i-=node->name->length+1;
		buffer[i]='/';
		memcpy(buffer+i+1,node->name->data,node->name->length);
	}
	if (buffer_length-i==1){
		i--;
		buffer[i]='/';
	}
	i++;
	for (u32 j=0;j<buffer_length-i;j++){
		buffer[j]=buffer[i+j];
	}
	buffer[buffer_length-i]=0;
	return buffer_length-i;
}

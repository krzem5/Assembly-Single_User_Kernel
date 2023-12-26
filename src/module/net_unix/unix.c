#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/socket/socket.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#include <net/unix.h>
#define KERNEL_LOG_NAME "net_unix"



static const socket_dtp_descriptor_t _net_unix_socket_dtp_descriptor;



static void _socket_create_pair_callback(socket_pair_t* pair){
	pair->sockets[0]->remote_ctx=pair->sockets[1];
	pair->sockets[1]->remote_ctx=pair->sockets[0];
}



static _Bool _socket_bind_callback(socket_vfs_node_t* socket_node,const void* address,u32 address_length){
	if (address_length!=sizeof(net_unix_address_t)){
		return 0;
	}
	net_unix_address_t fixed_address=*((const net_unix_address_t*)address);
	fixed_address.path[255]=0;
	return socket_move(&(socket_node->node),fixed_address.path);
}



static void _socket_debind_callback(socket_vfs_node_t* socket_node){
	socket_move(&(socket_node->node),NULL);
}



static _Bool _socket_connect_callback(socket_vfs_node_t* socket_node,const void* address,u32 address_length){
	if (address_length!=sizeof(net_unix_address_t)){
		return 0;
	}
	net_unix_address_t fixed_address=*((const net_unix_address_t*)address);
	fixed_address.path[255]=0;
	vfs_node_t* other_node=vfs_lookup(NULL,fixed_address.path,0,0,0);
	if (!other_node||(other_node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return 0;
	}
	socket_vfs_node_t* other_socket_node=(socket_vfs_node_t*)other_node;
	if (other_socket_node->descriptor!=&_net_unix_socket_dtp_descriptor){
		ERROR("Not a UNIX datagram socket");
		return 0;
	}
	other_node->rc++;
	socket_node->remote_ctx=other_node;
	return 1;
}



static void _socket_deconnect_callback(socket_vfs_node_t* socket_node){
	if (socket_node->remote_ctx){
		((vfs_node_t*)(socket_node->remote_ctx))->rc--;
	}
	socket_node->remote_ctx=NULL;
}



static u64 _socket_read_callback(socket_vfs_node_t* socket_node,void* buffer,u64 length,u32 flags){
	socket_packet_t* socket_packet=socket_pop_packet(&(socket_node->node),!(flags&VFS_NODE_FLAG_NONBLOCKING));
	if (!socket_packet){
		return 0;
	}
	if (length>socket_packet->size){
		length=socket_packet->size;
	}
	memcpy(buffer,socket_packet->data,length);
	socket_dealloc_packet(socket_packet);
	return length;
}



static u64 _socket_write_callback(socket_vfs_node_t* socket_node,const void* buffer,u64 length){
	if (!socket_node->remote_ctx){
		return 0;
	}
	void* data=amm_alloc(length);
	memcpy(data,buffer,length);
	if (socket_alloc_packet(&(socket_node->node),data,length)){
		return length;
	}
	amm_dealloc(data);
	return 0;
}



static _Bool _socket_write_packet_callback(socket_vfs_node_t* socket_node,const void* buffer,u32 length){
	return _socket_write_callback(socket_node,buffer,length)==length;
}



static const socket_dtp_descriptor_t _net_unix_socket_dtp_descriptor={
	"UNIX",
	SOCKET_DOMAIN_UNIX,
	SOCKET_TYPE_DGRAM,
	SOCKET_PROTOCOL_NONE,
	_socket_create_pair_callback,
	_socket_bind_callback,
	_socket_debind_callback,
	_socket_connect_callback,
	_socket_deconnect_callback,
	_socket_read_callback,
	_socket_write_callback,
	_socket_write_packet_callback
};



void net_unix_init(void){
	LOG("Registering UNIX datagram sockets...");
	socket_register_dtp_descriptor(&_net_unix_socket_dtp_descriptor);
}

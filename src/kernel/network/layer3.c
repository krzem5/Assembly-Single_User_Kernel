#include <kernel/bios/bios.h>
#include <kernel/clock/clock.h>
#include <kernel/vfs/vfs.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/network/layer2.h>
#include <kernel/network/layer3.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "layer3"



#define DEVICE_LIST_CACHE_FILE_PATH "/kernel/layer3_device_cache"

#define MAX_DEVICE_COUNT 1024



static _Bool _layer3_cache_enabled=0;
static lock_t _layer3_lock=LOCK_INIT_STRUCT;
static network_layer3_device_t* _layer3_devices;
static u32 _layer3_device_count=0;
static u32 _layer3_device_max_count=MAX_DEVICE_COUNT;
static u64 _layer3_last_ping_time=0;
static _Bool _layer3_cache_is_dirty=0;



static void _load_device_list_cache(void){
	if (!_layer3_cache_enabled){
		return;
	}
	vfs_node_t* node=vfs_get_by_path(NULL,DEVICE_LIST_CACHE_FILE_PATH,0);
	if (!node){
		INFO("Device cache file not found");
		return;
	}
	if (vfs_read(node,0,&_layer3_device_count,sizeof(u32))!=sizeof(u32)){
		goto _error;
	}
	if (_layer3_device_count>_layer3_device_max_count){
		ERROR("Too many devices");
		_layer3_device_count=_layer3_device_max_count;
	}
	for (u32 i=0;i<_layer3_device_count;i++){
		if (vfs_read(node,sizeof(u32)+56*i,_layer3_devices+i,56)!=56){
			goto _error;
		}
		(_layer3_devices+i)->flags&=~NETWORK_LAYER3_DEVICE_FLAG_ONLINE;
		(_layer3_devices+i)->ping=0;
		(_layer3_devices+i)->last_ping_time=0;
	}
	return;
_error:
	ERROR("Unable to read devices from cache file");
	_layer3_device_count=0;
}



static u32 _get_device_index(const u8* address){
	for (u32 i=0;i<_layer3_device_count;i++){
		for (u8 j=0;j<6;j++){
			if ((_layer3_devices+i)->address[j]!=address[j]){
				goto _next_device;
			}
		}
		return i;
_next_device:
	}
	return 0xffffffff;
}



static void _update_ping_time(network_layer3_device_t* device){
	device->flags|=NETWORK_LAYER3_DEVICE_FLAG_ONLINE;
	device->ping=clock_get_time()-_layer3_last_ping_time;
	device->last_ping_time=clock_get_time();
}



void network_layer3_init(void){
	LOG("Initializing layer3 network...");
	if (partition_boot->partition_config.type!=PARTITION_CONFIG_TYPE_KFS){
		_layer3_cache_enabled=0;
		WARN("Layer3 network device cache disabled, boot partition not formatted as KFS");
	}
	else{
		_layer3_cache_enabled=1;
	}
	_layer3_devices=kmm_alloc(MAX_DEVICE_COUNT*sizeof(network_layer3_device_t));
	_load_device_list_cache();
	network_layer3_refresh_device_list();
}



void network_layer3_process_packet(const u8* address,u16 buffer_length,const u8* buffer){
	if (!buffer_length){
		return;
	}
	_Bool is_response=buffer[0]&1;
	switch (buffer[0]>>1){
		case NETWORK_LAYER3_PACKET_TYPE_PING_PONG:
			if (is_response){
				lock_acquire_exclusive(&_layer3_lock);
				u32 index=_get_device_index(address);
				if (index!=0xffffffff){
					_update_ping_time(_layer3_devices+index);
				}
				lock_release_exclusive(&_layer3_lock);
			}
			else{
				u8 packet_buffer[1]={(NETWORK_LAYER3_PACKET_TYPE_PING_PONG<<1)|1};
				network_layer2_packet_t packet={
					.protocol=NETWORK_LAYER3_PROTOCOL_TYPE,
					.buffer_length=1,
					.buffer=packet_buffer
				};
				for (u8 i=0;i<6;i++){
					packet.address[i]=address[i];
				}
				network_layer2_send(&packet);
			}
			break;
		case NETWORK_LAYER3_PACKET_TYPE_ENUMERATION:
			if (is_response){
				if (buffer_length<50){
					break;
				}
				lock_acquire_exclusive(&_layer3_lock);
				if (_layer3_device_count>=_layer3_device_max_count){
					lock_release_exclusive(&_layer3_lock);
					ERROR("Too many devices");
					break;
				}
				u32 index=_get_device_index(address);
				if (index==0xffffffff){
					index=_layer3_device_count;
					_layer3_device_count++;
				}
				network_layer3_device_t* device=_layer3_devices+index;
				device->flags&=~NETWORK_LAYER3_DEVICE_FLAG_ONLINE;
				u8 changes=device->flags^buffer[1];
				device->flags=buffer[1];
				for (u8 i=0;i<6;i++){
					device->address[i]=address[i];
				}
				for (u8 i=0;i<16;i++){
					changes|=device->uuid[i]^buffer[i+2];
					device->uuid[i]=buffer[i+2];
				}
				for (u8 i=0;i<32;i++){
					changes|=device->serial_number[i]^buffer[i+18];
					device->serial_number[i]=buffer[i+18];
				}
				device->serial_number[32]=0;
				_update_ping_time(device);
				if (changes){
					_layer3_cache_is_dirty=1;
				}
				lock_release_exclusive(&_layer3_lock);
			}
			else{
				u8 packet_buffer[49]={(NETWORK_LAYER3_PACKET_TYPE_ENUMERATION<<1)|1};
				for (u8 i=0;i<16;i++){
					packet_buffer[i+1]=bios_data.uuid[i];
				}
				for (u8 i=0;i<32;i++){
					packet_buffer[i+17]=bios_data.serial_number[i];
				}
				network_layer2_packet_t packet={
					.protocol=NETWORK_LAYER3_PROTOCOL_TYPE,
					.buffer_length=49,
					.buffer=packet_buffer
				};
				for (u8 i=0;i<6;i++){
					packet.address[i]=address[i];
				}
				network_layer2_send(&packet);
			}
			break;
		default:
			INFO("Unknown packet type '%x/%u' received from %x:%x:%x:%x:%x:%x",buffer[0]>>1,is_response,address[0],address[1],address[2],address[3],address[4],address[5]);
			break;
	}
}



void network_layer3_refresh_device_list(void){
	u8 packet_buffer[1]={NETWORK_LAYER3_PACKET_TYPE_PING_PONG<<1};
	lock_acquire_shared(&_layer3_lock);
	for (u32 i=0;i<_layer3_device_count;i++){
		network_layer2_packet_t packet={
			.protocol=NETWORK_LAYER3_PROTOCOL_TYPE,
			.buffer_length=1,
			.buffer=packet_buffer
		};
		for (u8 j=0;j<6;j++){
			packet.address[j]=(_layer3_devices+i)->address[j];
		}
		network_layer2_send(&packet);
	}
	lock_release_shared(&_layer3_lock);
	packet_buffer[0]=NETWORK_LAYER3_PACKET_TYPE_ENUMERATION<<1;
	network_layer2_packet_t packet={
		{0xff,0xff,0xff,0xff,0xff,0xff},
		NETWORK_LAYER3_PROTOCOL_TYPE,
		1,
		packet_buffer
	};
	network_layer2_send(&packet);
	_layer3_last_ping_time=clock_get_time();
}



u32 network_layer3_get_device_count(void){
	lock_acquire_shared(&_layer3_lock);
	u32 out=_layer3_device_count;
	lock_release_shared(&_layer3_lock);
	return out;
}



const network_layer3_device_t* network_layer3_get_device(u32 index){
	lock_acquire_shared(&_layer3_lock);
	const network_layer3_device_t* out=(index>=_layer3_device_count?NULL:_layer3_devices+index);
	lock_release_shared(&_layer3_lock);
	return out;
}



_Bool network_layer3_delete_device(const u8* address){
	lock_acquire_exclusive(&_layer3_lock);
	_Bool out=0;
	for (u32 i=0;i<_layer3_device_count;i++){
		for (u8 j=0;j<6;j++){
			if ((_layer3_devices+i)->address[j]!=address[j]){
				goto _next_device;
			}
		}
		out=1;
		_layer3_device_count--;
		if (i!=_layer3_device_count){
			*(_layer3_devices+i)=*(_layer3_devices+_layer3_device_count);
		}
		_layer3_cache_is_dirty=1;
		break;
_next_device:
	}
	lock_release_exclusive(&_layer3_lock);
	return out;
}



void network_layer3_flush_cache(void){
	LOG("Flushing layer3 device cache...");
	if (!_layer3_cache_enabled||!_layer3_cache_is_dirty){
		return;
	}
	lock_acquire_shared(&_layer3_lock);
	_layer3_cache_is_dirty=0;
	vfs_node_t* node=vfs_get_by_path(NULL,DEVICE_LIST_CACHE_FILE_PATH,VFS_NODE_TYPE_FILE);
	if (!node){
		ERROR("Unable to open device cache file '%s'",DEVICE_LIST_CACHE_FILE_PATH);
		lock_release_shared(&_layer3_lock);
		return;
	}
	vfs_set_size(node,sizeof(u32)+56*_layer3_device_count);
	vfs_write(node,0,&_layer3_device_count,sizeof(u32));
	for (u32 i=0;i<_layer3_device_count;i++){
		vfs_write(node,sizeof(u32)+56*i,_layer3_devices+i,56);
	}
	lock_release_shared(&_layer3_lock);
}

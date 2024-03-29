#ifndef _SYS_SYSCALL_KERNEL_SYSCALLS_H_
#define _SYS_SYSCALL_KERNEL_SYSCALLS_H_ 1
#include <sys/syscall/syscall.h>
#include <sys/types.h>



static inline u64 _sys_syscall_clock_get_converion(u64* buffer){
	return _sys_syscall1(0x100000001,(u64)buffer);
}



static inline u64 _sys_syscall_cpu_get_count(void){
	return _sys_syscall0(0x100000002);
}



static inline u64 _sys_syscall_fd_close(u64 fd){
	return _sys_syscall1(0x100000003,fd);
}



static inline u64 _sys_syscall_fd_dup(u64 fd,u32 flags){
	return _sys_syscall2(0x100000004,fd,flags);
}



static inline u64 _sys_syscall_fd_iter_get(u64 iterator,char* buffer,u32 size){
	return _sys_syscall3(0x100000005,iterator,(u64)buffer,size);
}



static inline u64 _sys_syscall_fd_iter_next(u64 iterator){
	return _sys_syscall1(0x100000006,iterator);
}



static inline u64 _sys_syscall_fd_iter_start(u64 fd){
	return _sys_syscall1(0x100000007,fd);
}



static inline u64 _sys_syscall_fd_iter_stop(u64 iterator){
	return _sys_syscall1(0x100000008,iterator);
}



static inline u64 _sys_syscall_fd_open(u64 fd,const char* path,u32 flags){
	return _sys_syscall3(0x100000009,fd,(u64)path,flags);
}



static inline u64 _sys_syscall_fd_path(u64 fd,char* buffer,u32 size){
	return _sys_syscall3(0x10000000a,fd,(u64)buffer,size);
}



static inline u64 _sys_syscall_fd_read(u64 fd,void* buffer,u64 size,u32 flags){
	return _sys_syscall4(0x10000000b,fd,(u64)buffer,size,flags);
}



static inline u64 _sys_syscall_fd_resize(u64 fd,u64 size,u32 flags){
	return _sys_syscall3(0x10000000c,fd,size,flags);
}



static inline u64 _sys_syscall_fd_seek(u64 fd,u64 offset,u32 type){
	return _sys_syscall3(0x10000000d,fd,offset,type);
}



static inline u64 _sys_syscall_fd_stat(u64 fd,void* ptr,u32 size){
	return _sys_syscall3(0x10000000e,fd,(u64)ptr,size);
}



static inline u64 _sys_syscall_fd_write(u64 fd,const void* buffer,u64 size,u32 flags){
	return _sys_syscall4(0x10000000f,fd,(u64)buffer,size,flags);
}



static inline u64 _sys_syscall_memory_map(u64 length,u32 flags,u64 fd){
	return _sys_syscall3(0x100000010,length,flags,fd);
}



static inline u64 _sys_syscall_memory_change_flags(void* address,u64 length,u32 flags){
	return _sys_syscall3(0x100000011,(u64)address,length,flags);
}



static inline u64 _sys_syscall_memory_unmap(void* address,u64 length){
	return _sys_syscall2(0x100000012,(u64)address,length);
}



static inline u64 _sys_syscall_system_shutdown(u32 flags){
	return _sys_syscall1(0x100000013,flags);
}



static inline u64 _sys_syscall_thread_create(u64 rip,u64 rdi,u64 rsi,u64 rdx,u64 rsp){
	return _sys_syscall5(0x100000014,rip,rdi,rsi,rdx,rsp);
}



static inline u64 _sys_syscall_thread_get_priority(u64 thread){
	return _sys_syscall1(0x100000016,thread);
}



static inline u64 _sys_syscall_thread_set_priority(u64 thread,u32 priority){
	return _sys_syscall2(0x100000018,thread,priority);
}



static inline u64 _sys_syscall_thread_stop(u64 thread){
	return _sys_syscall1(0x100000019,thread);
}



static inline u64 _sys_syscall_time_get_boot_offset(void){
	return _sys_syscall0(0x10000001a);
}



static inline u64 _sys_syscall_uid_get(void){
	return _sys_syscall0(0x10000001b);
}



static inline u64 _sys_syscall_gid_get(void){
	return _sys_syscall0(0x10000001c);
}



static inline u64 _sys_syscall_uid_set(u32 uid){
	return _sys_syscall1(0x10000001d,uid);
}



static inline u64 _sys_syscall_gid_set(u32 gid){
	return _sys_syscall1(0x10000001e,gid);
}



static inline u64 _sys_syscall_uid_get_name(u32 uid,char* buffer,u32 buffer_length){
	return _sys_syscall3(0x10000001f,uid,(u64)buffer,buffer_length);
}



static inline u64 _sys_syscall_gid_get_name(u32 gid,char* buffer,u32 buffer_length){
	return _sys_syscall3(0x100000020,gid,(u64)buffer,buffer_length);
}



static inline u64 _sys_syscall_process_get_pid(void){
	return _sys_syscall0(0x100000021);
}



static inline u64 _sys_syscall_thread_get_tid(void){
	return _sys_syscall0(0x100000022);
}



static inline u64 _sys_syscall_process_start(const char* path,u32 argc,const char*const* argv,const char*const* environ,u32 flags){
	return _sys_syscall5(0x100000023,(u64)path,argc,(u64)argv,(u64)environ,flags);
}



static inline u64 _sys_syscall_thread_await_events(const u64* handles,u32 count){
	return _sys_syscall2(0x100000024,(u64)handles,count);
}



static inline u64 _sys_syscall_process_get_event(u64 handle){
	return _sys_syscall1(0x100000025,handle);
}



static inline u64 _sys_syscall_syscall_table_get_offset(const char* name){
	return _sys_syscall1(0x100000026,(u64)name);
}



static inline void _sys_syscall_scheduler_yield(void){
	_sys_syscall0(0x100000027);
}



static inline u64 _sys_syscall_acl_get_permissions(u64 handle,u64 process){
	return _sys_syscall2(0x100000028,handle,process);
}



static inline u64 _sys_syscall_acl_set_permissions(u64 handle,u64 process,u64 clear,u64 set){
	return _sys_syscall4(0x100000029,handle,process,clear,set);
}



static inline u64 _sys_syscall_acl_request_permissions(u64 handle,u64 process,u64 flags){
	return _sys_syscall3(0x10000002a,handle,process,flags);
}



static inline u64 _sys_syscall_socket_create(u8 domain,u8 type,u8 protocol){
	return _sys_syscall3(0x10000002b,domain,type,protocol);
}



static inline u64 _sys_syscall_socket_recv(u64 fd,void* buffer,u32 buffer_length,u32 flags){
	return _sys_syscall4(0x10000002c,fd,(u64)buffer,buffer_length,flags);
}



static inline u64 _sys_syscall_socket_send(u64 fd,const void* buffer,u32 buffer_length,u32 flags){
	return _sys_syscall4(0x10000002d,fd,(u64)buffer,buffer_length,flags);
}



static inline u64 _sys_syscall_socket_shutdown(u64 fd,u32 flags){
	return _sys_syscall2(0x10000002e,fd,flags);
}



static inline u64 _sys_syscall_socket_bind(u64 fd,const void* address,u32 address_length){
	return _sys_syscall3(0x10000002f,fd,(u64)address,address_length);
}



static inline u64 _sys_syscall_socket_connect(u64 fd,const void* address,u32 address_length){
	return _sys_syscall3(0x100000030,fd,(u64)address,address_length);
}



static inline u64 _sys_syscall_socket_create_pair(u8 domain,u8 type,u8 protocol,u64* out){
	return _sys_syscall4(0x100000031,domain,type,protocol,(u64)out);
}



static inline u64 _sys_syscall_timer_create(u64 interval,u64 count){
	return _sys_syscall2(0x100000032,interval,count);
}



static inline u64 _sys_syscall_timer_delete(u64 timer){
	return _sys_syscall1(0x100000033,timer);
}



static inline u64 _sys_syscall_timer_get_deadline(u64 timer){
	return _sys_syscall1(0x100000034,timer);
}



static inline u64 _sys_syscall_timer_update(u64 timer,u64 interval,u64 count){
	return _sys_syscall3(0x100000035,timer,interval,count);
}



static inline u64 _sys_syscall_timer_get_event(u64 timer){
	return _sys_syscall1(0x100000036,timer);
}



static inline u64 _sys_syscall_event_create(u32 is_active){
	return _sys_syscall1(0x100000037,is_active);
}



static inline u64 _sys_syscall_event_delete(u64 event){
	return _sys_syscall1(0x100000038,event);
}



static inline u64 _sys_syscall_event_dispatch(u64 event,u32 dispatch_flags){
	return _sys_syscall2(0x100000039,event,dispatch_flags);
}



static inline u64 _sys_syscall_event_set_active(u64 event,u32 is_active){
	return _sys_syscall2(0x10000003a,event,is_active);
}



static inline u64 _sys_syscall_fs_get_next(u64 fs){
	return _sys_syscall1(0x10000003b,fs);
}



static inline u64 _sys_syscall_fs_get_data(u64 fs,void* buffer,u32 buffer_length){
	return _sys_syscall3(0x10000003c,fs,(u64)buffer,buffer_length);
}



static inline u64 _sys_syscall_fs_mount(u64 fs,const char* path){
	return _sys_syscall2(0x10000003d,fs,(u64)path);
}



static inline u64 _sys_syscall_process_set_cwd(u64 process,u64 fd){
	return _sys_syscall2(0x10000003e,process,fd);
}



static inline u64 _sys_syscall_process_get_parent(u64 process){
	return _sys_syscall1(0x10000003f,process);
}



static inline u64 _sys_syscall_process_set_root(u64 process,u64 fd){
	return _sys_syscall2(0x100000040,process,fd);
}



static inline u64 _sys_syscall_pipe_create(const char* path){
	return _sys_syscall1(0x100000041,(u64)path);
}



static inline void _sys_syscall_signature_verify(const char* name,void* data,u64 size){
	_sys_syscall3(0x100000042,(u64)name,(u64)data,size);
}



static inline u64 _sys_syscall_memory_get_size(void* address){
	return _sys_syscall1(0x100000043,(u64)address);
}



#endif

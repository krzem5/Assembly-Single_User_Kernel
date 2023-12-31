#ifndef _SYS_MEMORY_MEMORY_H_
#define _SYS_MEMORY_MEMORY_H_ 1
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/types.h>



#define SYS_PAGE_SIZE 4096

#define SYS_MEMORY_FLAG_READ 1
#define SYS_MEMORY_FLAG_WRITE 2
#define SYS_MEMORY_FLAG_EXEC 4
#define SYS_MEMORY_FLAG_FILE 8
#define SYS_MEMORY_FLAG_NOWRITEBACK 16



static inline u64 sys_memory_align_up_address(u64 base){
	return (base+SYS_PAGE_SIZE-1)&(-SYS_PAGE_SIZE);
}



static inline u64 sys_memory_align_down_address(u64 base){
	return base&(-SYS_PAGE_SIZE);
}



u64 sys_memory_map(u64 length,u32 flags,sys_fd_t fd);



sys_error_t sys_memory_change_flags(void* address,u64 length,u32 flags);



sys_error_t sys_memory_unmap(void* address,u64 length);



#endif

#ifndef _KERNEL_SYSCALL_SYSCALL_H_
#define _KERNEL_SYSCALL_SYSCALL_H_ 1
#include <kernel/types.h>



typedef u64 (*syscall_callback_t)();



typedef struct _SYSCALL_TABLE{
	const char* name;
	const syscall_callback_t* functions;
	u32 function_count;
	u32 index;
} syscall_table_t;



u32 syscall_create_table(const char* name,const syscall_callback_t* functions,u32 function_count);



u64 syscall_get_user_pointer_max_length(const void* ptr);



u64 syscall_get_string_length(const void* ptr);



void syscall_enable(void);



#endif

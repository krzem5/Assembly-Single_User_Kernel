#ifndef _SYS_ERROR_ERROR_H_
#define _SYS_ERROR_ERROR_H_ 1
#include <sys/types.h>



#define SYS_IS_ERROR(error) (((s64)(error))<0)

#define _SYS_BUILD_ERROR(id) ((u64)(-((s64)(id))))

#define SYS_ERROR_OK _SYS_BUILD_ERROR(0x0000)
#define SYS_ERROR_INVALID_SYSCALL _SYS_BUILD_ERROR(0x0001)
#define SYS_ERROR_INVALID_ARGUMENT(index) _SYS_BUILD_ERROR((0x0002|((index)<<16)))
#define SYS_ERROR_NOT_FOUND _SYS_BUILD_ERROR(0x0003)
#define SYS_ERROR_INVALID_HANDLE _SYS_BUILD_ERROR(0x0004)
#define SYS_ERROR_NO_ACL _SYS_BUILD_ERROR(0x0005)
#define SYS_ERROR_DENIED _SYS_BUILD_ERROR(0x0006)
#define SYS_ERROR_UNSUPPORTED_OPERATION _SYS_BUILD_ERROR(0x0007)
#define SYS_ERROR_NO_SPACE _SYS_BUILD_ERROR(0x0008)
#define SYS_ERROR_NO_DATA _SYS_BUILD_ERROR(0x0009)
#define SYS_ERROR_INVALID_FORMAT _SYS_BUILD_ERROR(0x000a)
#define SYS_ERROR_INVALID_ADDRESS _SYS_BUILD_ERROR(0x000b)
#define SYS_ERROR_DISABLED_OPERATION _SYS_BUILD_ERROR(0x000c)
#define SYS_ERROR_NO_MEMORY _SYS_BUILD_ERROR(0x000d)
#define SYS_ERROR_ALREADY_PRESENT _SYS_BUILD_ERROR(0x000e)

#define SYS_ERROR_GET_INDEX(error) ((error)>>16)



typedef u64 sys_error_t;



#endif
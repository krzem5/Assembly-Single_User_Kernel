#ifndef _KERNEL_CLOCK_CLOCK_H_
#define _KERNEL_CLOCK_CLOCK_H_ 1
#include <kernel/types.h>



extern u64 clock_cpu_frequency;
extern u64 clock_conversion_factor;
extern u32 clock_conversion_shift;



u64 clock_get_ticks(void);



u64 clock_get_time(void);



u64 clock_ticks_to_time(u64 ticks);



#endif

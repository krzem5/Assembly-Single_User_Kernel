#ifndef _CORE_CLOCK_H_
#define _CORE_CLOCK_H_ 1
#include <core/types.h>



extern u64 clock_cpu_frequency;



void clock_init(void);



u64 clock_get_ticks(void);



u64 clock_get_time(void);



u64 clock_ticks_to_time(u64 ticks);



#endif
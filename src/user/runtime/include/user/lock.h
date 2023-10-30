#ifndef _USER_SPINlock_H_
#define _USER_SPINlock_H_ 1
#include <user/types.h>



#define SPINLOCK_INIT_STRUCT (0)



typedef u32 spinlock_t;



void spinlock_init(spinlock_t* out);



void spinlock_acquire_exclusive(spinlock_t* lock);



void spinlock_release_exclusive(spinlock_t* lock);



void spinlock_acquire_shared(spinlock_t* lock);



void spinlock_release_shared(spinlock_t* lock);



#endif

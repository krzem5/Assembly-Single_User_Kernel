#ifndef _KERNEL_MEMORY_PMM_H_
#define _KERNEL_MEMORY_PMM_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define PAGE_SIZE 4096
#define PAGE_SIZE_SHIFT 12
#define LARGE_PAGE_SIZE 2097152
#define LARGE_PAGE_SIZE_SHIFT 21
#define EXTRA_LARGE_PAGE_SIZE 1073741824
#define EXTRA_LARGE_PAGE_SIZE_SHIFT 30

#define PMM_ALLOCATOR_SIZE_COUNT 16

#define PMM_LOW_ALLOCATOR_LIMIT 0x40000000ull

#define PMM_MEMORY_HINT_LOW_MEMORY 1

#define PMM_DECLARE_COUNTER(name) \
	static u16 KERNEL_CORE_DATA PMM_COUNTER_##name=0; \
	static const char KERNEL_CORE_RDATA _pmm_counter_descriptor_name_##name[]=#name; \
	static const pmm_counter_descriptor_t KERNEL_CORE_RDATA _pmm_counter_descriptor_##name={ \
		_pmm_counter_descriptor_name_##name, \
		&(PMM_COUNTER_##name) \
	}; \
	static const pmm_counter_descriptor_t* __attribute__((used,section(".pmmcounter"))) _pmm_counter_descriptor_ptr_##name=&_pmm_counter_descriptor_##name;



typedef struct _PMM_ALLOCATOR_PAGE_HEADER{
	u64 prev;
	u64 next;
	u8 idx;
} pmm_allocator_page_header_t;



typedef struct _PMM_ALLOCATOR{
	u64 first_address;
	u64 last_address;
	u64* bitmap;
	u64 blocks[PMM_ALLOCATOR_SIZE_COUNT];
	lock_t lock;
	u16 block_bitmap;
} pmm_allocator_t;



typedef struct _PMM_COUNTER{
	const char* name;
	u64 count;
} pmm_counter_t;



typedef struct _PMM_COUNTERS{
	u32 length;
	pmm_counter_t data[];
} pmm_counters_t;



typedef struct _PMM_COUNTER_DESCRIPTOR{
	const char* name;
	u16* var;
} pmm_counter_descriptor_t;



static inline u64 pmm_align_up_address(u64 base){
	return (base+PAGE_SIZE-1)&(-PAGE_SIZE);
}



static inline u64 pmm_align_up_address_large(u64 base){
	return (base+LARGE_PAGE_SIZE-1)&(-LARGE_PAGE_SIZE);
}



static inline u64 pmm_align_up_address_extra_large(u64 base){
	return (base+EXTRA_LARGE_PAGE_SIZE-1)&(-EXTRA_LARGE_PAGE_SIZE);
}



static inline u64 pmm_align_down_address(u64 base){
	return base&(-PAGE_SIZE);
}



static inline u64 pmm_align_down_address_large(u64 base){
	return base&(-LARGE_PAGE_SIZE);
}



static inline u64 pmm_align_down_address_extra_large(u64 base){
	return base&(-EXTRA_LARGE_PAGE_SIZE);
}



extern u64 pmm_adjusted_kernel_end;



void pmm_init(void);



void pmm_init_high_mem(void);



u64 pmm_alloc(u64 count,u8 counter,_Bool memory_hint);



u64 pmm_alloc_zero(u64 count,u8 counter,_Bool memory_hint);



void pmm_dealloc(u64 address,u64 count,u8 counter);



void pmm_get_counters(pmm_counters_t* out);



#endif

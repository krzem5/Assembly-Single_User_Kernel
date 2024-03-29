#ifndef _TEST_TEST_H_
#define _TEST_TEST_H_ 1
#include <kernel/types.h>



#define TEST_MODULE(name) WARN("Module '%s'...",name)
#define TEST_FUNC(name) WARN("Function '%s'...",name)
#define TEST_GROUP(name) WARN("Group '%s'...",name)
#define TEST_ASSERT(x) \
	do{ \
		if (!(x)){ \
			ERROR("%u: %s: Test failed",__LINE__,#x); \
			test_fail_count++; \
		} \
		else{ \
			test_pass_count++; \
		} \
	} while (0)



extern u64 test_pass_count;
extern u64 test_fail_count;



#endif

#include <linker/search_path.h>
#include <linker/shared_object.h>
#include <linker/symbol.h>
#include <sys/types.h>



#ifdef KERNEL_COVERAGE
SYS_PUBLIC void __sys_linker_set_root_object_gcov_info(u64 base,u64 size){
	shared_object_executable->gcov_info_base=base;
	shared_object_executable->gcov_info_size=size;
}
#endif



SYS_PUBLIC void SYS_NOCOVERAGE __sys_linker_execute_fini(void){
	shared_object_execute_fini();
}



SYS_PUBLIC u64 __sys_linker_get_object_root(void){
	return (u64)shared_object_root;
}



SYS_PUBLIC u64 __sys_linker_get_object_next(u64 handle){
	return (u64)(((const shared_object_t*)handle)->next);
}



SYS_PUBLIC const char* __sys_linker_get_object_path(u64 handle){
	return ((const shared_object_t*)handle)->path;
}



SYS_PUBLIC u64 __sys_linker_get_object_base(u64 handle){
	return ((const shared_object_t*)handle)->image_base;
}



SYS_PUBLIC u64 __sys_linker_lookup_symbol(u64 handle,const char* name){
	return (handle?symbol_lookup_by_name_in_shared_object((const shared_object_t*)handle,name):symbol_lookup_by_name(name));
}



SYS_PUBLIC u64 __sys_linker_load_library(const char* name,u32 flags){
	return (u64)shared_object_load(name,flags);
}



SYS_PUBLIC const char* __sys_linker_update_library_path(const char* new){
	return search_path_update_search_directories(new);
}

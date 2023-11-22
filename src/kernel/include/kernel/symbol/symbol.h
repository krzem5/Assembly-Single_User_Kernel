#ifndef _KERNEL_SYMBOL_SYMBOL_H_
#define _KERNEL_SYMBOL_SYMBOL_H_ 1
#include <kernel/memory/smm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



typedef struct _SYMBOL{
	rb_tree_node_t rb_node;
	const char* module;
	string_t* name;
	_Bool is_public;
} symbol_t;



void symbol_init(void);



void symbol_add(const char* module,const char* name,u64 address,_Bool is_public);



const symbol_t* symbol_lookup(u64 address);



const symbol_t* symbol_lookup_by_name(const char* name);



#endif

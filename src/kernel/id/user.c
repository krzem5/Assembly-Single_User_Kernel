#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "uid"



typedef struct _UID_GROUP{
	rb_tree_node_t rb_node;
} uid_group_t;



typedef struct _UID_DATA{
	rb_tree_node_t rb_node;
	string_t* name;
	rb_tree_t group_tree;
} uid_data_t;



static pmm_counter_descriptor_t _uid_group_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_uid_group");
static pmm_counter_descriptor_t _uid_data_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_uid_data");
static omm_allocator_t _uid_data_allocator=OMM_ALLOCATOR_INIT_STRUCT("uid_data",sizeof(uid_data_t),8,1,&_uid_data_omm_pmm_counter);
static omm_allocator_t _uid_group_allocator=OMM_ALLOCATOR_INIT_STRUCT("uid_group",sizeof(uid_group_t),8,1,&_uid_group_omm_pmm_counter);



static rb_tree_t _uid_tree;
static spinlock_t _uid_global_lock=SPINLOCK_INIT_STRUCT;



void uid_init(void){
	LOG("Initializing user ID tree...");
	rb_tree_init(&_uid_tree);
	INFO("Creating root user...");
	if (!uid_create(0,"root")||!uid_add_group(0,0)){
		panic("Unable to create root user");
	}
}



_Bool uid_create(uid_t uid,const char* name){
	spinlock_acquire_exclusive(&_uid_global_lock);
	if (rb_tree_lookup_node(&_uid_tree,uid)){
		spinlock_release_exclusive(&_uid_global_lock);
		return 0;
	}
	uid_data_t* uid_data=omm_alloc(&_uid_data_allocator);
	uid_data->rb_node.key=uid;
	uid_data->name=smm_alloc(name,0);
	rb_tree_init(&(uid_data->group_tree));
	rb_tree_insert_node(&_uid_tree,&(uid_data->rb_node));
	spinlock_release_exclusive(&_uid_global_lock);
	return 1;
}



_Bool uid_add_group(uid_t uid,gid_t gid){
	spinlock_acquire_exclusive(&_uid_global_lock);
	uid_data_t* uid_data=(uid_data_t*)rb_tree_lookup_node(&_uid_tree,uid);
	if (!uid_data||rb_tree_lookup_node(&(uid_data->group_tree),gid)){
		spinlock_release_exclusive(&_uid_global_lock);
		return 0;
	}
	uid_group_t* uid_group=omm_alloc(&_uid_group_allocator);
	uid_group->rb_node.key=gid;
	rb_tree_insert_node(&(uid_data->group_tree),&(uid_group->rb_node));
	spinlock_release_exclusive(&_uid_global_lock);
	return 1;
}

#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/usb/address_space.h>
#include <kernel/usb/controller.h>



static pmm_counter_descriptor_t _usb_root_controller_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_usb_root_controller");
static pmm_counter_descriptor_t _usb_controller_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_usb_controller");
static omm_allocator_t* _usb_root_controller_allocator=NULL;
static omm_allocator_t* _usb_controller_allocator=NULL;



usb_root_controller_t* usb_root_controller_alloc(void){
	if (!_usb_root_controller_allocator){
		_usb_root_controller_allocator=omm_init("usb_root_controller",sizeof(usb_root_controller_t),8,2,&_usb_root_controller_omm_pmm_counter);
		spinlock_init(&(_usb_root_controller_allocator->lock));
	}
	usb_root_controller_t* out=omm_alloc(_usb_root_controller_allocator);
	usb_address_space_init(&(out->address_space));
	return out;
}



usb_controller_t* usb_controller_alloc(usb_root_controller_t* root_controller){
	if (!_usb_controller_allocator){
		_usb_controller_allocator=omm_init("usb_controller",sizeof(usb_controller_t),8,2,&_usb_controller_omm_pmm_counter);
		spinlock_init(&(_usb_controller_allocator->lock));
	}
	usb_controller_t* out=omm_alloc(_usb_controller_allocator);
	out->root_controller=root_controller;
	return out;
}



void usb_root_controller_dealloc(usb_root_controller_t* root_controller){
	omm_dealloc(_usb_root_controller_allocator,root_controller);
}



void usb_controller_dealloc(usb_controller_t* controller){
	omm_dealloc(_usb_controller_allocator,controller);
}

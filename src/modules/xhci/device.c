#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/usb/controller.h>
#include <kernel/usb/device.h>
#include <kernel/usb/structures.h>
#include <kernel/util/util.h>
#include <xhci/device.h>
#include <xhci/registers.h>
#define KERNEL_LOG_NAME "xhci"



static pmm_counter_descriptor_t _xhci_driver_pmm_counter=PMM_COUNTER_INIT_STRUCT("xhci");
static pmm_counter_descriptor_t _xhci_device_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_xhci_device");
static omm_allocator_t _xhci_device_allocator=OMM_ALLOCATOR_INIT_STRUCT("xhci_device",sizeof(xhci_device_t),8,1,&_xhci_device_omm_pmm_counter);



static KERNEL_INLINE u32 _align_size(u32 size){
	return (size+63)&0xffffffc0;
}



static u32 _get_total_memory_size(const xhci_device_t* device){
	u32 out=0;
	out+=_align_size((device->slots+1)*sizeof(xhci_device_context_base_t));
	out+=_align_size(XHCI_RING_SIZE*sizeof(xhci_transfer_block_t));
	out+=_align_size(XHCI_RING_SIZE*sizeof(xhci_transfer_block_t));
	out+=_align_size(sizeof(xhci_event_ring_segment_t));
	return out;
}



static void _xhci_pipe_update(void* ctx,usb_device_t* device,usb_pipe_t* pipe){
	panic("pipe_update");
}



static void _xhci_pipe_transfer_setup(void* ctx,usb_pipe_t* pipe,const usb_control_request_t* request,void* data){
	panic("_xhci_pipe_transfer_setup");
}



static void _xhci_pipe_transfer_normal(void* ctx,usb_pipe_t* pipe,void* data,u16 length){
	panic("_xhci_pipe_transfer_normal");
}



static _Bool _xhci_detect_port(void* ctx,u16 port){
	const xhci_device_t* xhci_device=ctx;
	return !!((xhci_device->port_registers+port)->portsc&PORTSC_CCS);
}



static u8 _xhci_reset_port(void* ctx,u16 port){
	const xhci_device_t* xhci_device=ctx;
	xhci_port_registers_t* port_registers=xhci_device->port_registers+port;
	if (!(port_registers->portsc&PORTSC_CCS)||(port_registers->portsc&PORTSC_PLS_MASK)!=PORTSC_PLS_U0){
		return USB_DEVICE_SPEED_INVALID;
	}
	SPINLOOP((port_registers->portsc&(PORTSC_CCS|PORTSC_PED))==PORTSC_CCS);
	if (!(port_registers->portsc&PORTSC_PED)){
		return USB_DEVICE_SPEED_INVALID;
	}
	switch (port_registers->portsc&PORTSC_SPEED_MASK){
		case PORTSC_SPEED_FULL:
			return USB_DEVICE_SPEED_FULL;
		case PORTSC_SPEED_LOW:
			return USB_DEVICE_SPEED_LOW;
		case PORTSC_SPEED_HIGH:
			return USB_DEVICE_SPEED_HIGH;
		case PORTSC_SPEED_SUPER:
			return USB_DEVICE_SPEED_SUPER;
	}
	return USB_DEVICE_SPEED_INVALID;
}



static void _xhci_disconnect_port(void* ctx,u16 port){
	panic("_xhci_disconnect_port");
}



static void _xhci_init_device(pci_device_t* device){
	if (device->class!=0x0c||device->subclass!=0x03||device->progif!=0x30){
		return;
	}
	pci_device_enable_memory_access(device);
	pci_device_enable_bus_mastering(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,0,&pci_bar)){
		return;
	}
	LOG("Attached XHCI driver to PCI device %x:%x:%x",device->address.bus,device->address.slot,device->address.func);
	xhci_registers_t* registers=(void*)vmm_identity_map(pci_bar.address,sizeof(xhci_registers_t));
	xhci_operational_registers_t* operational_registers=(void*)vmm_identity_map(pci_bar.address+registers->caplength,sizeof(xhci_operational_registers_t));
	if (operational_registers->pagesize!=1){
		WARN("Page size not supported");
		return;
	}
	xhci_device_t* xhci_device=omm_alloc(&_xhci_device_allocator);
	xhci_device->registers=registers;
	xhci_device->operational_registers=operational_registers;
	xhci_device->ports=registers->hcsparams1>>24;
	xhci_device->interrupts=(registers->hcsparams1>>8)&0x7ff;
	xhci_device->slots=registers->hcsparams1;
	xhci_device->context_size=((registers->hccparams1&0x04)?64:32);
	xhci_device->port_registers=(void*)vmm_identity_map(pci_bar.address+0x400,xhci_device->ports*sizeof(xhci_port_registers_t));
	xhci_device->doorbell_registers=(void*)vmm_identity_map(pci_bar.address+xhci_device->registers->dboff,xhci_device->ports*sizeof(xhci_doorbell_t));
	xhci_device->interrupt_registers=(void*)vmm_identity_map(pci_bar.address+xhci_device->registers->rtsoff+0x20,xhci_device->interrupts*sizeof(xhci_interrupt_registers_t));
	INFO("Ports: %u, Interrupts: %u, Slots: %u, Context size: %u",xhci_device->ports,xhci_device->interrupts,xhci_device->slots,xhci_device->context_size);
	void* data=(void*)(pmm_alloc_zero(pmm_align_up_address(_get_total_memory_size(xhci_device))>>PAGE_SIZE_SHIFT,&_xhci_driver_pmm_counter,PMM_MEMORY_HINT_LOW_MEMORY)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	xhci_device->device_context_base_array=data;
	data+=_align_size((xhci_device->slots+1)*sizeof(xhci_device_context_base_t));
	xhci_device->command_ring=data;
	data+=_align_size(XHCI_RING_SIZE*sizeof(xhci_transfer_block_t));
	xhci_device->event_ring=data;
	data+=_align_size(XHCI_RING_SIZE*sizeof(xhci_transfer_block_t));
	xhci_device->event_ring_segment=data;
	if (xhci_device->operational_registers->usbcmd&USBCMD_RS){
		xhci_device->operational_registers->usbcmd&=~USBCMD_RS;
		SPINLOOP(!(xhci_device->operational_registers->usbsts&USBSTS_HCH));
	}
	xhci_device->operational_registers->usbcmd=USBCMD_HCRST;
	SPINLOOP(xhci_device->operational_registers->usbcmd&USBCMD_HCRST);
	SPINLOOP(xhci_device->operational_registers->usbsts&USBSTS_CNR);
	xhci_device->event_ring_segment->address=((u64)(xhci_device->event_ring))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	xhci_device->event_ring_segment->size=XHCI_RING_SIZE;
	xhci_device->operational_registers->config=xhci_device->slots;
	xhci_device->operational_registers->dcbaap=((u64)(xhci_device->device_context_base_array))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	xhci_device->operational_registers->crcr=((u64)(xhci_device->command_ring))-VMM_HIGHER_HALF_ADDRESS_OFFSET+CRCR_RCS;
	xhci_device->interrupt_registers->erstsz=1;
	xhci_device->interrupt_registers->erstba=((u64)(xhci_device->event_ring_segment))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	xhci_device->interrupt_registers->erdp=((u64)(xhci_device->event_ring))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	u32 spb=xhci_device->registers->hcsparams2;
	spb=(spb>>27)|((spb>>16)&0x3e0);
	if (spb){
		WARN("Setup XHCI scratchpad of size %u",spb);
	}
	xhci_device->operational_registers->usbcmd|=USBCMD_RS;
	COUNTER_SPINLOOP(0xfff);
	usb_root_controller_t* root_controller=usb_root_controller_alloc();
	root_controller->device=xhci_device;
	root_controller->pipe_update=_xhci_pipe_update;
	root_controller->pipe_transfer_setup=_xhci_pipe_transfer_setup;
	root_controller->pipe_transfer_normal=_xhci_pipe_transfer_normal;
	usb_controller_t* usb_controller=usb_controller_alloc(root_controller);
	usb_controller->device=xhci_device;
	usb_controller->detect=_xhci_detect_port;
	usb_controller->reset=_xhci_reset_port;
	usb_controller->disconnect=_xhci_disconnect_port;
	usb_device_t* root_hub=usb_device_alloc(usb_controller,USB_DEVICE_TYPE_HUB,0);
	root_hub->hub.port_count=xhci_device->ports;
	usb_device_enumerate_children(root_hub);
}



void xhci_locate_devices(void){
	HANDLE_FOREACH(HANDLE_TYPE_PCI_DEVICE){
		pci_device_t* device=handle->object;
		_xhci_init_device(device);
	}
}
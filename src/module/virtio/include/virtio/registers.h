#ifndef _VIRTIO_REGISTERS_H_
#define _VIRTIO_REGISTERS_H_ 1
#include <kernel/types.h>



// PCI structure types
#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2
#define VIRTIO_PCI_CAP_ISR_CFG 3
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
#define VIRTIO_PCI_CAP_PCI_CFG 5 // ignored (not presented by ex. virtio-fs)

#define VIRTIO_PCI_CAP_MIN VIRTIO_PCI_CAP_COMMON_CFG
#define VIRTIO_PCI_CAP_MAX VIRTIO_PCI_CAP_DEVICE_CFG

// Registers
#define VIRTIO_REG_DEVICE_FEATURE_SELECT 0x00
#define VIRTIO_REG_DEVICE_FEATURE 0x04
#define VIRTIO_REG_DRIVER_FEATURE_SELECT 0x08
#define VIRTIO_REG_DRIVER_FEATURE 0x0c
#define VIRTIO_REG_MSIX_CONFIG 0x10
#define VIRTIO_REG_NUM_QUEUES 0x12
#define VIRTIO_REG_DEVICE_STATUS 0x14
#define VIRTIO_REG_CONFIG_GENERATION 0x15
#define VIRTIO_REG_QUEUE_SELECT 0x16
#define VIRTIO_REG_QUEUE_SIZE 0x18
#define VIRTIO_REG_QUEUE_MSIX_VECTOR 0x1a
#define VIRTIO_REG_QUEUE_ENABLE 0x1c
#define VIRTIO_REG_QUEUE_NOTIFY_OFF 0x1e
#define VIRTIO_REG_QUEUE_DESC_LO 0x20
#define VIRTIO_REG_QUEUE_DESC_HI 0x24
#define VIRTIO_REG_QUEUE_DRIVER_LO 0x28
#define VIRTIO_REG_QUEUE_DRIVER_HI 0x2c
#define VIRTIO_REG_QUEUE_DEVICE_LO 0x30
#define VIRTIO_REG_QUEUE_DEVICE_HI 0x34

// Device Status flags
#define VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE 0x01
#define VIRTIO_DEVICE_STATUS_FLAG_DRIVER 0x02
#define VIRTIO_DEVICE_STATUS_FLAG_DRIVER_OK 0x04
#define VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK 0x08
#define VIRTIO_DEVICE_STATUS_FLAG_DEVICE_NEEDS_RESET 0x40
#define VIRTIO_DEVICE_STATUS_FLAG_FAILED 0x80

// Features
#define VIRTIO_F_INDIRECT_DESC 28
#define VIRTIO_F_EVENT_IDX 29
#define VIRTIO_F_VERSION_1 32

// Descriptor flags
#define VIRTQ_DESC_F_NEXT 1
#define VIRTQ_DESC_F_WRITE 2

// Queue Available flags
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1



typedef volatile struct KERNEL_PACKED _VIRTIO_QUEUE_DESCRIPTOR{
	u64 address;
	u32 length;
	u16 flags;
	u16 next;
} virtio_queue_descriptor_t;



typedef volatile struct KERNEL_PACKED _VIRTIO_QUEUE_AVAILABLE{
	u16 flags;
	u16 index;
	u16 ring[];
} virtio_queue_available_t;



typedef volatile struct KERNEL_PACKED _VIRTIO_QUEUE_USED_ENTRY{
	u32 index;
	u32 length;
} virtio_queue_used_entry_t;



typedef volatile struct KERNEL_PACKED _VIRTIO_QUEUE_USED{
	u16 flags;
	u16 index;
	virtio_queue_used_entry_t ring[];
} virtio_queue_used_t;



typedef volatile struct KERNEL_PACKED _VIRTIO_QUEUE_EVENT{
	u16 index;
} virtio_queue_event_t;



#endif

#ifndef _KERNEL_USB_STRUCTURES_H_
#define _KERNEL_USB_STRUCTURES_H_ 1
#include <kernel/types.h>



#define USB_DIR_OUT 0x00
#define USB_DIR_IN 0x80

#define USB_TYPE_STANDARD 0x00
#define USB_TYPE_CLASS 0x20
#define USB_TYPE_VENDOR 0x40

#define USB_RECIP_DEVICE 0x00
#define USB_RECIP_INTERFACE 0x01
#define USB_RECIP_ENDPOINT 0x02
#define USB_RECIP_OTHER 0x03

#define USB_REQ_GET_STATUS 0x00
#define USB_REQ_CLEAR_FEATURE 0x01
#define USB_REQ_SET_FEATURE 0x03
#define USB_REQ_SET_ADDRESS 0x05
#define USB_REQ_GET_DESCRIPTOR 0x06
#define USB_REQ_SET_DESCRIPTOR 0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_REQ_GET_INTERFACE 0x0a
#define USB_REQ_SET_INTERFACE 0x0b

#define USB_DT_DEVICE 0x01
#define USB_DT_CONFIG 0x02
#define USB_DT_STRING 0x03
#define USB_DT_INTERFACE 0x04
#define USB_DT_ENDPOINT 0x05
#define USB_DT_DEVICE_QUALIFIER 0x06
#define USB_DT_OTHER_SPEED_CONFIG 0x07
#define USB_DT_ENDPOINT_COMPANION 0x30

#define USB_ENDPOINT_XFER_CONTROL 0x00
#define USB_ENDPOINT_XFER_ISOC 0x01
#define USB_ENDPOINT_XFER_BULK 0x02
#define USB_ENDPOINT_XFER_INT 0x03
#define USB_ENDPOINT_XFER_MASK 0x03



typedef struct _USB_RAW_CONTROL_REQUEST{
	u8 bRequestType;
	u8 bRequest;
	u16 wValue;
	u16 wIndex;
	u16 wLength;
} usb_raw_control_request_t;



typedef struct _USB_RAW_DEVICE_DESCRIPTOR{
	u8 bLength;
	u8 bDescriptorType;
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize0;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;
} usb_raw_device_descriptor_t;



typedef struct _USB_RAW_CONFIGURATION_DESCRIPTOR{
	u8 bLength;
	u8 bDescriptorType;
	u16 wTotalLength;
	u8 bNumInterfaces;
	u8 bConfigurationValue;
	u8 iConfiguration;
	u8 bmAttributes;
	u8 bMaxPower;
	u8 extra_data[];
} usb_raw_configuration_descriptor_t;



typedef struct _USB_RAW_INTERFACE_DESCRIPTOR{
	u8 bLength;
	u8 bDescriptorType;
	u8 bInterfaceNumber;
	u8 bAlternateSetting;
	u8 bNumEndpoints;
	u8 bInterfaceClass;
	u8 bInterfaceSubClass;
	u8 bInterfaceProtocol;
	u8 iInterface;
} usb_raw_interface_descriptor_t;



typedef struct _USB_RAW_ENDPOINT_DESCRIPTOR{
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u16 wMaxPacketSize;
	u8 bInterval;
} usb_raw_endpoint_descriptor_t;



#endif

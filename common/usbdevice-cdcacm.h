#ifndef USBDEVICE_CDCACM_H_
#define USBDEVICE_CDCACM_H_

#define ACM_USB_TYPE			(0x0200)
#define ACM_ID_VENDOR			(0xDEAD)
#define ACM_ID_PRODUCT			(0xBEEF)
#define ACM_MAX_PACKET_SIZE		(64)
#define ACM_EP_MAX_PACKET_SIZE		(16)
#define ACM_COMM_POLL_INTERVAL		(255)
#define ACM_INFO_STRINGS_NUM		(3)

#include <stdlib.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/scb.h>

struct cdcacm_func_descriptors {
	struct usb_cdc_header_descriptor		head;
	struct usb_cdc_call_management_descriptor	manage;
	struct usb_cdc_acm_descriptor			acm;
	struct usb_cdc_union_descriptor			cdc_union;
} __attribute__((packed));

struct usbcdcacm {
	usbd_device					*usbd_dev;
	struct usb_device_descriptor			dev_cdc;
	struct usb_endpoint_descriptor			comm_endp[1];
	struct usb_endpoint_descriptor			data_endp[2];

	struct cdcacm_func_descriptors			cdc_func_dsc;

	struct usb_interface_descriptor			comm_iface[1];
	struct usb_interface_descriptor			data_iface[1];
	struct usb_interface				ifaces[2];

	struct usb_config_descriptor			config;

	uint8_t usbd_control_buffer[128];

};

void cons_set_device_decriptor(struct usb_device_descriptor *device_dsc);

void cons_set_command_endpoint(struct usb_endpoint_descriptor *comm_ep);

void cons_set_data_endpoint(struct usb_endpoint_descriptor *data_ep);

void cons_set_functional_descriptors(struct cdcacm_func_descriptors *fdsc);

void cons_set_command_interface(struct usb_interface_descriptor *comm_iface,
		struct usb_endpoint_descriptor *comm_ep,
		struct cdcacm_func_descriptors *fdsc);

void cons_set_data_interface(struct usb_interface_descriptor *data_iface,
		struct usb_endpoint_descriptor *data_ep);

void cons_set_interfaces(struct usb_interface *ifaces,
		struct usb_interface_descriptor *comm_iface,
		struct usb_interface_descriptor *data_iface);

void cons_set_config_descriptor(struct usb_config_descriptor *config,
		struct usb_interface *ifaces);

void cons_set_device_usb_cdcacm(struct usbcdcacm *usb_console);

void cons_data_rx_cb(usbd_device *usbd_dev, uint8_t endpoint);

enum usbd_request_return_codes cons_control_request(usbd_device *usbd_dev,
		struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
		void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req));

void cons_set_configuration(usbd_device *usbd_dev, uint16_t wValue);

#endif /* USBDEVICE_CDCACM_H_ */

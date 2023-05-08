#include "usbdevice-cdcacm.h"




static const char * cons_strings[] = {
	"Oleksii Nedopytalskyi AVKS-12",
	"Dice Throwing Random Generator",
	"version: 0.0.1",
};

void cons_set_device_decriptor(struct usb_device_descriptor *device_dsc)
{
	device_dsc->bLength = USB_DT_DEVICE_SIZE;
	device_dsc->bDescriptorType = USB_DT_DEVICE;
	device_dsc->bcdUSB = ACM_USB_TYPE;
	device_dsc->bDeviceClass = USB_CLASS_CDC;
	device_dsc->bDeviceSubClass = 0;
	device_dsc->bDeviceProtocol = 0;
	device_dsc->bMaxPacketSize0 = ACM_MAX_PACKET_SIZE;
	device_dsc->idVendor = ACM_ID_VENDOR;
	device_dsc->idProduct = ACM_ID_PRODUCT;
	device_dsc->bcdDevice = ACM_USB_TYPE;
	device_dsc->iManufacturer = 1;
	device_dsc->iProduct = 2;
	device_dsc->iSerialNumber = 3;
	device_dsc->bNumConfigurations = 1;

	return;
}


void cons_set_command_endpoint(struct usb_endpoint_descriptor *comm_ep)
{
	comm_ep->bLength = USB_DT_ENDPOINT_SIZE;
	comm_ep->bDescriptorType = USB_DT_ENDPOINT;
	comm_ep->bEndpointAddress = 0x83;
	comm_ep->bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT;
	comm_ep->wMaxPacketSize = ACM_EP_MAX_PACKET_SIZE;
	comm_ep->bInterval = ACM_COMM_POLL_INTERVAL;

	return;
}

void cons_set_data_endpoint(struct usb_endpoint_descriptor *data_ep)
{
	data_ep[0].bLength = USB_DT_ENDPOINT_SIZE;
	data_ep[0].bDescriptorType = USB_DT_ENDPOINT;
	data_ep[0].bEndpointAddress = 0x01;
	data_ep[0].bmAttributes = USB_ENDPOINT_ATTR_BULK;
	data_ep[0].wMaxPacketSize = ACM_EP_MAX_PACKET_SIZE;
	data_ep[0].bInterval = 1;

	data_ep[1].bLength = USB_DT_ENDPOINT_SIZE;
	data_ep[1].bDescriptorType = USB_DT_ENDPOINT;
	data_ep[1].bEndpointAddress = 0x82;
	data_ep[1].bmAttributes = USB_ENDPOINT_ATTR_BULK;
	data_ep[1].wMaxPacketSize = ACM_EP_MAX_PACKET_SIZE;
	data_ep[1].bInterval = 1;

	return;
}

void cons_set_functional_descriptors(struct cdcacm_func_descriptors *fdsc)
{
	fdsc->head.bFunctionLength = sizeof(struct usb_cdc_header_descriptor);
	fdsc->head.bDescriptorType = CS_INTERFACE;
	fdsc->head.bDescriptorSubtype = USB_CDC_TYPE_HEADER;
	fdsc->head.bcdCDC = 0x0110;

	fdsc->manage.bFunctionLength =
		sizeof(struct usb_cdc_call_management_descriptor);
	fdsc->manage.bDescriptorType = CS_INTERFACE;
	fdsc->manage.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT;
	fdsc->manage.bmCapabilities = 0;
	fdsc->manage.bDataInterface = 1;

	fdsc->acm.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor);
	fdsc->acm.bDescriptorType = CS_INTERFACE;
	fdsc->acm.bDescriptorSubtype = USB_CDC_TYPE_ACM;
	fdsc->acm.bmCapabilities = 0;

	fdsc->cdc_union.bFunctionLength = sizeof(struct usb_cdc_union_descriptor);
	fdsc->cdc_union.bDescriptorType = CS_INTERFACE;
	fdsc->cdc_union.bDescriptorSubtype = USB_CDC_TYPE_UNION;
	fdsc->cdc_union.bControlInterface = 0;
	fdsc->cdc_union.bSubordinateInterface0 = 1;

	return;
}


void cons_set_command_interface(struct usb_interface_descriptor *comm_iface,
		struct usb_endpoint_descriptor *comm_ep,
		struct cdcacm_func_descriptors *fdsc)
{
	comm_iface[0].bLength = USB_DT_INTERFACE_SIZE;
	comm_iface[0].bDescriptorType = USB_DT_INTERFACE;
	comm_iface[0].bInterfaceNumber = 0;
	comm_iface[0].bAlternateSetting = 0;
	comm_iface[0].bNumEndpoints = 1;
	comm_iface[0].bInterfaceClass = USB_CLASS_CDC;
	comm_iface[0].bInterfaceSubClass = USB_CDC_SUBCLASS_ACM;
	comm_iface[0].bInterfaceProtocol = USB_CDC_PROTOCOL_AT;
	comm_iface[0].iInterface = 0;

	comm_iface[0].endpoint = comm_ep;
	comm_iface[0].extra = fdsc;
	comm_iface[0].extralen = sizeof(*fdsc);

	return;
}

void cons_set_data_interface(struct usb_interface_descriptor *data_iface,
		struct usb_endpoint_descriptor *data_ep)
{
	data_iface[0].bLength = USB_DT_INTERFACE_SIZE;
	data_iface[0].bDescriptorType = USB_DT_INTERFACE;
	data_iface[0].bInterfaceNumber = 1;
	data_iface[0].bAlternateSetting = 0;
	data_iface[0].bNumEndpoints = 2;
	data_iface[0].bInterfaceClass = USB_CLASS_DATA;
	data_iface[0].bInterfaceSubClass = 0;
	data_iface[0].bInterfaceProtocol = 0;
	data_iface[0].iInterface = 0;

	data_iface[0].endpoint = data_ep;

	return;
}

void cons_set_interfaces(struct usb_interface *ifaces,
		struct usb_interface_descriptor *comm_iface,
		struct usb_interface_descriptor *data_iface)
{
	ifaces[0].num_altsetting = 1;
	ifaces[0].altsetting = comm_iface;

	ifaces[1].num_altsetting = 1;
	ifaces[1].altsetting = data_iface;

	return;
}

void cons_set_config_descriptor(struct usb_config_descriptor *config,
		struct usb_interface *ifaces)
{
	config->bLength = USB_DT_CONFIGURATION_SIZE;
	config->bDescriptorType = USB_DT_CONFIGURATION;
	config->wTotalLength = 0;
	config->bNumInterfaces = 2;
	config->bConfigurationValue = 1;
	config->bmAttributes = 0x80;
	config->bMaxPower = 0x32;

	config->interface = ifaces;

	return;
}


enum usbd_request_return_codes cons_control_request(usbd_device *usbd_dev,
		struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
		void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)buf;
	(void)usbd_dev;

	switch (req->bRequest) {
		case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
			char local_buf[10];
			struct usb_cdc_notification *notif = (void *)local_buf;
			/* echo signals back to host as notification */
			notif->bmRequestType = 0xA1;
			notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
			notif->wValue = 0;
			notif->wIndex = 0;
			notif->wLength = 2;
			local_buf[8] = req->wValue & 3;
			local_buf[9] = 0;
			usbd_ep_write_packet(usbd_dev, 0x82, local_buf, 10);

			return USBD_REQ_HANDLED;
		}
		case USB_CDC_REQ_SET_LINE_CODING: {
			if (*len < sizeof(struct usb_cdc_line_coding)) {
				return USBD_REQ_NOTSUPP;
			}

			return USBD_REQ_HANDLED;
		}
		default: {
			__asm__("nop");
		}
	}

	return USBD_REQ_NOTSUPP;
}

void cons_data_rx_cb(usbd_device *usbd_dev, uint8_t endpoint)
{
	char buffer[64];
	int length = 0;

	(void)endpoint;
	length = usbd_ep_read_packet(usbd_dev, 0x01, buffer, ACM_MAX_PACKET_SIZE);
	if (length) {
		while (usbd_ep_write_packet(usbd_dev, 0x82, buffer, length) == 0)
		{
			__asm__("nop");
		}
	}

	return;
}

void cons_set_configuration(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;

	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK,
			ACM_MAX_PACKET_SIZE, cons_data_rx_cb);

	usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK,
			ACM_MAX_PACKET_SIZE, NULL);

	usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT,
			ACM_EP_MAX_PACKET_SIZE, NULL);

	usbd_register_control_callback(usbd_dev,
			USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
			USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
			cons_control_request);

	return;
}

void cons_set_device_usb_cdcacm(struct usbcdcacm *usb_console)
{
	cons_set_device_decriptor(&usb_console->dev_cdc);

	cons_set_command_endpoint(&usb_console->comm_endp[0]);
	cons_set_data_endpoint(&usb_console->data_endp[0]);

	cons_set_functional_descriptors(&usb_console->cdc_func_dsc);

	cons_set_command_interface(&usb_console->comm_iface[0],
			&usb_console->comm_endp[0],
			&usb_console->cdc_func_dsc);

	cons_set_data_interface(&usb_console->data_iface[0],
			&usb_console->data_endp[0]);

	cons_set_interfaces(&usb_console->ifaces[0],
			&usb_console->comm_iface[0],
			&usb_console->data_iface[0]);

	cons_set_config_descriptor(&usb_console->config,
			&usb_console->ifaces[0]);

	usb_console->usbd_dev = usbd_init(&otgfs_usb_driver,
			&usb_console->dev_cdc, &usb_console->config,
			cons_strings, ACM_INFO_STRINGS_NUM,
			&usb_console->usbd_control_buffer[0],
			sizeof(usb_console->usbd_control_buffer));

	usbd_register_set_config_callback(usb_console->usbd_dev,
			cons_set_configuration);

	return;
}

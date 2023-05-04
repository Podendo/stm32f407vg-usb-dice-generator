#include <stdio.h>
#include "usbdevice.h"

const uint8_t sysex_identity[] = {
	0x04,	/* USB Framing (3 byte SysEx) */
	0xf0,	/* SysEx start */
	0x7e,	/* non-realtime */
	0x00,	/* Channel 0 */
	0x04,	/* USB Framing (3 byte SysEx) */
	0x7d,	/* Educational/prototype manufacturer ID */
	0x66,	/* Family code (byte 1) */
	0x66,	/* Family code (byte 2) */
	0x04,	/* USB Framing (3 byte SysEx) */
	0x51,	/* Model number (byte 1) */
	0x19,	/* Model number (byte 2) */
	0x00,	/* Version number (byte 1) */
	0x04,	/* USB Framing (3 byte SysEx) */
	0x00,	/* Version number (byte 2) */
	0x01,	/* Version number (byte 3) */
	0x00,	/* Version number (byte 4) */
	0x05,	/* USB Framing (1 byte SysEx) */
	0xf7,	/* SysEx end */
	0x00,	/* Padding */
	0x00,	/* Padding */
};

/* 12 bytes of desig and a \0 */
static char usb_serial_number[25];

/* sending messages via usb array of strings: */
static const char * usb_strings[] = {
	"libopencm3.org",
	"MIDI demo",
	usb_serial_number
};



void usbmidi_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	(void)ep;
	char buffer[64];
	int len = usbd_ep_read_packet(usbd_dev, MIDI_FROM_HOST_EP, buffer,
			sizeof(buffer)/sizeof(char));

	if ( len ) {
		while ( usbd_ep_write_packet(usbd_dev,
			MIDI_TO_HOST_EP, sysex_identity,
			sizeof(sysex_identity)) == 0) {
			__asm__("nop");
		}
	} else {
		__asm__("nop");
	}

	return;
}


void usbmidi_set_config(usbd_device *usbd_dev, uint16_t wvalue)
{
	(void)wvalue;

	usbd_ep_setup(usbd_dev, MIDI_FROM_HOST_EP, USB_ENDPOINT_ATTR_BULK, 64,
			usbmidi_data_rx_cb);

	usbd_ep_setup(usbd_dev, MIDI_TO_HOST_EP, USB_ENDPOINT_ATTR_BULK, 64,
			NULL /* no callback for sending */);
	return;
}


void usbmidi_send_event(usbd_device *usbd_dev, int pressed)
{
	/* USB framing: vitrual cable 0, note on
	* MIDI command: note on, channel 1
	* Note 60 (middle C)
	* Normal Velocity
	*/
	char buffer[4] = { 0x08, 0x80, 60, 64, };

	buffer[0] |= pressed;
	buffer[1] |= pressed << 4;

	while ( usbd_ep_write_packet(usbd_dev, MIDI_TO_HOST_EP,
				buffer, sizeof(buffer)) == 0 ) {
		__asm__("nop");
	}

	return;
}


void usbmidi_set_device_descriptor(struct usb_device_descriptor *dev_dsc)
{
	/* Table B-1: MIDI Adapter Device Descriptor */
	dev_dsc->bLength = USB_DT_DEVICE_SIZE;
	dev_dsc->bDescriptorType = USB_DT_DEVICE;
	dev_dsc->bcdUSB = MIDI_BCD_USB;
	dev_dsc->bDeviceClass = MIDI_DEV_CLASS;
	dev_dsc->bDeviceSubClass = MIDI_DEV_SUB_CLASS;
	dev_dsc->bDeviceProtocol = MIDI_DEV_PROTO;
	dev_dsc->bMaxPacketSize0 = MIDI_MAX_PACKET_SIZE;
	dev_dsc->idVendor = MIDI_VENDOR_ID;
	dev_dsc->idProduct = MIDI_PRODUCT_ID;
	dev_dsc->iSerialNumber = MIDI_SERIALN;
	dev_dsc->bNumConfigurations = MIDI_CFG_NUM;

	return;
}


void usbmidi_set_endpnt_descriptor(struct
		usb_midi_endpoint_descriptor *midiep_dsc)
{
	/* B-12 specification: MIDI Adapter Class-specific Bulk OUT endpoint */
	midiep_dsc[0].head.bLength = sizeof(struct usb_midi_endpoint_descriptor);
	midiep_dsc[0].head.bDescriptorType = USB_AUDIO_DT_CS_ENDPOINT;
	midiep_dsc[0].head.bDescriptorSubType = USB_MIDI_SUBTYPE_MS_GENERAL;
	midiep_dsc[0].head.bNumEmbMIDIJack = 1;
	midiep_dsc[0].jack[0].baAssocJackID = 0x01;

	/* B-14 specification: MIDI Adapter Class-specific Bulk IN endpoint */
	midiep_dsc[1].head.bLength = sizeof(struct usb_midi_endpoint_descriptor);
	midiep_dsc[1].head.bDescriptorType = USB_AUDIO_DT_CS_ENDPOINT;
	midiep_dsc[1].head.bDescriptorSubType = USB_MIDI_SUBTYPE_MS_GENERAL;
	midiep_dsc[1].head.bNumEmbMIDIJack = 1;
	midiep_dsc[1].jack[0].baAssocJackID = 0x03;

	return;
}


void usbmidi_set_usb_ep_descriptor(struct usb_endpoint_descriptor *usbep_dsc,
		struct usb_midi_endpoint_descriptor *midiextra)
{
	/* Table B-11: MIDI Adapter Standart Bulk OUT Endpoint Descriptor */
	usbep_dsc[0].bLength = USB_DT_ENDPOINT_SIZE;
	usbep_dsc[0].bDescriptorType = USB_DT_ENDPOINT;
	usbep_dsc[0].bEndpointAddress = MIDI_FROM_HOST_EP;
	usbep_dsc[0].bmAttributes = USB_ENDPOINT_ATTR_BULK;
	usbep_dsc[0].wMaxPacketSize = 0x40;
	usbep_dsc[0].bInterval = 0x00;

	usbep_dsc[0].extra = &midiextra[0];
	usbep_dsc[0].extralen = sizeof(midiextra[0]);

	usbep_dsc[1].bLength = USB_DT_ENDPOINT_SIZE;
	usbep_dsc[1].bDescriptorType = USB_DT_ENDPOINT;
	usbep_dsc[1].bEndpointAddress = MIDI_TO_HOST_EP;
	usbep_dsc[1].bmAttributes = USB_ENDPOINT_ATTR_BULK;
	usbep_dsc[1].wMaxPacketSize = 0x40;
	usbep_dsc[1].bInterval = 0x00;

	usbep_dsc[1].extra = &midiextra[1];
	usbep_dsc[1].extralen = sizeof(midiextra[1]);

	return;
}


void usbmidi_set_auctrl_descriptor(struct
		audio_control_functional_descriptors *auc_dsc)
{
	/* Table B-4: MIDI Adapter Class-specific AC Interface Descriptor */
	auc_dsc->header_head.bLength =
		sizeof(struct usb_audio_header_descriptor_head) +
		1 * sizeof(struct usb_audio_header_descriptor_body);

	auc_dsc->header_head.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE;
	auc_dsc->header_head.bDescriptorSubtype = USB_AUDIO_TYPE_HEADER;
	auc_dsc->header_head.bcdADC = 0x0100;

	auc_dsc->header_head.wTotalLength =
		sizeof(struct usb_audio_header_descriptor_head) +
		1 * sizeof(struct usb_audio_header_descriptor_body);

	auc_dsc->header_head.binCollection = 1;

	auc_dsc->header_body.baInterfaceNr = 0x01;

	return;
}


void usbmidi_set_iface_audio_ctl(struct usb_interface_descriptor *audio,
		struct audio_control_functional_descriptors *extra)
{
	/* Table B-3: MIDI Adapter Standart AC Interface Descriptor */
	audio[0].bLength = USB_DT_INTERFACE_SIZE;
	audio[0].bDescriptorType = USB_DT_INTERFACE;
	audio[0].bInterfaceNumber = 0;
	audio[0].bAlternateSetting = 0;
	audio[0].bNumEndpoints = 0;
	audio[0].bInterfaceClass = USB_CLASS_AUDIO;
	audio[0].bInterfaceSubClass = USB_AUDIO_SUBCLASS_CONTROL;
	audio[0].bInterfaceProtocol = 0;
	audio[0].iInterface = 0;

	audio[0].extra = &extra[0];
	audio[0].extralen = sizeof(extra[0]);

	return;
}


void usbmidi_set_stream_descriptor(struct
		midi_streaming_functional_descriptors *str_dsc)
{
	/* Table B-6: Midi Adapter Class-specific MS Interface Descriptor */
	str_dsc->header.bLength = sizeof(struct usb_midi_header_descriptor);
	str_dsc->header.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE;
	str_dsc->header.bDescriptorSubtype = USB_MIDI_SUBTYPE_MS_HEADER;
	str_dsc->header.bcdMSC = 0x0100;
	str_dsc->header.wTotalLength =
		sizeof(struct midi_streaming_functional_descriptors);

	/* Table B-7 MIDI Adapter MIDI IN Jack Descriptor (embedded) */
	str_dsc->in_embedded.bLength = sizeof(struct usb_midi_in_jack_descriptor);
	str_dsc->in_embedded.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE;
	str_dsc->in_embedded.bDescriptorSubtype = USB_MIDI_SUBTYPE_MIDI_IN_JACK;
	str_dsc->in_embedded.bJackID = 0x01;
	str_dsc->in_embedded.iJack = 0x00;

	/* Table B-8 MIDI Adapter MIDI IN Jack Descriptor (external) */
	str_dsc->in_external.bLength = sizeof(struct usb_midi_in_jack_descriptor);
	str_dsc->in_external.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE;
	str_dsc->in_external.bDescriptorSubtype = USB_MIDI_SUBTYPE_MIDI_IN_JACK;
	str_dsc->in_external.bJackType = USB_MIDI_JACK_TYPE_EXTERNAL;
	str_dsc->in_external.bJackID = 0x02;
	str_dsc->in_external.iJack = 0x00;

	/* Table B-9 MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */
	str_dsc->out_embedded.head.bLength =
		sizeof(struct usb_midi_out_jack_descriptor);
	str_dsc->out_embedded.head.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE;
	str_dsc->out_embedded.head.bDescriptorSubtype =
		USB_MIDI_SUBTYPE_MIDI_OUT_JACK;

	str_dsc->out_embedded.head.bJackType = USB_MIDI_JACK_TYPE_EMBEDDED;
	str_dsc->out_embedded.head.bJackID = 0x03;
	str_dsc->out_embedded.head.bNrInputPins = 1;

	str_dsc->out_embedded.source[0].baSourceID = 0x02;
	str_dsc->out_embedded.source[0].baSourcePin = 0x01;
	str_dsc->out_embedded.tail.iJack = 0x00;

	/* Table B-10 MIDI Adapter MIDI OUT Jack Descriptor (External) */
	str_dsc->out_external.head.bLength =
		sizeof(struct usb_midi_out_jack_descriptor);

	str_dsc->out_external.head.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE;
	str_dsc->out_external.head.bDescriptorSubtype =
		USB_MIDI_SUBTYPE_MIDI_OUT_JACK;

	str_dsc->out_external.head.bJackType = USB_MIDI_JACK_TYPE_EXTERNAL;
	str_dsc->out_external.head.bJackID = 0x04;
	str_dsc->out_external.head.bNrInputPins = 1;

	str_dsc->out_external.source[0].baSourceID = 0x01;
	str_dsc->out_external.source[0].baSourcePin = 0x01;
	str_dsc->out_external.tail.iJack = 0x00;

	return;
}


void usbmidi_set_iface_streaming(struct usb_interface_descriptor *midi,
		struct usb_endpoint_descriptor *usbep_dsc,
		struct midi_streaming_functional_descriptors *str_dsc)
{
	/* Table B-5: MIDI Adapter Standart MS Interface Descriptor */
	midi[0].bLength = USB_DT_INTERFACE_SIZE;
	midi[0].bDescriptorType = USB_DT_INTERFACE;
	midi[0].bInterfaceNumber = 1;
	midi[0].bAlternateSetting = 0;
	midi[0].bNumEndpoints = 2;
	midi[0].bInterfaceClass = USB_CLASS_AUDIO;
	midi[0].bInterfaceSubClass = USB_AUDIO_SUBCLASS_MIDISTREAMING;
	midi[0].bInterfaceProtocol = 0;
	midi[0].iInterface = 0;

	midi[0].endpoint = usbep_dsc;

	midi[0].extra = str_dsc;
	midi[0].extralen = sizeof(*str_dsc);

	return;
}

void usbmidi_set_ifaces(struct usb_interface *midifaces,
		struct usb_interface_descriptor *audio,
		struct usb_interface_descriptor *midi)
{
	midifaces[0].num_altsetting = 1;
	midifaces[0].altsetting = audio;

	midifaces[1].num_altsetting = 1;
	midifaces[1].altsetting = midi;

	return;
}

void usbmidi_set_config_descriptor(struct usb_config_descriptor *config,
		struct usb_interface *midifaces)
{
	/* Table B-2: MIDI Adapter Configuration Descriptor */
	config->bLength = USB_DT_CONFIGURATION_SIZE;
	config->bDescriptorType = USB_DT_CONFIGURATION;
	config->wTotalLength = 0;
	config->bNumInterfaces = 2;
	config->bConfigurationValue = 1;
	config->iConfiguration = 0;
	config->bmAttributes = 0x80; /* bus powered */
	config->bMaxPower = 0x32;

	config->interface = midifaces;

	return;
}


void usbmidi_set_gadget(struct usbgadget *gadget)
{
	usbmidi_set_device_descriptor(&gadget->device_desc);
	usbmidi_set_endpnt_descriptor(&gadget->midi_bulk_ep[0]);
	usbmidi_set_usb_ep_descriptor(&gadget->bulk_endp[0],
			&gadget->midi_bulk_ep[0]);

	usbmidi_set_auctrl_descriptor(&gadget->au_ctrl_fncdesc);
	usbmidi_set_iface_audio_ctl(&gadget->audio_control_iface[0],
			&gadget->au_ctrl_fncdesc);

	usbmidi_set_stream_descriptor(&gadget->midi_str_fncdesc);
	usbmidi_set_iface_streaming(&gadget->midi_streaming_iface[0],
			&gadget->bulk_endp[0], &gadget->midi_str_fncdesc);

	usbmidi_set_ifaces(&gadget->midifaces[0], &gadget->audio_control_iface[0],
			&gadget->midi_streaming_iface[0]);

	usbmidi_set_config_descriptor(&gadget->config, &gadget->midifaces[0]);

	desig_get_unique_id_as_string(usb_serial_number, sizeof(usb_serial_number));

	gadget->usbd_dev = usbd_init(&otgfs_usb_driver,
			&gadget->device_desc, &gadget->config,
			usb_strings, MIDI_STR_NUM,
			&gadget->usbd_control_buffer[0],
			sizeof(gadget->usbd_control_buffer));

	usbd_register_set_config_callback(gadget->usbd_dev, usbmidi_set_config);

	return;
}

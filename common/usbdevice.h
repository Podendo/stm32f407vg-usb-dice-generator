#ifndef USBGADGET_H_
#define USBGADGET_H_

#include <inttypes.h>

#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/usb/audio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/midi.h>

#define MIDI_BCD_USB		(0x0200)
#define MIDI_VENDOR_ID		(0xDEAD)
#define MIDI_PRODUCT_ID		(0xBEEF)
#define MIDI_DEV_BCD		(0x0100)

#define MIDI_DEV_CLASS		(0)
#define MIDI_DEV_SUB_CLASS	(0)
#define MIDI_DEV_PROTO		(0)
#define MIDI_MAX_PACKET_SIZE	(64)

#define MIDI_MANUFAC		(1)
#define MIDI_PRODUCT		(2)
#define MIDI_SERIALN		(3)
#define MIDI_CFG_NUM		(1)
#define MIDI_STR_NUM		(3)

#define MIDI_FROM_HOST_EP	(0x01)
#define MIDI_TO_HOST_EP		(0x81)


struct audio_control_functional_descriptors {
	struct usb_audio_header_descriptor_head header_head;
	struct usb_audio_header_descriptor_body header_body;
} __attribute__((packed));


struct midi_streaming_functional_descriptors {
	struct usb_midi_header_descriptor header;
	struct usb_midi_in_jack_descriptor in_embedded;
	struct usb_midi_in_jack_descriptor in_external;
	struct usb_midi_out_jack_descriptor out_embedded;
	struct usb_midi_out_jack_descriptor out_external;
} __attribute__((packed));


struct usbgadget {
	usbd_device					*usbd_dev;

	struct usb_device_descriptor			device_desc;

	struct usb_endpoint_descriptor			bulk_endp[2];
	struct usb_midi_endpoint_descriptor		midi_bulk_ep[2];

	struct audio_control_functional_descriptors	au_ctrl_fncdesc;
	struct midi_streaming_functional_descriptors	midi_str_fncdesc;

	struct usb_interface_descriptor			audio_control_iface[1];
	struct usb_interface_descriptor			midi_streaming_iface[1];

	struct usb_interface				midifaces[2];
	struct usb_config_descriptor			config;

	uint8_t usbd_control_buffer[128];
};



void usbmidi_data_rx_cb(usbd_device *usbd_dev, uint8_t ep);

void usbmidi_send_event(usbd_device *usb_dev, int pressed);

void usbmidi_button_poll(usbd_device *usbd_dev);

void usbmidi_set_config(usbd_device *usbd_dev, uint16_t wvalue);


/*
 * @brief setup function for the custom usb device
 * @param *gadget - usbgadget, part of usb-device structure
 */
void usbmidi_set_gadget(struct usbgadget *gadget);

/*
 * @brief USB-2-0 standart usb-device descriptor
 * @param *dev_dsc - usb device descriptor
 */
void usbmidi_set_device_descriptor(struct usb_device_descriptor *dev_dsc);

/*
 * @brief Standart endpoint descriptor
 * @param *usbep_dsc - usb endpoint descriptor
 * @param *midiextra - usb midi endpoint descriptor
 */
void usbmidi_set_usb_ep_descriptor(struct usb_endpoint_descriptor *usbep_dsc,
		struct usb_midi_endpoint_descriptor *midiextra);

void usbmidi_set_endpnt_descriptor(struct
		usb_midi_endpoint_descriptor *midiep_dsc);

void usbmidi_set_auctrl_descriptor(struct
		audio_control_functional_descriptors *auc_dsc);

void usbmidi_set_stream_descriptor(struct
		midi_streaming_functional_descriptors *str_dsc);

void usbmidi_set_config_descriptor(struct usb_config_descriptor *midicfg,
		struct usb_interface *midifaces);

void usbmidi_set_iface_audio_ctl(struct usb_interface_descriptor *audio,
		struct audio_control_functional_descriptors *extra);

void usbmidi_set_iface_streaming(struct usb_interface_descriptor *midi,
		struct usb_endpoint_descriptor *usbep_dsc,
		struct midi_streaming_functional_descriptors *str_dsc);

void usbmidi_set_ifaces(struct usb_interface *midifaces,
		struct usb_interface_descriptor *audio,
		struct usb_interface_descriptor *midi);

#endif /* USB_GADGET_H_ */

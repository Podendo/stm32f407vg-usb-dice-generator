#ifndef MAIN_H_
#define MAIN_H_

#include "usbdevice-cdcacm.h"
#include "usbdevice.h"
#include "dicethrow.h"



#define CONSOLE_BUF_LEN		(128)
#define DEVICE_KEY_DELAY	(20)


struct joysticks {
	uint32_t adc_chan_vrx;
	uint32_t adc_chan_vry;
	uint32_t btn_sw;
};

struct boardleds {
	uint32_t ledport;
	uint16_t ledgpio;
	uint8_t  lednumb;
};

struct device {
	struct usbgadget	midi_usb;
	struct usbcdcacm	cons_usb;
	struct joysticks	joystick;
	struct boardleds	ledsrect;
};

/*
 * @brief device goes to sleep for some amount of time
 * @param delay_ms - value in millis, how long device will sleep;
 */
void device_sleep_ms(uint32_t delay_ms);

/*
 * @brief setup system clocks for peripherals
 */
void device_setup_clocks(void);

/*
 * @brief setup CPU frequency for our device
 */
void device_setup_systick(void);

/*
 * @brief setup our general-purpose I/O
 * used in the device
 */
void device_setup_gpios(void);

/*
 * @brief setup device serial port for
 * host-pc console communicationm used
 * interface - USART(2), 115200 baud
 */
void device_setup_serial(void);

/*
 * @brief send data package via USART
 * interface to host-pc console
 */
void device_send_serial(uint32_t usart, uint8_t *data, uint8_t nbytes);

/*
 * @brief read button state for generating random number
 * this number is used for 'throwing' dice and blink n-tymes
 * @param gpioport - button gpio port
 * @param gpiopin -button gpio pin
 */
void device_dice_btn(uint32_t gpioport, uint16_t gpiopin, usbd_device *usbd_dev);

/*
 * @brief blink four on-board leds serially by one
 * to indicate `something` in program
 */
void device_led_blinking(void);

/*
 * @brief setup adc for joystick (external peripheral)
 */
void device_adc_setup(void);

/*
 * @btief read configured adc channel without DMA
 * or hardware interrupts, naive approach
 * @param channel - ADC channel
 */
uint16_t device_read_adc_naiive(uint8_t channel);

#endif /* MAIN_H_ */

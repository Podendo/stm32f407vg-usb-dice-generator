#include <stdlib.h>
#include <string.h>

#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/adc.h>
#include <libopencm3/stm32/f4/dac.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/nvic.h>
#include <libopencm3/stm32/f4/usart.h>

#include "main.h"
#include "dicethrow.h"


/* message for UART-TTY debugging: */
static const char *mesg = "hello to console!\0";

/* system ticks for timing: */
static volatile uint32_t systick_ms;

/* on-board device stucture with used peripherals */
static struct device mydevice;

/* serial console data buffer and nbytes */
static uint8_t usart_rx_buffer[CONSOLE_BUF_LEN], urxb = 0;


int main(void)
{
	unsigned long randval = 0;

	device_setup_clocks();
	device_setup_gpios();
	device_setup_serial();
	device_setup_systick();

	/*device_adc_setup();*/
	device_sleep_ms(500);

	gpio_set(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);
	device_sleep_ms(500);
	gpio_clear(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);

	/* usbmidi_set_gadget(&mydevice.midi_usb); */
	cons_set_device_usb_cdcacm(&mydevice.cons_usb);

	device_send_serial(USART2, (uint8_t *)mesg, strlen(mesg));

	while (1) {
		usbd_poll(mydevice.cons_usb.usbd_dev);

		/* device_dice_btn(GPIOA, GPIO0, mydevice.midi_usb.usbd_dev); */
	}
	/* program never reaches this point */

	return 0;
}

void device_sleep_ms(uint32_t delay_ms)
{
	uint32_t current_ms = systick_ms;
	while ( (systick_ms - current_ms) < delay_ms ) {
		continue;
	}

	return;
}

void device_setup_clocks(void)
{
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOD);

	rcc_periph_clock_enable(RCC_USART2);
	rcc_periph_clock_enable(RCC_OTGFS);
	rcc_periph_clock_enable(RCC_ADC1);

	return;
}

void device_setup_systick(void)
{
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	systick_set_reload(2100 - 1);

	systick_counter_enable();
	systick_interrupt_enable();

	return;
}

void device_setup_gpios(void)
{
	gpio_clear(GPIOA, GPIO0);
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0);

	gpio_clear(GPIOA, GPIO5);
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO5);

	gpio_clear(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			GPIO12 | GPIO13 | GPIO14 | GPIO15);

	gpio_clear(GPIOA, GPIO11 | GPIO12);
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);

	return;
}

void device_adc_setup(void)
{
	gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO4);
	gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO5);

	adc_power_off(ADC1);
	adc_disable_scan_mode(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC);

	adc_power_on(ADC1);

	return;
}

uint16_t device_read_adc_naiive(uint8_t channel)
{
	uint16_t regs = 0;
	uint8_t channel_array[16];

	channel_array[0] = channel;
	adc_set_regular_sequence(ADC1, 1, channel_array);
	adc_start_conversion_regular(ADC1);

	while ( !adc_eoc(ADC1) ) {
		__asm__("nop");
	}

	regs = adc_read_regular(ADC1);

	return regs;
}

void device_setup_serial(void)
{
	nvic_enable_irq(NVIC_USART2_IRQ);

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO3);

	gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO3);

	gpio_set_af(GPIOA, GPIO_AF7, GPIO2 | GPIO3);

	usart_set_baudrate(USART2, 115200);
	usart_set_databits(USART2, 8);
	usart_set_stopbits(USART2, USART_STOPBITS_1);
	usart_set_mode(USART2, USART_MODE_TX_RX);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

	usart_enable_rx_interrupt(USART2);

	usart_enable(USART2);

	return;
}

void usart2_isr(void)
{
	uint32_t usart_register = 0;

	usart_register = USART_SR(USART2);
	if ( usart_register & USART_SR_RXNE ) {
		usart_rx_buffer[urxb] = USART_DR(USART2);

		if ( urxb == CONSOLE_BUF_LEN ) {
			urxb = 0;
		} else {
			__asm__("nop");
		}

		urxb += 1;
	}

	return;
}

void device_send_serial(uint32_t usart, uint8_t *data, uint8_t nbytes)
{
	uint8_t nbyte = 0;

	for ( nbyte = 0; nbyte < nbytes; ++nbyte ) {
		usart_send_blocking(usart, data[nbyte]);
	}

	return;
}

void device_dice_btn(uint32_t gpioport, uint16_t gpiopin, usbd_device *usbd_dev)
{
	unsigned long seed = 0;
	uint32_t entry_time_ms = systick_ms;
	uint16_t btn_state = 0;

	if ( (btn_state = gpio_get(gpioport, gpiopin)) ) {
		while (systick_ms - entry_time_ms < DEVICE_KEY_DELAY) {
			__asm__("nop");
		}
	} else {
		__asm__("nop");
	}

	if ( btn_state & gpio_get(gpioport, gpiopin) ) {
		dice_init_genrand(systick_ms);
		seed = dice_get_genrand() >> 24;

		while ( seed > 6 ) {
			seed = seed % 6;
		}

		seed = seed + 1;

		while ( seed ) {
			device_send_serial(USART2, (uint8_t *)&seed, 1);
			usbmidi_send_event(usbd_dev, !!btn_state);
			device_led_blinking();
			seed = seed - 1;
		}

		gpio_clear(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);
	} else {
		__asm__("nop");
	}

	return;
}


void device_led_blinking(void)
{
	gpio_set(GPIOD, GPIO12);
	device_sleep_ms(625);
	gpio_set(GPIOD, GPIO13);
	device_sleep_ms(625);
	gpio_set(GPIOD, GPIO14);
	device_sleep_ms(625);
	gpio_set(GPIOD, GPIO15);
	device_sleep_ms(625);

	gpio_clear(GPIOD, GPIO12);
	device_sleep_ms(625);
	gpio_clear(GPIOD, GPIO13);
	device_sleep_ms(625);
	gpio_clear(GPIOD, GPIO14);
	device_sleep_ms(625);
	gpio_clear(GPIOD, GPIO15);
	device_sleep_ms(625);

	return;
}

void sys_tick_handler(void)
{
	systick_ms += 1;

	return;
}

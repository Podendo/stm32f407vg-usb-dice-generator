#ifndef AD7608ADC_H_
#define AD7608ADC_H_

#include <inttypes.h>
#include <libopencm3/stm32/f4/rcc.h>


struct spi_pin_config {
	uint32_t	gpioport;
	uint8_t		mode;
	uint8_t		otype;
	uint8_t		speed;
	uint8_t		alt_func_num;
	uint8_t		pull_up_down;
	uint16_t	gpio;
};

struct spi_ini_config {
	uint32_t	spi_numb;
	uint32_t	baudrate;
	uint32_t	lsbfirst;
	uint32_t	polarity;
	uint32_t	clkphaze;
	uint32_t	dfformat;
};

struct adc_spi_configuration {
	enum rcc_periph_clken	clken;
	struct spi_ini_config	spi_ini_cfg;
	struct spi_pin_config	nss_pin_cfg;
	struct spi_pin_config	clk_pin_cfg;
	struct spi_pin_config	miso_pin_config;
	struct spi_pin_config	mosi_pin_config;

};

struct ad7608adc {
	struct adc_spi_configuration	adc_spi_cfg;
};


void ad7608_setup_adc(struct ad7608adc *adc);

void ad7608_setup_spi(struct adc_spi_configuration *spi);




#endif /* AD7608ADC_H_ */

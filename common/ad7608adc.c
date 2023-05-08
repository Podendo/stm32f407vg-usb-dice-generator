#include "ad7608adc.h"

#include <stdlib.h>
#include <stdlib.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/nvic.h>
#include <libopencm3/stm32/f4/spi.h>

extern void device_sleep_ms(uint32_t delay_ms);

static inline void configure_nss_pin(struct spi_pin_config *nss_pin)
{
	/* configure SPI CS gpio pin (hardware-dependent) */
	nss_pin->gpioport	= GPIOE;
	nss_pin->mode		= GPIO_MODE_OUTPUT;
	nss_pin->pull_up_down	= GPIO_PUPD_NONE;
	nss_pin->gpio		= GPIO3;
	nss_pin->otype		= GPIO_OTYPE_PP;
	nss_pin->speed		= GPIO_OSPEED_50MHZ;

	return;
}

static inline void configure_clk_pin(struct spi_pin_config *clk_pin)
{
	/* configure SPI CLK pin (hardware-dependent) */
	clk_pin->gpioport	= GPIOA;
	clk_pin->mode		= GPIO_MODE_AF;
	clk_pin->pull_up_down	= GPIO_PUPD_PULLDOWN;
	clk_pin->gpio		= GPIO5;
	clk_pin->alt_func_num	= GPIO_AF5;

	return;
}

static inline void
configure_data_pins(struct spi_pin_config *miso, struct spi_pin_config *mosi)
{
	/* configure SPI MOSI pin (hardware-dependent) */
	mosi->gpioport		= GPIOA;
	mosi->mode		= GPIO_MODE_AF;
	mosi->pull_up_down	= GPIO_PUPD_PULLDOWN;
	mosi->gpio		= GPIO7;
	mosi->otype		= GPIO_OTYPE_PP;
	mosi->speed		= GPIO_OSPEED_50MHZ;

	/* configure SPI MISO pin (hardware-dependent) */
	miso->gpioport		= GPIOA;
	miso->mode		= GPIO_MODE_AF;
	miso->pull_up_down	= GPIO_PUPD_PULLDOWN;
	miso->gpio		= GPIO6;
	miso->otype		= GPIO_OTYPE_PP;
	miso->speed		= GPIO_OSPEED_50MHZ;

	return;
}

static inline void configure_spi_params(struct spi_ini_config *cfg)
{
	/*
	 * Set up SPI in Master mode with:
	 * clock baud rate: 168 MHz of peripheral clock frequency
	 * clock polarity: idle high
	 * clock phase: data valid on falling edge
	 * data frame format: 8-bit
	 * 2-frame format: MSB first
	 * in RM page 879 CPHA=1 CPOL=0
	 */
	cfg->spi_numb = SPI1;
	cfg->baudrate = SPI_CR1_BAUDRATE_FPCLK_DIV_256;
	cfg->polarity = SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE;
	cfg->clkphaze = SPI_CR1_CPHA_CLK_TRANSITION_1;
	cfg->dfformat = SPI_CR1_DFF_8BIT;
	cfg->lsbfirst = SPI_CR1_MSBFIRST;

	return;
}

void ad7608_setup_spi(struct adc_spi_configuration *spi)
{
	/* enable SPI peripheral clock for external ADC */
	spi->clken = RCC_SPI1;

	/* set nss-pin parameters structure */
	configure_nss_pin(&spi->nss_pin_cfg);

	/* set up /NSS(CS) pin for SPI interface and set it to hight: */
	gpio_mode_setup(spi->nss_pin_cfg.gpioport, spi->nss_pin_cfg.mode,
			spi->nss_pin_cfg.pull_up_down, spi->nss_pin_cfg.gpio);

	gpio_set_output_options(spi->nss_pin_cfg.gpioport, spi->nss_pin_cfg.otype,
			spi->nss_pin_cfg.speed, spi->nss_pin_cfg.gpio);

	gpio_set(spi->nss_pin_cfg.gpioport, spi->nss_pin_cfg.gpio);

	/* set up clk-pin parameters structure */
	configure_clk_pin(&spi->clk_pin_cfg);

	/* set up SCLK (CLK) pin for SPI interface: */
	gpio_mode_setup(spi->clk_pin_cfg.gpioport, spi->clk_pin_cfg.mode,
			spi->clk_pin_cfg.pull_up_down, spi->clk_pin_cfg.gpio);

	gpio_set_af(spi->clk_pin_cfg.gpioport,
			spi->clk_pin_cfg.alt_func_num, spi->clk_pin_cfg.gpio);

	/* set up mosi/miso pin parameters structures */
	configure_data_pins(&spi->miso_pin_config, &spi->mosi_pin_config);

	/* set up MOSI-MISO (data) pins for SPI interface */
	gpio_mode_setup(spi->mosi_pin_config.gpioport,
			spi->mosi_pin_config.mode,
			spi->mosi_pin_config.pull_up_down,
			spi->mosi_pin_config.gpio);

	gpio_mode_setup(spi->miso_pin_config.gpioport,
			spi->miso_pin_config.mode,
			spi->miso_pin_config.pull_up_down,
			spi->miso_pin_config.gpio);

	gpio_set_output_options(spi->miso_pin_config.gpioport,
			spi->miso_pin_config.otype,
			spi->miso_pin_config.speed,
			spi->miso_pin_config.gpio);

	gpio_set_output_options(spi->mosi_pin_config.gpioport,
			spi->mosi_pin_config.otype,
			spi->mosi_pin_config.speed,
			spi->mosi_pin_config.gpio);

	gpio_set_af(spi->miso_pin_config.gpioport,
			spi->miso_pin_config.alt_func_num, spi->miso_pin_config.gpio);

	gpio_set_af(spi->mosi_pin_config.gpioport,
			spi->mosi_pin_config.alt_func_num, spi->mosi_pin_config.gpio);

	/* set up spi configuration structure */
	configure_spi_params(&spi->spi_ini_cfg);

	/* set up configured SPI interface */
	spi_reset(spi->spi_ini_cfg.spi_numb);
	spi_init_master(spi->spi_ini_cfg.spi_numb, spi->spi_ini_cfg.baudrate,
			spi->spi_ini_cfg.polarity, spi->spi_ini_cfg.clkphaze,
			spi->spi_ini_cfg.dfformat, spi->spi_ini_cfg.lsbfirst);

	spi_enable_software_slave_management(spi->spi_ini_cfg.spi_numb);
	spi_enable_ss_output(spi->spi_ini_cfg.spi_numb);
	spi_enable(spi->spi_ini_cfg.spi_numb);

	return;
}

void ad7608_setup_adc(struct ad7608adc *adc)
{
	ad7608_setup_spi(&adc->adc_spi_cfg);
	return;
}

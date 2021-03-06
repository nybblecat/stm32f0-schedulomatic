#include "lowlevel.h"

/********* Low_init *******
*  Attends to initialization of clocks, pins, and peripherals.
*   Inputs: none
*  Outputs: none
*/
void Low_init(void) {
	rcc_init();
	gpio_init();

}

/********* rcc_init *******
*  Enables the clock for GPIO and other peripherals.
*   Inputs: none
*  Outputs: none
*/
void rcc_init(void) {
	rcc_clock_setup_in_hsi_out_48mhz();
	/*  rcc_clock_setup_in_hse_8mhz_out_48mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_DMA);
	rcc_periph_clock_enable(RCC_SPI1);
	rcc_periph_clock_enable(RCC_USART2);
}

/********* gpio_init *******
*  Initializes the GPIO registers and sets up peripheral and other output pins.
*   Inputs: none
*  Outputs: none
*/
void gpio_init(void) {

	/* USART peripheral for serial debug via /tty/ACMx */
	gpio_mode_setup( GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO3 );
	gpio_set_af( GPIOA, GPIO_AF1, GPIO2 | GPIO3 );
	gpio_set_output_options( GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO2 | GPIO3 );

	/* LED for debugging purposes */
	gpio_mode_setup( PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_LED );
	gpio_set_output_options( PORT_LED, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_LED );

	/* Lines for timing/debugging measurements */
	gpio_mode_setup( PORT_SIG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_SIG0 );
	gpio_mode_setup( PORT_SIG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_SIG1 );
	gpio_mode_setup( PORT_SIG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_SIG2 );
	gpio_mode_setup( PORT_SIG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_SIG3 );

	gpio_set_output_options( PORT_SIG, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_SIG0 );
	gpio_set_output_options( PORT_SIG, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_SIG1 );
	gpio_set_output_options( PORT_SIG, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_SIG2 );
	gpio_set_output_options( PORT_SIG, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_SIG3 );

	/* SPI peripheral pins - Simplex transmission with NSS output */
	gpio_mode_setup( PORT_SPI, GPIO_MODE_AF, GPIO_PUPD_NONE, MOSI | SCK | NSS );
	gpio_set_af( PORT_SPI, GPIO_AF0, MOSI | SCK | NSS );
	gpio_set_output_options( PORT_SPI, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, MOSI | SCK | NSS );
	/* external pullup */
	gpio_set_output_options( PORT_SPI, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, NSS );

	/* Additional pins for OLED display: Reset (RES) */
	gpio_mode_setup( PORT_OLED, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, RST );
	/* external pullup */
	gpio_set_output_options( PORT_OLED, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, RST );

	/* Additional pins for OLED display: Data/Command Select (DC) */
	gpio_mode_setup( PORT_OLED, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, DC );
	gpio_set_output_options( PORT_OLED, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, DC );
}


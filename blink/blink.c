/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 * Copyright (c) 2015 Chuck McManis <cmcmanis@mcmanis.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define CODE_KIND_MANUALLY_HARDCODED 0
#define CODE_KIND_MANUALLY_CONSTANTS 1
#define CODE_KIND_API_BASE 2
#define CODE_KIND_API_EXTENDED 3

#define CODE_KIND CODE_KIND_API_EXTENDED

static void gpio_setup(void)
{
	/* Enable GPIOA clock. */
#if CODE_KIND == CODE_KIND_MANUALLY_HARDCODED
	/* Manually: */
	RCC_AHB1ENR |= 0x01;
#elif CODE_KIND == CODE_KIND_MANUALLY_CONSTANTS
	/* Manually: */
	RCC_AHB1ENR |= RCC_AHB1ENR_IOPAEN;
#else
	/* Using API functions: */
	rcc_periph_clock_enable(RCC_GPIOA);
#endif

	/* Set GPIO5 (in GPIO port A) to 'output push-pull'. */
#if CODE_KIND == CODE_KIND_MANUALLY_HARDCODED
	/* Manually: */
	GPIOA_MODER = (GPIOA_MODER & 0xFFFFF3FF) | 0x00000400;
	GPIOA_OTYPER &= ~(0x1<<5);
	GPIOA_OSPEEDR |= 0x03<<10;
	GPIOA_PUPDR &= 0xFFFFF3FF;
#elif CODE_KIND == CODE_KIND_MANUALLY_CONSTANTS
	/* Manually: */
	GPIOA_MODER = (GPIOA_MODER & ~GPIO_MODE_MASK(5)) | GPIO_MODE(5, GPIO_MODE_OUTPUT);
	GPIOA_OTYPER = (GPIOA_OTYPER & ~GPIO5) | (GPIO_OTYPE_PP << 5);
	GPIOA_OSPEEDR = (GPIOA_OSPEEDR & ~GPIO_OSPEED_MASK(5)) | GPIO_OSPEED(5, GPIO_OSPEED_100MHZ);
	GPIOA_PUPDR = (GPIOA_PUPDR & ~GPIO_PUPD_MASK(5)) | GPIO_PUPD(5, GPIO_PUPD_NONE);
#else
	/* Using API functions: */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO5);
#endif
}

int main(void)
{
	int i;

	gpio_setup();

	/* Blink the LED (PC8) on the board. */
	while (1) {
#if CODE_KIND == CODE_KIND_MANUALLY_HARDCODED
		/* Manually: */
		GPIOA_ODR |= 1 << 5;		/* LED on */
		for (i = 0; i < 1000000; i++)	/* Wait a bit. */
			__asm__("nop");
		GPIOA_ODR &= ~(1<<5);		/* LED off */
		for (i = 0; i < 1000000; i++)	/* Wait a bit. */
			__asm__("nop");

#elif CODE_KIND == CODE_KIND_MANUALLY_CONSTANTS
		/* Manually: */
		GPIOA_BSRR = GPIO5;		/* LED on */
		for (i = 0; i < 1000000; i++)	/* Wait a bit. */
			__asm__("nop");
		GPIOA_BSRR = GPIO5 << 16;	/* LED off */
		for (i = 0; i < 1000000; i++)	/* Wait a bit. */
			__asm__("nop");

#elif CODE_KIND == CODE_KIND_API_BASE
		/* Using API functions gpio_set()/gpio_clear(): */
		gpio_set(GPIOA, GPIO5);		/* LED on */
		for (i = 0; i < 1000000; i++)	/* Wait a bit. */
			__asm__("nop");
		gpio_clear(GPIOA, GPIO5);	/* LED off */
		for (i = 0; i < 1000000; i++)	/* Wait a bit. */
			__asm__("nop");

#else
		/* Using API function gpio_toggle(): */
		gpio_toggle(GPIOA, GPIO5);	/* LED on/off */
		for (i = 0; i < 1000000; i++) {	/* Wait a bit. */
			__asm__("nop");
		}
#endif
	}

	return 0;
}

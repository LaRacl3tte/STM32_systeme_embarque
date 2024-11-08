#include "sys/clock.h"
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencmsis/core_cm3.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

void init_LD2(void)
{
    /* on positionne ce qu'il faut dans les différents
       registres concernés */
    RCC_AHB1ENR |= 0x01;                                   // Activation du GPIO
    GPIOA_MODER = (GPIOA_MODER & 0xFFFFF3FF) | 0x00000400; // On mets GPIOA_MODER à 01 : General purpose output
    GPIOA_OTYPER &= ~(0x1 << 5);
    GPIOA_OSPEEDR |= 0x03 << 10;
    GPIOA_PUPDR &= 0xFFFFF3FF;
}

void init_PB(void)
{
    RCC_AHB1ENR |= 0x01 << 2;                 // Activation du GPIOA
    GPIOC_MODER = (GPIOC_MODER & 0xF3FFFFFF); // On mets GPIOC_MODER à 00 : General purpose input
}
/* Ne pas optimiser cette fonction : on dépend du nombre
 * approximatif d'instructions qu'elle exécute
 */
__attribute__((optimize("O0"))) void tempo_500ms(void)
{
    volatile uint32_t duree;
    /* estimation, suppose que le compilateur n'optimise pas trop... */
    for (duree = 0; duree < 5600000; duree++)
    {
        ;
    }
}

void init_USART(void)
{
    GPIOA_MODER = (GPIOA_MODER & 0xFFFFFF0F) | 0x000000A0;
    GPIOA_AFRL = (GPIOA_AFRL & 0xFFFF00FF) | 0x00007700;
    USART2_BRR = get_APB1CLK() / 9600;
    USART2_CR3 = 0;
    USART2_CR2 = 0;
}

void _putc(const char c)
{
    while ((USART2_SR & 0x80) == 0)
        ;
    USART2_DR = c;
}

void _puts(const char *c)
{
    int len = strlen(c);
    for (int i = 0; i < len; i++)
    {
        _putc(c[i]);
    }
}

char _getc(void)
{
    /* À compléter */
    return 0;
}

int main(void)
{
    printf("\e[2J\e[1;1H\r\n");
    printf("\e[01;32m*** Welcome to Nucleo F446 ! ***\e[00m\r\n");

    printf("\e[01;31m\t%08lx-%08lx-%08lx\e[00m\r\n", DESIG_UNIQUE_ID0,
           DESIG_UNIQUE_ID1, DESIG_UNIQUE_ID2);
    printf("SYSCLK = %9lu Hz\r\n", get_SYSCLK());
    printf("AHBCLK = %9lu Hz\r\n", get_AHBCLK());
    printf("APB1CLK= %9lu Hz\r\n", get_APB1CLK());
    printf("APB2CLK= %9lu Hz\r\n", get_APB2CLK());
    printf("\r\n");

    init_LD2();
    init_PB();

    // while (1)
    // {
    //     if (GPIOC_IDR & (1 << 13)) {
    //         GPIOA_ODR &= ~(1 << 5);
    //     }
    //     else {
    //         GPIOA_ODR |= 1 << 5;
    //     }
    // }

    while(1){
        GPIOA_BSRR = GPIO5;           
        tempo_500ms();
        GPIOA_BSRR = GPIO5 << 16;     
        tempo_500ms();
    }
    
    return 0;
}

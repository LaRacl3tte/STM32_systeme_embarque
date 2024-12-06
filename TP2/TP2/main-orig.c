#include "sys/clock.h"
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/stm32/usart.h>
#include <libopencmsis/core_cm3.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef USE_RAW_REGISTER
#define USE_RAW_REGISTER 1
#endif

void init_LD2(void) {
#if USE_RAW_REGISTER
    /* on positionne ce qu'il faut dans les différents
       registres concernés */
    RCC_AHB1ENR |= 0x01;
    GPIOA_MODER = (GPIOA_MODER & 0xFFFFF3FF) | 0x00000400;
    GPIOA_OTYPER &= ~(0x1 << 5);
    GPIOA_OSPEEDR |= 0x03 << 10;
    GPIOA_PUPDR &= 0xFFFFF3FF;
#else
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);
#endif
}

void init_BP(void) {
    /* GPIOC_MODER = ... */
}

void button_irqPC13_init(void) {
#if USE_RAW_REGISTER
    RCC_APB2ENR |= 1<<14;
    /* set PC13 as EXTI13 input */
    SYSCFG_EXTICR4 = (SYSCFG_EXTICR4 & ~(0xf << 4)) | (0x2 << 4);
    /* Setup interrupt for EXTI13, falling edge */
    EXTI_IMR |= (1 << 13);
    EXTI_RTSR &= ~(1 << 13);
    EXTI_FTSR |= (1 << 13);
    EXTI_PR |= (1 << 13);
    /* enable EXTI15-10 IRQ PC13 */
    NVIC_ISER(40 / 32) = (1 << (40 % 32));
#else
    rcc_periph_clock_enable(RCC_SYSCFG);

    exti_select_source(EXTI13, GPIOC);

    exti_enable_request(EXTI13);
    exti_set_trigger(EXTI13, EXTI_TRIGGER_FALLING);
    exti_reset_request(EXTI13);

    nvic_enable_irq(NVIC_EXTI15_10_IRQ);
    nvic_set_priority(NVIC_EXTI15_10_IRQ, 200);
#endif
}

/* Ne pas optimiser cette fonction : on dépend du nombre
 * approximatif d'instructions qu'elle exécute
 */
__attribute__((optimize("O0"))) void tempo_500ms(void) {
    volatile uint32_t duree;
    /* estimation, suppose que le compilateur n'optimise pas trop... */
    for (duree = 0; duree < 5600000; duree++) {
        ;
    }
}

/* fonction agissant comme une barrière pour les accès mémoires */
inline static void barriere(void) { __asm__ volatile("" ::: "memory"); }

void init_USART(void) {
#if USE_RAW_REGISTER
    RCC_AHB1ENR |= 0x01;
    GPIOA_MODER = (GPIOA_MODER & 0xFFFFFF0F) | 0x000000A0;
    GPIOA_AFRL = (GPIOA_AFRL & 0xFFFF00FF) | 0x00007700;
    USART2_BRR = get_APB1CLK() / 9600;
    USART2_CR3 = 0;
    USART2_CR2 = 0;
    USART2_CR1 = (1 << 2) | (1 << 3) | (1 << 13);
#else
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART2);
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO3);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2 | GPIO3);

    usart_set_baudrate(USART2, 9600);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

    usart_enable(USART2);
#endif
}

/* Initialisation du timer système (systick) */
/* La fréquence en entrée est en Hz.
 * Elle doit être >= 11Hz du fait de la taille du registre LOAD */
void systick_init(uint32_t freq) {
#if USE_RAW_REGISTER
    /* ATTENTION: la valeur calculée doit tenir dans le registre sur 24 bits */
    uint32_t p = get_SYSCLK() / freq;
    STK_RVR = (p - 1) & 0x00FFFFFF;
    STK_CVR = 0;
    STK_CSR |= STK_CSR_CLKSOURCE | STK_CSR_TICKINT | STK_CSR_ENABLE;
#else
    /* De manière équivalente, mais en passant par l'API

       Note: l'API gère la configuration où la fréquence de l'horloge
       source divisé par 8, on peut alors descendre jusqu'à une
       fréquence de 2Hz (au lieu de 11Hz).
     */
    systick_set_frequency(freq, get_SYSCLK());
    systick_counter_enable();
    /* activation des interruptions en dernier */
    systick_interrupt_enable();
#endif
}

/* La liste des traitants pré-configurés avec libopencm3 est disponible ici :
   http://libopencm3.org/docs/latest/stm32f4/html/group__CM3__nvic__isrprototypes__STM32F4.html
   en plus des traitants de bases visibles ici :
   http://libopencm3.org/docs/latest/stm32f4/html/group__CM3__nvic__defines.html

   Vous pourrez être particulièrement intéressés par
   - void sys_tick_handler(void)
   - void exti15_10_isr(void)
   - void usart2_isr(void)
*/

/* L'attibut "interrupt" n'est plus nécessaire sur l'architecture
 * ARM-Cortex-M4 : Les fonctions d'interruption respectent les
 * conventions d'appel de la plateforme.  Voir par exemple le
 * paragraphe "Exception Entry & Exit" de
 * https://interrupt.memfault.com/blog/arm-cortex-m-exceptions-and-nvic
 */
// __attribute__((interrupt))
void sys_tick_handler() {
    /* Le fait de définir cette fonction suffit pour
     * qu'elle soit utilisée comme traitant,
     * cf les fichiers de compilation et d'édition de lien
     * pour plus de détails.
     */
    /* ... */
}

void _putc(const char c) {
    while ((USART2_SR & 0x80) == 0)
        ;
    USART2_DR = c;
}

void _puts(const char *c) {
    int len = strlen(c);
    for (int i = 0; i < len; i++) {
        _putc(c[i]);
    }
}

char _getc(void) {
    /* À compléter */
    return 0;
}


/* Fonction non bloquante envoyant une chaîne par l'UART */
int _async_puts(const char *s) {
    /* Cette fonction doit utiliser un traitant d'interruption
     * pour gérer l'envoi de la chaîne s (qui doit rester
     * valide pendant tout l'envoi). Elle doit donc être
     * non bloquante (pas d'attente active non plus) et
     * renvoyer 0.
     *
     * Si une chaîne est déjà en cours d'envoi, cette
     * fonction doit renvoyer 1 (et ignorer la nouvelle
     * chaîne).
     *
     * Si s est NULL, le code de retour permet de savoir
     * si une chaîne est encore en cours d'envoi ou si
     * une nouvelle chaîne peut être envoyée.
     */
    /* À compléter */
    (void)s; // to avoid unused paramater warning
    return 0;
}

static volatile uint32_t mode = 0;

void exti15_10_isr() {
#if USE_RAW_REGISTER
    EXTI_PR |= (1 << 13); /* Clear pending register EXTIPR[13] */
#else
    exti_reset_request(EXTI13);
#endif
    mode = mode ^ 1; /* switch the mode value 0 <-> 1 */
}

int main(void) {
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

    while (1)
        ;
    return 0;
}

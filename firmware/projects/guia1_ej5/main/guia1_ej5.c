/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 25/03/2026 | Document creation		                         |
 *
 * @author Houriet Celina
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
typedef struct{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

gpioConf_t gpio_vector[4] = {
    {GPIO_20, GPIO_OUTPUT},  // b0 -> GPIO_20
    {GPIO_21, GPIO_OUTPUT},  // b1 -> GPIO_21
    {GPIO_22, GPIO_OUTPUT},  // b2 -> GPIO_22
    {GPIO_23, GPIO_OUTPUT}   // b3 -> GPIO_23
};

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void deBCDaGPIO(uint8_t bcd, gpioConf_t *gpio_vector) {
    for(int i = 0; i < 4; i++) {
        int bit = (bcd >> i) & 1;  // Extraer el bit i del BCD
        if(bit) {
            GPIOOn(gpio_vector[i].pin);  // Setear GPIO a 1
        } else {
            GPIOOff(gpio_vector[i].pin); // Setear GPIO a 0
        }
    }
}

void app_main(void) {
    for(int i = 0; i < 4; i++) {
        GPIOInit(gpio_vector[i].pin, gpio_vector[i].dir);
    }
    uint8_t bcd = 5;
    printf("Procesando BCD: %d\n", bcd);
    for(int i = 0; i < 4; i++) {
        int bit = (bcd >> i) & 1;
        printf("Bit %d (GPIO_%d): %d\n", i, gpio_vector[i].pin, bit);
    }
    deBCDaGPIO(bcd, gpio_vector);
    printf("GPIOs configurados según BCD.\n");
}

/*==================[end of file]============================================*/
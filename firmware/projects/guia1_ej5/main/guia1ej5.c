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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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

gpioConf_t digit_vector[3] = {
    {GPIO_19, GPIO_OUTPUT},  // Dígito 1
    {GPIO_18, GPIO_OUTPUT},  // Dígito 2
    {GPIO_9, GPIO_OUTPUT}    // Dígito 3
};

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number);
void deBCDaGPIO(uint8_t bcd, gpioConf_t *gpio_vector);
void mostrarEnDisplay(uint32_t data, uint8_t digits, gpioConf_t *segment_vector, gpioConf_t *digit_vector);

/*==================[external functions definition]==========================*/
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number) {
    for (int8_t i = digits - 1; i >= 0; i--) {
        bcd_number[i] = data % 10;
        data /= 10;
    }
    return 0;
}

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

void mostrarEnDisplay(uint32_t data, uint8_t digits, gpioConf_t *segment_vector, gpioConf_t *digit_vector) {
    uint8_t bcd_array[digits];
    convertToBcdArray(data, digits, bcd_array);
    while(1) {
        for(uint8_t d = 0; d < digits; d++) {
            // Apagar todos los dígitos
            for(uint8_t i = 0; i < digits; i++) {
                GPIOOff(digit_vector[i].pin);
            }
            // Setear segmentos para el dígito d
            deBCDaGPIO(bcd_array[d], segment_vector);
            // Encender el dígito d
            GPIOOn(digit_vector[d].pin);
            // Delay para multiplexado
            vTaskDelay(5 / portTICK_PERIOD_MS);
        }
    }
}

void app_main(void) {
    // Configurar GPIOs para segmentos
    for(int i = 0; i < 4; i++) {
        GPIOInit(gpio_vector[i].pin, gpio_vector[i].dir);
    }
    // Configurar GPIOs para dígitos
    for(int i = 0; i < 3; i++) {
        GPIOInit(digit_vector[i].pin, digit_vector[i].dir);
    }
    uint32_t data = 123;
    uint8_t digits = 3;
    printf("Mostrando %lu en display de %d dígitos\n", data, digits);
    mostrarEnDisplay(data, digits, gpio_vector, digit_vector);
}
/*==================[end of file]============================================*/
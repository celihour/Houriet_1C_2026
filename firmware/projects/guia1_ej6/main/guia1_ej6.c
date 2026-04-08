/*! @mainpage Conversión a BCD y visualización en display de 7 segmentos
 *
 * @section genDesc General Description
 *
 * Se implementa la conversión de un número decimal a formato BCD y su posterior
 * visualización en un display de 7 segmentos multiplexado.
 *
 * El funcionamiento consiste en:
 * - Convertir un número decimal a sus dígitos individuales.
 * - Representar cada dígito en formato BCD utilizando 4 GPIO.
 * - Seleccionar cada dígito del display mediante líneas SEL para multiplexación.
 *
 * @section hardConn Hardware Connection
 *
 * El display de 7 segmentos está conectado al ESP32 de la siguiente manera:
 *
 * | Peripheral | ESP32   |
 * |:----------:|:-------:|
 * | D1         | GPIO_20 |
 * | D2         | GPIO_21 |
 * | D3         | GPIO_22 |
 * | D4         | GPIO_23 |
 * | SEL_1      | GPIO_19 |
 * | SEL_2      | GPIO_18 |
 * | SEL_3      | GPIO_9  |
 * | +5V        | +5V     |
 * | GND        | GND     |
 *
 * @section changelog Changelog
 *
 * | Date       | Description                                               |
 * |:----------:|:----------------------------------------------------------|
 * | 08/04/2026 | Conversión decimal a BCD y visualización en display.       |
 *
 * @author Houriet Celina celina.houriet@ingenieria.uner.edu.ar
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

/**
 * @brief Estructura para configuración de pines GPIO.
 */
typedef struct{
    gpio_t pin;          /*!< GPIO pin number */
    io_t dir;            /*!< GPIO direction */
} gpioConf_t;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Convierte un número decimal en un arreglo de dígitos.
 *
 * @param data Número decimal a convertir.
 * @param digits Cantidad de dígitos a obtener.
 * @param bcd_number Arreglo donde se almacenan los dígitos obtenidos.
 *
 * @return int8_t Retorna 0 si la conversión fue exitosa.
 */
int8_t convertirBCD(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
    for(uint8_t i = 0; i < digits; i++)
    {
        bcd_number[digits - i - 1] = data % 10;
        data = data / 10;
    }
    return 0;
}

/**
 * @brief Configura los GPIO de datos según el valor BCD recibido.
 *
 * @param numero Dígito a representar (0 a 9).
 * @param gpio_bcd Vector con los 4 GPIO correspondientes a D1-D4.
 */
void deBCDaGPIO(uint8_t numero, gpioConf_t *gpio_bcd)
{
    for(uint8_t i = 0; i < 4; i++)
    {
        uint8_t bit = (numero >> i) & 0x01;

        if(bit)
        {
            GPIOOn(gpio_bcd[i].pin);
        }
        else
        {
            GPIOOff(gpio_bcd[i].pin);
        }
    }
}

/**
 * @brief Muestra un número decimal en un display multiplexado de 3 dígitos.
 *
 * @param data Número decimal a mostrar.
 * @param digits Cantidad de dígitos del display.
 * @param gpio_bcd Vector con los GPIO de datos (D1-D4).
 * @param gpio_sel Vector con los GPIO selectores (SEL_1, SEL_2, SEL_3).
 */
void mostrarEnDisplay(uint32_t data, uint8_t digits, gpioConf_t *gpio_bcd, gpioConf_t *gpio_sel)
{
    uint8_t digitos[digits];
    convertirBCD(data, digits, digitos);

    while(1)
    {
        for(uint8_t i = 0; i < digits; i++)
        {
            for(uint8_t j = 0; j < digits; j++)
            {
                GPIOOff(gpio_sel[j].pin);
            }

            deBCDaGPIO(digitos[i], gpio_bcd);
            GPIOOn(gpio_sel[i].pin);

            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

/*==================[main]===================================================*/

/**
 * @brief Función principal del programa.
 */
void app_main(void)
{
    gpioConf_t gpio_bcd[4] = {
        {GPIO_20, GPIO_OUTPUT},
        {GPIO_21, GPIO_OUTPUT},
        {GPIO_22, GPIO_OUTPUT},
        {GPIO_23, GPIO_OUTPUT}
    };

    gpioConf_t gpio_sel[3] = {
        {GPIO_19, GPIO_OUTPUT},
        {GPIO_18, GPIO_OUTPUT},
        {GPIO_9,  GPIO_OUTPUT}
    };

    for(uint8_t i = 0; i < 4; i++)
    {
        GPIOInit(gpio_bcd[i].pin, gpio_bcd[i].dir);
    }

    for(uint8_t i = 0; i < 3; i++)
    {
        GPIOInit(gpio_sel[i].pin, gpio_sel[i].dir);
    }

    mostrarEnDisplay(123, 3, gpio_bcd, gpio_sel);
}

/*==================[end of file]============================================*/
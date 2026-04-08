/*! @mainpage Conversión a BCD y visualización en display de 7 segmentos
 *
 * @section genDesc General Description
 *
 * Se implementa la conversión de un número decimal a formato BCD 
 * y su visualización en un display de 7 segmentos.
 *
 * El funcionamiento básico es:
 * - Convertir un número decimal a sus dígitos individuales en BCD.
 * - Mostrar cada dígito en el display
 * - Utilizar pines GPIO para controlar los segmentos del display y la selección
 *   de dígitos.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * El display de 7 segmentos está conectado al ESP32 de la siguiente manera:
 *
 * |    Peripheral  |   ESP32   	| 
 * |:--------------:|:--------------|
 * | 	D1	 	| 	GPIO_20		|
 * | 	D2	 	| 	GPIO_21		| 
 * | 	D3	 	| 	GPIO_22		| 
 * | 	D4	 	| 	GPIO_23		| 
 * | 	SEL_1	 | 	GPIO_19		| 
 * | 	SEL_3	 | 	GPIO_9		| 
 * | 	+5V	 	| 	+5V		    |
 * | 	GND	 	| 	GND		    | 
 *
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 08/04/2026 | Convertimos de decimal a BCD y lo mostramos en un display de 7 segmentos|
 *
 * @author Houriet Celina celina.houriet@ingenieria.uner.edu.ar
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

/**
 * @brief Estructura de configuración para pines GPIO
 *
 * Esta estructura define la configuración de un pin GPIO, incluyendo el número del pin
 * y su dirección (entrada o salida).
 */
typedef struct{
    gpio_t pin;			/*!< GPIO pin number */
    io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/**
 * @brief Convierte un número decimal a un arreglo BCD
 *
 * Esta función toma un número decimal y lo convierte a su representación BCD
 * en un arreglo de dígitos.
 *
 * @param data Número decimal a convertir
 * @param digits Número de dígitos en el arreglo BCD 
 * @param bcd_number Arreglo donde se almacenarán los dígitos BCD 
 * 
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
 * @brief Escribe un dígito BCD en los pines GPIO correspondientes
 *
 * Esta función configura los pines GPIO para mostrar un dígito en un display
 * de 7 segmentos usando codificación BCD. Cada bit del número se asigna a un pin GPIO.
 *
 * @param numero Dígito BCD a mostrar 
 * @param gpio_bcd Arreglo de 4 configuraciones GPIO para los bits BCD 
 *
 */
void deBCDaGPIO(uint8_t numero, gpioConf_t *gpio_bcd)
{
    for(uint8_t i = 0; i < 4; i++)
    {
        uint8_t bit = (numero >> i) & 0x01;

        if(bit)
            GPIOOn(gpio_bcd[i].pin);
        else
            GPIOOff(gpio_bcd[i].pin);
    }
}

/**
 * @brief Muestra un número en el display de 7 segmentos 
 *
 * Esta función convierte un número decimal a BCD y lo muestra en un display
 * de 7 segmentos multiplexado, alternando entre los dígitos con un delay de 5ms
 * La función entra en un bucle infinito para mantener la visualización continua
 *
 * @param data Número decimal a mostrar 
 * @param digits Número de dígitos del display 
 * @param gpio_bcd Arreglo de 4 configuraciones GPIO para los bits BCD
 * @param gpio_sel Arreglo de configuraciones GPIO para la selección de dígitos
 *
 * @note Esta función no retorna; entra en un bucle infinito.
 * @warning Asegúrese de que 'gpio_bcd' tenga 4 elementos y 'gpio_sel' tenga 'digits' elementos.
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
 * @brief Función principal de la aplicación
 *
 * Esta función inicializa los pines GPIO para el display de 7 segmentos,
 * configura 4 pines para los bits BCD y 3 pines para la selección de dígitos,
 * y luego llama a mostrarEnDisplay para visualizar el número elegido.
 *
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
        GPIOInit(gpio_bcd[i].pin, gpio_bcd[i].dir);

    for(uint8_t i = 0; i < 3; i++)
        GPIOInit(gpio_sel[i].pin, gpio_sel[i].dir);

    mostrarEnDisplay(982, 3, gpio_bcd, gpio_sel);
}
/*==================[external functions definition]==========================*/

/*==================[end of file]============================================*/
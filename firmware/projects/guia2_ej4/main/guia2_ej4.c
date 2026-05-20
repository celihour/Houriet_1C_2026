/*! @mainpage Guía 2 - Ejercicio 4
 *
 * @section Convertidor de señal analógica a digital
 *
 * Se digitaliza una señal analógica utilizando el ADC (CH1) de la placa ESP-EDU.
 * La conversión se dispara mediante una interrupción periódica de Timer.
 * La frecuencia de muestreo utilizada es 500 Hz (periodo = 2 ms).
 *
 * Los valores digitalizados se envían a la PC mediante UART en formato ASCII.
 *
 * @section hardConn Hardware Connection
 *
 * | Peripheral | ESP32 |
 * |:----------:|:-----:|
 * |   CH1 ADC  | CH1   |
 *
 * @section changelog Changelog
 *
 * |   Date     | Description          |
 * |:----------:|:---------------------|
 * | 13/05/2026 | Document creation    |
 *
 * @author Celina Houriet
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/
#define FRECUENCIA_MUESTREO_HZ   500
#define PERIODO_MUESTREO_A      2000
#define PERIODO_MUESTREO_B      20*1000
#define ECG_LENGTH (sizeof(ecg_data) / sizeof(ecg_data[0]))

/*==================[internal data definition]===============================*/
volatile uint16_t dato_adc = 0;
static uint16_t ecg_index = 0;

TaskHandle_t envio_uart = NULL;
TaskHandle_t salida_dac = NULL;

static const uint8_t ecg_data[] = {
    17,17,17,17,17,17,17,17,17,17,17,18,18,18,17,17,17,17,17,17,17,18,18,18,18,18,18,18,17,17,16,16,16,16,17,17,18,18,18,17,17,17,17,
18,18,19,21,22,24,25,26,27,28,29,31,32,33,34,34,35,37,38,37,34,29,24,19,15,14,15,16,17,17,17,16,15,14,13,13,13,13,13,13,13,12,12,
10,6,2,3,15,43,88,145,199,237,252,242,211,167,117,70,35,16,14,22,32,38,37,32,27,24,24,26,27,28,28,27,28,28,30,31,31,31,32,33,34,36,
38,39,40,41,42,43,45,47,49,51,53,55,57,60,62,65,68,71,75,79,83,87,92,97,101,106,111,116,121,125,129,133,136,138,139,140,140,139,137,
133,129,123,117,109,101,92,84,77,70,64,58,52,47,42,39,36,34,31,30,28,27,26,25,25,25,25,25,25,25,25,24,24,24,24,25,25,25,25,25,25,25,
24,24,24,24,24,24,24,24,23,23,22,22,21,21,21,20,20,20,20,20,19,19,18,18,18,19,19,19,19,18,17,17,18,18,18,18,18,18,18,18,17,17,17,17,
17,17,17
};
/*==================[internal functions definition]==========================*/
/**
 * @brief Dispara la lectura del ADC y envío por UART.
 * Se ejecuta cada PERIODO_MUESTREO_US (2 ms, 500 Hz).
 * Notifica a TareaEnvioUART mediante task notification.
 */
void InterrupcionTimerA(void *param)
{
  vTaskNotifyGiveFromISR(envio_uart, NULL);
}

/**
 * @brief Dispara la reproducción del ECG por DAC.
 * Se ejecuta cada PERIODO_MUESTREO_US (2 ms, 500 Hz).
 * Notifica a TareaSalidaDAC mediante task notification.
 */
void InterrupcionTimerB(void *param)
{
  vTaskNotifyGiveFromISR(salida_dac, NULL);
}

/**
 * @brief Tarea que lee CH1 del ADC y envía el valor por UART al graficador.
 * Queda bloqueada hasta recibir notificación de InterrupcionTimerA.
 */
void TareaEnvioUART(void *param)
{
	while(1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &dato_adc);
		UartSendString(UART_PC, ">ch1:");
		UartSendString(UART_PC, (char*)UartItoa(dato_adc, 10));
		UartSendString(UART_PC, "\r\n");
	}
}

/**
 * @brief Tarea que escribe la señal de ECG en el DAC de forma cíclica.
 * Queda bloqueada hasta recibir notificación de InterrupcionTimerB.
 */
void TareaSalidaDAC(void *param)
{
    while(1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogOutputWrite(ecg_data[ecg_index]);
        ecg_index++;
        if(ecg_index >= ECG_LENGTH)
            ecg_index = 0;
    }
}

/*==================[external functions definition]==========================*/
/**
 * @brief Función principal de la aplicación.
 *
 * Inicializa ADC (CH1), DAC, UART (115200 baud), Timer A y Timer B,
 * crea las tareas FreeRTOS y arranca los timers.
 */
void app_main(void)
{
    analog_input_config_t adc_config = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0
    };
    AnalogInputInit(&adc_config);

    AnalogOutputInit();

    serial_config_t uart_config = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL
    };
    UartInit(&uart_config);

    timer_config_t timer_a_config = {
        .timer = TIMER_A,
        .period = PERIODO_MUESTREO_A,
        .func_p = InterrupcionTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_a_config);

    timer_config_t timer_b_config = {
        .timer = TIMER_B,
        .period = PERIODO_MUESTREO_B,
        .func_p = InterrupcionTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_b_config);

    xTaskCreate(TareaEnvioUART, "EnvioUART", 2048, NULL, 5, &envio_uart);
    xTaskCreate(TareaSalidaDAC, "SalidaDAC", 2048, NULL, 5, &salida_dac);

    TimerStart(TIMER_A);
    TimerStart(TIMER_B);
}
/*==================[end of file]============================================*/
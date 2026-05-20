/**
 * @file guia2_ej3.c
 * @brief Actividad 3 - Medidor de distancia por ultrasonido con puerto serie.
 * 
 * @mainpage Medidor de distancia con interrupciones y puerto serie.
 *
 * @section genDesc 
 *
 * El firmware mide la distancia con un sensor HC-SR04 y muestra el valor en cm
 * en un display numérico de 3 dígitos. Además utiliza los LEDs de la placa
 * para indicar el rango de distancia. Envía los datos de medición por puerto serie
 * en formato: 3 dígitos + espacio + "cm" + "\r\n".
 *
 * @section controls Controles
 *
 * - TEC1 (SWITCH_1 / GPIO_4): activar / detener medición en el momento de la interrupción.
 * - TEC2 (SWITCH_2 / GPIO_15): mantener el resultado en HOLD en el momento de la interrupción.
 * - Puerto serie: "O" para activar/detener medición, "H" para HOLD.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral    |   ESP-EDU Board |
 * |:----------------:|:----------------:|
 * | ECHO             | GPIO_3           |
 * | TRIGGER          | GPIO_2           |
 * | +5V              | +5V              |
 * |  GND  			  | GND              |  
 *
 * @section changelog Changelog
 *
 * |   Date    | Description                                                       |
 * |:----------:|:-----------------------------------------------------------------|
 * | 15/04/2026 | Implementación de medición, LEDs y HOLD con interrupciones       |
 * | 06/05/2026 | Agregado puerto serie para envío de datos y control remoto      |
 *
 * @author Houriet Celina (celina.houriet@ingenieria.uner.edu.ar)
 */
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "timer_mcu.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
#define SENSOR_ECHO_PIN        GPIO_3
#define SENSOR_TRIGGER_PIN     GPIO_2
#define PERIODO_MEDICION_US 1000000

/*==================[internal data definition]===============================*/
/** @brief Indica si la medición está activa o detenida (controlada con TEC1). */
bool medir = true;

/** @brief Indica si el sistema está en modo HOLD (controlado con TEC2). */
bool hold = false;

/** @brief Handle de la tarea de control. */
TaskHandle_t control_tarea= NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Actualiza el estado de los LEDs según la distancia medida.
 * @param distancia Distancia medida en centímetros.
 */
void ActualizarLedsPorDistancia(uint16_t distancia){

    if(distancia < 10){
        LedsOffAll();
    }
    else if(distancia < 20){
        LedOn(LED_1);
        LedOff(LED_2);
        LedOff(LED_3); 
    }
    else if(distancia < 30){
        LedOn(LED_1);
        LedOn(LED_2);
        LedOff(LED_3);
    }
    else{
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    }

}
/**
 * @brief Envía la distancia medida por UART hacia la PC.
 *
 * El formato de envío es: `XXX cm\r\n`, donde XXX son siempre
 * 3 dígitos ASCII (con ceros a la izquierda si es necesario).
 * Si la distancia supera 999 cm se envía "999 cm\r\n".
 *
 */
void EnviarDistanciaUART(uint16_t distancia) {
    char buffer[10];

    if (distancia > 999) distancia = 999;

    buffer[0] = (distancia / 100) % 10 + '0';
    buffer[1] = (distancia / 10)  % 10 + '0';
    buffer[2] = (distancia % 10)       + '0';
    buffer[3] = ' ';
    buffer[4] = 'c';
    buffer[5] = 'm';
    buffer[6] = '\r';
    buffer[7] = '\n';
    buffer[8] = '\0';

    UartSendString(UART_PC, buffer);
}

/**
 * @brief Tarea principal de medición, display, LEDs y comunicación serie.
 *
 * Se activa mediante notificación desde el timer cada PERIODO_MEDICION_US.
 * En cada ciclo realiza dos acciones:
 * -# Lee el puerto serie y procesa comandos 'O' (on/off) y 'H' (hold).
 * -# Si llega una notificación del timer y @ref medir es true:
 *    - Lee la distancia del sensor HC-SR04.
 *    - Envía el valor por UART.
 *    - Actualiza el LCD (si no está en HOLD).
 *    - Actualiza los LEDs.
 *    Si @ref medir es false, apaga LEDs y LCD.
 */
static void TareaMedicion(void *params) {
    uint8_t dato;

    while (1) {
        if (UartReadByte(UART_PC, &dato)) {
            if (dato == 'O' || dato == 'o') {
                medir = !medir;
            } else if (dato == 'H' || dato == 'h') {
                hold = !hold;
            }
        }

        if (ulTaskNotifyTake(pdTRUE, 0)) {
            if (medir) {
                uint16_t distancia = HcSr04ReadDistanceInCentimeters();

                EnviarDistanciaUART(distancia);

                if (!hold) {
                    LcdItsE0803Write(distancia);
                }

                ActualizarLedsPorDistancia(distancia);

            } else {
                LedsOffAll();
                LcdItsE0803Off();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * @brief Notifica a la tarea de medición desde la ISR.
 */
void FuncTimerA(void *param) {
    vTaskNotifyGiveFromISR(control_tarea, pdFALSE);
}

/**
 * @brief ISR para SWITCH_1 (TEC1). Activa o detiene la medición.
 */
void interrupcion_tecla_1(void *args) {
     medir = !medir; 
    }

/**
 * @brief ISR para SWITCH_2 (TEC2). Activa o desactiva el modo HOLD.
 */
void interrupcion_tecla_2(void *args) {
     hold = !hold; 
    }

/*==================[external functions definition]==========================*/
/**
 * @brief Función principal del programa. 
 *
 * Inicializa todos los periféricos y crea la tarea de medición:
 * -# LEDs y switches con sus interrupciones.
 * -# LCD e HC-SR04.
 * -# UART a 115200 baud en modo polling.
 * -# Timer A con período de PERIODO_MEDICION_US.
 * -# Tarea FreeRTOS TareaMedicion.
 */
void app_main(void) {
    LedsInit();
    SwitchesInit();
    SwitchActivInt(SWITCH_1, interrupcion_tecla_1, NULL);
    SwitchActivInt(SWITCH_2, interrupcion_tecla_2, NULL);
    LcdItsE0803Init();
    HcSr04Init(SENSOR_ECHO_PIN, SENSOR_TRIGGER_PIN);

    serial_config_t uart_config = {
        .port      = UART_PC,
        .baud_rate = 115200,
        .func_p    = NULL,
        .param_p   = NULL
    };
    UartInit(&uart_config);

    timer_config_t timer_config = {
        .timer   = TIMER_A,
        .period  = PERIODO_MEDICION_US,
        .func_p  = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_config);

    xTaskCreate(&TareaMedicion, "ControlTarea", 2048, NULL, 5, &control_tarea);

    TimerStart(timer_config.timer);
}
/*==================[end of file]============================================*/
/**
 * @file guia2_ej2.c
 * @brief Implementación de un medidor de distancia por ultrasonido con interrupciones.
 * 
 * @mainpage Medidor de distancia con interrupciones.
 *
 * @section genDesc General Description
 *
 * El firmware mide la distancia con un sensor HC-SR04 y muestra el valor en cm
 * en un display numérico de 3 dígitos. Además utiliza los LEDs de la placa
 * para indicar el rango de distancia.
 *
 * @section controls Controles
 *
 * - TEC1 (SWITCH_1 / GPIO_4): activar / detener medición en el momento de la interrupción.
 * - TEC2 (SWITCH_2 / GPIO_15): mantener el resultado en HOLD en el momento de la interrupción.
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

/*==================[external functions definition]==========================*/

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

/*
 * @brief Tarea de medición y actualización de display/LEDs.
 */
static void TareaMedicion(void *params){
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(medir){
            uint16_t distancia = HcSr04ReadDistanceInCentimeters();
            if(!hold){
                LcdItsE0803Write(distancia);
            }
            ActualizarLedsPorDistancia(distancia);
        }
        else{
            LedsOffAll();
            LcdItsE0803Off();
        }
    }
}

/*
 * @brief Notifica a la tarea de medición que es momento de realizar una nueva medición.
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(control_tarea, pdFALSE);    
}

/**
 * @brief Interrupción para TEC1: activa o detiene la medición.
 */
void interrupcion_tecla_1(void *args){
	medir= !medir;
}
/**
 * @brief Interrupción para TEC2: activa o desactiva el modo HOLD.
 */
void interrupcion_tecla_2(void *args){
	hold= !hold;
}

/**
 * @brief Función principal.
 *
 * Inicializa los periféricos y crea las tareas:
 * - TareaMedicion: medición y actualización de display/LEDs.
 * - Interrupciones para TEC1 y TEC2 para controlar medición y HOLD.
 * - Timer para disparar la tarea de medición cada un segundo.
 */

void app_main(void){
    LedsInit();
    SwitchesInit();
	SwitchActivInt(SWITCH_1, interrupcion_tecla_1, NULL);
	SwitchActivInt(SWITCH_2, interrupcion_tecla_2, NULL);
    LcdItsE0803Init();
    HcSr04Init(SENSOR_ECHO_PIN, SENSOR_TRIGGER_PIN);

    timer_config_t timer_config = {
        .timer = TIMER_A,
        .period = PERIODO_MEDICION_US,
        .func_p = FuncTimerA,
        .param_p = NULL
    };

    TimerInit(&timer_config);

    xTaskCreate(&TareaMedicion, "ControlTarea", 2048, NULL, 5, &control_tarea);

    TimerStart(timer_config.timer);
}
/*==================[end of file]============================================*/
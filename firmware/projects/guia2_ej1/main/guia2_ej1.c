/**
 * @file guia2_ej1.c
 * @brief Medidor de distancia con HC-SR04, display numérico y LEDs.
 * 
 * @mainpage Medidor de distancia por ultrasonido.
 *
 * @section genDesc General Description
 *
 * El firmware mide la distancia con un sensor HC-SR04 y muestra el valor en cm
 * en un display numérico de 3 dígitos. Además utiliza los LEDs de la placa
 * para indicar el rango de distancia.
 *
 * @section controls Controles
 *
 * - TEC1 (SWITCH_1 / GPIO_4): activar / detener medición.
 * - TEC2 (SWITCH_2 / GPIO_15): mantener el resultado en HOLD.
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
 * |   Date    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 15/04/2026 | Implementación de medición, LEDs y HOLD       |
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
#include "hc_sr04.h"
#include "lcditse0803.h"

/*==================[macros and definitions]=================================*/
#define SENSOR_ECHO_PIN        GPIO_3
#define SENSOR_TRIGGER_PIN     GPIO_2
#define PERIODO_TECLAS_MS      10
#define PERIODO_MEDICION_MS    1000

/*==================[internal data definition]===============================*/
/** @brief Indica si la medición está activa o detenida (controlada con TEC1). */
bool medir = true;

/** @brief Indica si el sistema está en modo HOLD (controlado con TEC2). */
bool hold = false;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Actualiza la iluminación de los LEDs de la placa según la distancia recibida.
 * * @param distancia Valor numérico de la distancia medida en centímetros.
 */
void ActualizarLedsPorDistancia(uint16_t distancia);

/**
 * @brief Tarea encargada de leer el estado de las teclas y conmutar las variables de control.
 * @param params Puntero genérico de FreeRTOS (no utilizado).
 */
static void TareaTeclas(void *params);

/**
 * @brief Tarea que gestiona la medición del sensor y la actualización de la interfaz.
 * Si el sistema está en modo medición, lee la distancia del sensor HC-SR04 y actualiza el display y los LEDs.
 * * @param params Puntero genérico de FreeRTOS (no utilizado).
 */
static void TareaMedicion(void *params);

/*==================[external functions definition]==========================*/

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

static void TareaTeclas(void *params){
    while(1){
        uint8_t teclas = SwitchesRead();
        if(teclas & SWITCH_1){
            medir = !medir;
        }

        if(teclas & SWITCH_2){
            hold = !hold;
        }

        vTaskDelay(pdMS_TO_TICKS(PERIODO_TECLAS_MS));
    }
}

static void TareaMedicion(void *params){
    while(1){
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
        vTaskDelay(pdMS_TO_TICKS(PERIODO_MEDICION_MS));
    }
}

/**
 * @brief Función principal.
 *
 * Inicializa los periféricos y crea las tareas:
 * - TareaTeclas: lectura de teclas.
 * - TareaMedicion: medición y actualización de display/LEDs.
 */

void app_main(void){
    LedsInit();
    SwitchesInit();
    LcdItsE0803Init();
    HcSr04Init(SENSOR_ECHO_PIN, SENSOR_TRIGGER_PIN);

    xTaskCreate(TareaTeclas, "TareaTeclas", 2048, NULL, 5, NULL);
    xTaskCreate(TareaMedicion, "TareaMedicion", 2048, NULL, 5, NULL);
}
/*==================[end of file]============================================*/
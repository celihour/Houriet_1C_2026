/*! @mainpage LED Control
 *
 * \section genDesc General Description
 *
 * This example demonstrates the control of LEDs with different modes (ON, OFF, TOGGLE).
 * 
 * @section changelog Changelog
 *
 * |   Date     | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation                              |
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
#include "led.h"
/*==================[macros and definitions]=================================*/

struct leds{
	uint8_t mode;
	uint8_t n_led; 
	uint8_t n_ciclos;
	uint8_t periodo;
} my_leds;

enum{ON, OFF, TOGGLE};
#define CONFIG_BLINK_PERIOD 1000
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void control_leds(struct leds *mis_leds){
	switch(mis_leds -> mode){
		case ON:
			LedOn(mis_leds -> n_led);
			break;
		case OFF:
			LedOff(mis_leds -> n_led);
			break;
		case TOGGLE:
			uint8_t i=0;
			while(i < mis_leds->n_ciclos){
				LedToggle(mis_leds->n_led);
				vTaskDelay(mis_leds->periodo / portTICK_PERIOD_MS);
				i++;
			}
	}
}
/*==================[external functions definition]==========================*/

void app_main(void){
	LedsInit();
	my_leds.mode = ON;
	my_leds.n_led = LED_1;
	my_leds.n_ciclos = 5;
	my_leds.periodo = CONFIG_BLINK_PERIOD;
	control_leds(&my_leds);

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	my_leds.mode = TOGGLE;
	my_leds.n_led = LED_2;
	my_leds.n_ciclos = 10;
	my_leds.periodo = CONFIG_BLINK_PERIOD;
	control_leds(&my_leds);

	vTaskDelay(2000 / portTICK_PERIOD_MS);

	my_leds.mode = OFF;
	my_leds.n_led = LED_1;
	my_leds.n_ciclos = 5;
	my_leds.periodo = CONFIG_BLINK_PERIOD;
	control_leds(&my_leds);
	
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	my_leds.mode = ON;
	my_leds.n_led = LED_3;
	my_leds.n_ciclos = 5;
	my_leds.periodo = CONFIG_BLINK_PERIOD;
	control_leds(&my_leds);

}
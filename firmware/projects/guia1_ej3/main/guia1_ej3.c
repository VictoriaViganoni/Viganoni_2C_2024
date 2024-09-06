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
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 100
#define ON 1
#define OFF 0
#define TOGGLE 2
/*==================[internal data definition]===============================*/
struct leds
{
	uint8_t mode;
	uint8_t n_led;
	uint8_t n_ciclos;
	uint16_t periodo;
} my_leds;


/*==================[internal functions declaration]=========================*/
void funcion_con_puntero(struct leds *puntero_led){
	if (puntero_led->mode==ON){
		if(puntero_led->n_led==1){
			LedOn(LED_1);
		}
		else if(puntero_led->n_led==2){
			LedOn(LED_2);
		}
		else if(puntero_led->n_led==3){
			LedOn(LED_3);
		}
	}
	else if(puntero_led->mode==OFF){
		if(puntero_led->n_led==1){
			LedOff(LED_1);
		}
		else if(puntero_led->n_led==2){
			LedOff(LED_2);
		}
		else if(puntero_led->n_led==3){
			LedOff(LED_3);
		}
	}
	else if(puntero_led->mode==TOGGLE){

		for(int i=0;i<puntero_led->n_ciclos;i++){
			if(puntero_led->n_led==1){
					LedToggle(LED_1);
				}
				else if(puntero_led->n_led==2){
					LedToggle(LED_2);
				}
				else if(puntero_led->n_led==3){
					LedToggle(LED_3);
				}
			for (int j=0;j<(puntero_led->periodo/CONFIG_BLINK_PERIOD);j++){

				vTaskDelay(CONFIG_BLINK_PERIOD/portTICK_PERIOD_MS);

			}
		}	
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
LedsInit();

my_leds.mode=TOGGLE;
my_leds.n_ciclos=10;
my_leds.n_led=1;
my_leds.periodo=500;

funcion_con_puntero(&my_leds);
	
}
/*==================[end of file]============================================*/
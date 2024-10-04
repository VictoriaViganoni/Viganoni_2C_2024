/*! @mainpage Proyecto 2: ejercicio 1
 *
 * @section genDesc General Description
 *
 *Este programa implementa la medición y visualización de distancia utilizando un sensor ultrasónico HC-SR04 y 
 *una pantalla LCD conectada a una placa EDU-ESP. El sistema enciende diferentes combinaciones de LEDs según la 
 *distancia medida: si es menor a 10 cm, apaga todos los LEDs; entre 10 y 20 cm, enciende el LED_1; entre 20 y 30 cm, 
 *enciende los LEDs 1 y 2; y si es mayor a 30 cm, enciende los LEDs 1, 2 y 3. La distancia también se muestra en la 
 *pantalla LCD. El programa permite iniciar o detener la medición con el botón TEC1 y mantener el último valor medido 
 *en el display con TEC2, actualizando la medición cada segundo.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	Trigger	 	| 	GPIO_2		|
 * | 	+5V 	 	|    +5V 		|
 * | 	 GND	 	| 	 GND		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 06/09/2024 | Document creation		                         |
 * 
 * @section Consigna
 * Diseñar el firmware modelando con un diagrama de flujo de manera que cumpla con las siguientes funcionalidades:
 *
 * Mostrar distancia medida utilizando los leds de la siguiente manera:
 *
 * Si la distancia es menor a 10 cm, apagar todos los LEDs.
 * Si la distancia está entre 10 y 20 cm, encender el LED_1.
 * Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 *
 * Mostrar el valor de distancia en cm utilizando el display LCD.
 * Usar TEC1 para activar y detener la medición.
 * Usar TEC2 para mantener el resultado (“HOLD”).
 * Refresco de medición: 1 s
 *
 * Se deberá conectar a la EDU-ESP un sensor de ultrasonido HC-SR04 y una pantalla LCD y utilizando los drivers provistos por la cátedra 
 * implementar la aplicación correspondiente. Se debe subir al repositorio el código. Se debe incluir en la documentación, realizada con doxygen, el diagrama de flujo. 
 *
 * @author maria victoria viganoni (maria.viganoni@ingeniera.uner.edu.ar)
 */
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "lcditse0803.h"
#include "switch.h"
#include "led.h"
#include "hc_sr04.h"
/*==================[macros and definitions]=================================*/
/** @def REFRESH_TECLAS
 * @brief Define el tiempo en milisegundos para el retardo en la tarea de gestión de teclas.
 */
#define REFRESH_TECLAS 50

/** @def REFRESH_MEDICION
 * @brief Define el tiempo en milisegundos para el retardo en la tarea de medición con el sensor ultrasónico.
 */
#define REFRESH_MEDICION 1000

/** @def REFRESH_DISPLAY
 * @brief Define el tiempo en milisegundos para el retardo en la actualización de la pantalla LCD.
 */
#define REFRESH_DISPLAY 100

/*==================[internal data definition]===============================*/
/** 
 * @def distancia
 * @brief Variable global de tipo entero sin signo que almacena la distancia medida por el sensor ultrasónico en centímetros.
 */
uint16_t distancia = 0;

/** 
 * @def hold
 * @brief Variable global booleana que indica si se debe mantener el último valor de distancia medido en el display.
 */
bool hold = false;

/** 
 * @def on
 * @brief Variable global booleana que indica si el sistema de medición está activo o inactivo.
 */
bool on = true;

/*==================[internal functions declaration]=========================*/
/**
 * @fn void sensarTask()
 * @brief Esta tarea realiza las mediciones de distancia usando el sensor ultrasónico.
 * Actualiza la variable "distancia" y realiza un retraso conforme al intervalo de medición especificado.
 * @return
 */
void sensarTask()
{ 
    while (true)
    {
        if (on)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
        vTaskDelay(REFRESH_MEDICION / portTICK_PERIOD_MS);
    }
}

/**
 * @fn void mostrarTask()
 * @brief Esta tarea muestra los datos medidos en la pantalla LCD y controla el encendido de los LEDs.
 * Dependiendo de la distancia medida, enciende o apaga los LEDs correspondientes. Además, si no está activado el modo "hold", 
 * actualiza el valor mostrado en el display.
 * @return
 */
void mostrarTask()
{
    while (true)
    {
        if (on)
        {
            if (distancia < 10)
            {
                LedsOffAll();
            }
            else if ((distancia > 10) & (distancia < 20))
            {
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }
            else if ((distancia > 20) & (distancia < 30))
            {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);
            }
            else if (distancia > 30)
            {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
            }

            if (!hold)
            {
                LcdItsE0803Write(distancia);
            }
        }
        else
        {
            LcdItsE0803Off();
            LedsOffAll();
        }
        vTaskDelay(REFRESH_DISPLAY / portTICK_PERIOD_MS);
    }
}
/**
 * @fn void teclasTask()
 * @brief Esta tarea gestiona el control de la aplicación utilizando las teclas.
 * Permite activar o desactivar la medición con TEC1 y mantener el valor de la medición con TEC2.
 * @return
 */
void teclasTask()
{
    uint8_t teclas;
    while (true)
    {
        teclas = SwitchesRead();
        switch (teclas)
        {
        case SWITCH_1:
            on = !on;
            break;
        case SWITCH_2:
            hold = !hold;
            break;
        }
        vTaskDelay(REFRESH_TECLAS / portTICK_PERIOD_MS);
    }
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
    LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();
    SwitchesInit();

    xTaskCreate(&sensarTask, "medir", 512, NULL, 5, NULL);
    xTaskCreate(&teclasTask, "mostrar", 512, NULL, 5, NULL);
    xTaskCreate(&mostrarTask, "teclas", 512, NULL, 5, NULL);
}
/*==================[end of file]============================================*/
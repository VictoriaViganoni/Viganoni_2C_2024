/*! @mainpage Proyecto 2: ejercicio 2
 *
 * @section genDesc General Description
 *
 *Este código controla un sistema de medición de distancia con un sensor ultrasónico y muestra 
 *los resultados en una pantalla LCD, además de encender LEDs según la distancia medida. Utiliza 
 *un ESP32 con interrupciones para gestionar el control de teclas y temporizadores. La distancia se 
 *mide mediante el sensor ultrasónico conectado a los pines GPIO 2 y 3, y los valores se actualizan 
 *en intervalos definidos por REFRESH_MEDICION y REFRESH_DISPLAY. Las tareas encargadas de sensar y 
 *mostrar los datos se manejan de forma asíncrona mediante notificaciones y variables globales como 
 *distancia, hold, y on controlan el estado del sistema.
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
 * | 13/09/2024 | Document creation		                         |
 * 
 * @section Consigna
 * Cree un nuevo proyecto en el que modifique la actividad del punto 1 de manera de utilizar interrupciones 
 * para el control de las teclas y el control de tiempos (Timers). 
 *
 *
 * @author maria victoria viganoni (maria.viganoni@ingeniera.uner.edu.ar)
 *
 */
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "gpio_mcu.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/

/** @def REFRESH_MEDICION
 * @brief Define el tiempo en milisegundos para el retardo en la tarea de medición con el sensor ultrasónico.
 */
#define REFRESH_MEDICION 1000000

/** @def REFRESH_DISPLAY
 * @brief Define el tiempo en milisegundos para el retardo en la actualización de la pantalla LCD.
 */
#define REFRESH_DISPLAY 1000000


/*==================[internal data definition]===============================*/
/**
 * @def Sensar_task_handle 
 * @brief Manejador de la tarea encargada de sensar.
 * Este objeto es utilizado para gestionar la tarea que realiza las funciones
 * de sensado dentro del sistema. Permite la manipulación y notificación de la 
 * tarea desde otras partes del código, incluyendo la reanudación o suspensión
 * de su ejecución.
 */
TaskHandle_t Sensar_task_handle = NULL;

/**
 * @def Mostrar_task_handle
 * @brief Manejador de la tarea encargada de mostrar información.
 * Este objeto permite gestionar la tarea responsable de mostrar información en
 * el sistema. Se utiliza para notificar, suspender o reanudar dicha tarea desde
 * otros contextos de ejecución, como interrupciones o tareas concurrentes.
 */
TaskHandle_t Mostrar_task_handle = NULL;

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
 * @fn FuncTimerSensar(void *param)
 * @param param 
 * @brief Envía una notificación a la tarea encargada de sensar.
 * Esta función se utiliza para notificar de manera asíncrona a la tarea de sensado, 
 * desencadenando su ejecución. Se emplea en un contexto de interrupción, utilizando 
 * la función vTaskNotifyGiveFromISR para evitar bloqueos en la ejecución del sistema.
 * @return
 */
void FuncTimerSensar(void *param)
{
    vTaskNotifyGiveFromISR(Sensar_task_handle, pdFALSE); 
}

/**
 * @fn void FuncTimerMostrar(void *param)
 * @param param 
 * @brief Envía una notificación a la tarea encargada de mostrar.
 * Esta función notifica de manera asíncrona a la tarea responsable de mostrar información, 
 * desencadenando su ejecución. Se ejecuta en un contexto de interrupción, usando la función 
 * vTaskNotifyGiveFromISR, asegurando así una operación eficiente sin bloqueos del sistema.
 * @return
 */
void FuncTimerMostrar(void *param)
{
    vTaskNotifyGiveFromISR(Mostrar_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al mostrar */
}

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
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /*la tarea espera en este punto hasta recibir la notificacion*/
        if (on)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
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
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
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
    }
}

/**
 * @fn TeclaOn()
 * @brief Alterna el estado de la bandera booleana `on`.
 * Esta función invierte el valor de la variable booleana `on`. Si el valor de 
 * `on` es verdadero, lo cambia a falso, y viceversa. Se utiliza para controlar 
 * el encendido o activación de un proceso o sistema.
 * @return
 */
void TeclaOn()
{
    on = !on;
}

/**
 * @fn TeclaHold()
 * @brief Alterna el estado de la bandera booleana `hold` si la bandera `on` está activa.
 * Esta función cambia el estado de la variable booleana `hold` solo si la 
 * bandera `on` está activada (es verdadera). Se utiliza para mantener o pausar 
 * una acción mientras el sistema está encendido.
 * @return
 */
void TeclaHold()
{
    if (on)
    {
        hold = !hold;
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();
    SwitchesInit();

    timer_config_t timer_medicion = {
        .timer = TIMER_A,
        .period = REFRESH_MEDICION,
        .func_p = FuncTimerSensar,
        .param_p = NULL};
    TimerInit(&timer_medicion);

    timer_config_t timer_mostrar = {
        .timer = TIMER_B,
        .period = REFRESH_DISPLAY,
        .func_p = FuncTimerMostrar,
        .param_p = NULL};
    TimerInit(&timer_mostrar);

    SwitchActivInt(SWITCH_1, &TeclaOn, NULL);
    SwitchActivInt(SWITCH_2, &TeclaHold, NULL);
    xTaskCreate(&sensarTask, "sensar", 512, NULL, 5, &Sensar_task_handle);
    xTaskCreate(&mostrarTask, "mostrar", 512, NULL, 5, &Mostrar_task_handle);
    TimerStart(timer_medicion.timer);
    TimerStart(timer_mostrar.timer);
}
/*==================[end of file]============================================*/
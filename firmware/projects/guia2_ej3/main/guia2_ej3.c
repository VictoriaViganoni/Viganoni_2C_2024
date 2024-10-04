/*! @mainpage Proyecto 2: ejercicio 3
 *
 * @section genDesc General Description
 *
 *Este código implementa un sistema que mide distancias utilizando un sensor ultrasónico, 
 *visualiza los resultados en una pantalla LCD y los envía a un terminal en la PC a través 
 *de un puerto serie. El sistema permite controlar la funcionalidad de la EDU-ESP mediante 
 *teclas físicas y comandos enviados por puerto serie ('O' para encender/apagar el sistema 
 *y 'H' para mantener o liberar la distancia mostrada). Se utilizan tareas para el sensado 
 *y la visualización, gestionadas mediante timers y notificaciones, además de un puerto 
 *UART para enviar los datos en un formato predefinido al terminal.
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
 * | 20/09/2024 | Document creation		                         |
 * 
 * @section Consigna 
 *Cree un nuevo proyecto en el que modifique la actividad del punto 2 agregando ahora el puerto serie. 
 *Envíe los datos de las mediciones para poder observarlos en un terminal en la PC, siguiendo el siguiente formato:
 *3 dígitos ascii + 1 carácter espacio + dos caracteres para la unidad (cm) + cambio de línea “ \r\n”
 *Además debe ser posible controlar la EDU-ESP de la siguiente manera:
 *Con las teclas “O” y “H”, replicar la funcionalidad de las teclas 1 y 2 de la EDU-ESP
 *
 * @author maria victoria viganoni (maria.viganoni@ingeniera.uner.edu.ar)
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
#include "uart_mcu.h"
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
 * @brief Controlador de la tarea encargada de sensar.
 * Este objeto es utilizado para gestionar la tarea que realiza las funciones
 * de sensado dentro del sistema. Permite la manipulación y notificación de la 
 * tarea desde otras partes del código, incluyendo la reanudación o suspensión
 * de su ejecución.
 */
TaskHandle_t Sensar_task_handle = NULL;

/**
 * @def Mostrar_task_handle
 * @brief Controlador de la tarea encargada de mostrar información.
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
bool on;

/*==================[internal functions declaration]=========================*/
/**
 * @fn escribirDistanciaEnPc()
 * @brief Permite mostrar por monitor serial la distancia sensada en tiempo real
 * @return
 */
void escribirDistanciaEnPc()
{
    UartSendString(UART_PC, "distancia ");
    UartSendString(UART_PC, (char *)UartItoa(distancia, 10));
    UartSendString(UART_PC, " cm\r\n");
}

/**
 * @fn sensarTask()
 * @brief sensarTask es una tarea destinada a la realización de medir la distancia al sensor de ultrasonido
 * @return 
 */
void sensarTask()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (on)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
    }
}

/**
 * @fn mostrarTask()
 * @brief mostrarTask es un tarea destinada a la realización de encender LEDs en función de la distancia sensada además de mostrar 
 * por LCD la distancia medida en el sensor.
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
            escribirDistanciaEnPc();
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
 * @brief Cambia el estado de la bandera booleana On
 * @return
 */
void TeclaOn()
{
    on = !on;
}

/**
 * @fn TeclaHold()
 * @brief Cambia el estado de la bandera booleana Hold
 * @return
 */
void TeclaHold()
{
    hold = !hold;
}

/**
 * @fn TeclaOnHold()
 * @brief Cambia el estado de las banderas booleanas On y Hold en función de la entrada por conexión de puerto
 * serie
 * @return
*/
void TeclasOnHold()
{
    uint8_t tecla;
    UartReadByte(UART_PC, &tecla);
    switch (tecla)
    {
    case 'O':
        on = !on;
        break;

    case 'H':
        hold = !hold;
        break;
    }
}

/**
 * @fn FuncTimerSensar()
 * @brief Envía una notificación a la tarea sensar
 * @return
*/
void FuncTimerSensar()
{
    vTaskNotifyGiveFromISR(Sensar_task_handle, pdFALSE); 
}

/**
 * @fn FuncTimerMostrar()
 * @brief Envía una notificación a la tarea mostrar
 * @return
*/
void FuncTimerMostrar()
{
    vTaskNotifyGiveFromISR(Mostrar_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al mostrar */
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();
    SwitchesInit();

    serial_config_t ControlarUart =
        {
            .port = UART_PC,
            .baud_rate = 115200,
            .func_p = TeclasOnHold,
            .param_p = NULL,
        };
    UartInit(&ControlarUart);
    
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

  
    SwitchActivInt(SWITCH_1, TeclaOn, NULL);
    // SwitchActivInt('o', TeclaO, NULL);

    SwitchActivInt(SWITCH_2, TeclaHold, NULL);
    // SwitchActivInt('H', TeclaH, NULL);

  
    xTaskCreate(&sensarTask, "sensar", 512, NULL, 5, &Sensar_task_handle);
    xTaskCreate(&mostrarTask, "teclas", 512, NULL, 5, &Mostrar_task_handle);
    TimerStart(timer_medicion.timer);
    TimerStart(timer_mostrar.timer);
}
/*==================[end of file]============================================*/
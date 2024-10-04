/*! @mainpage Proyecto 2: ejercicio 4
 *
 * @section genDesc General Description
 *
 * El programa implementa la digitalización de una señal analógica y su transmisión por UART 
 * a un graficador en la PC, basado en dos tareas de conversión A/D y D/A. La señal se lee 
 * desde el canal CH1 del conversor A/D del ESP32 y se transmite de forma periódica a la PC 
 * a través del puerto UART. La conversión A/D ocurre cada 2000 ms, mientras que la conversión 
 * D/A se realiza con datos simulados de una señal ECG almacenada en un buffer. Ambas tareas 
 * son manejadas por temporizadores que envían notificaciones para continuar los ciclos de conversión.
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
 * | 27/09/2024 | Document creation		                         |
 * 
 * @section Consigna
 * Diseñar e implementar una aplicación, basada en el driver analog_io_mcu.h y el driver de transmisión serie uart_mcu.h, 
 * que digitalice una señal analógica y la transmita a un graficador de puerto serie de la PC. Se debe tomar la entrada CH1 
 * del conversor AD y la transmisión se debe realizar por la UART conectada al puerto serie de la PC, en un formato compatible 
 * con un graficador por puerto serie. 
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
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/

/**
 * @def TIEMPOCONVERSION_AD
 * @brief Define el tiempo (en milisegundos) que toma realizar una conversión A/D.
 */
#define TIEMPOCONVERSION_AD 2000

/**
 * @def TIEMPOCONVERSION_DA
 * @brief Define el tiempo (en milisegundos) que toma realizar una conversión D/A.
 */
#define TIEMPOCONVERSION_DA 4000

/**
 * @def BUFFER_SIZE 
 * @brief Tamaño del vector que almacena los datos de una señal ECG.
 */
#define BUFFER_SIZE 231

/*==================[internal data definition]===============================*/

/** 
 * @def ConversorAD_task_handle 
 * @brief Controlador de la tarea encargada de las conversiones A/D con interrupciones.
 */
TaskHandle_t ConversorAD_task_handle = NULL;

/** 
 * @def ConversorDA_task_handle 
 * @brief Controlador de la tarea encargada de las conversiones D/A con interrupciones.
 */
TaskHandle_t ConversorDA_task_handle = NULL;

/** 
 * @def ValorAnalogico 
 * @brief Variable de tipo entero sin signo que almacena el valor analógico leído.
 */
uint16_t valorAnalogico = 0;

/** 
 * @defecg
 * @brief Vector que contiene los datos de una señal ECG, con un tamaño definido por BUFFER_SIZE.
 */
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77,77,76,76,
};

/*==================[internal functions declaration]=========================*/
/** 
 * @fn EscribirValor() 
 * @brief Permite enviar por el monitor serie los valores analógicos leídos.
 * @return
 */
void EscribirValor()
{
    UartSendString(UART_PC, (char *)UartItoa(valorAnalogico, 10));
    UartSendString(UART_PC, "\r");
}

/** 
 * @fn AD_conversor_task()
 * @brief Tarea encargada de la conversión A/D.
 * @return
 */
void AD_conversor_task()
{

    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH1, &valorAnalogico);
        EscribirValor();
    }
}

/** 
* @fn DA_conversor_task()
* @brief Tarea encargada de la conversión D/A.
* @return
*/
void DA_conversor_task()
{
    uint8_t i = 0;
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogOutputWrite(ecg[i]);
        i++;
        if (i == BUFFER_SIZE-1)
        {
            i = 0;
        }
    }
}

/** 
 * @fn FuncTimerConverDA()
 * @brief Función que envía una notificación para continuar la tarea de conversión D/A.
 * @return
 */
void FuncTimerConverDA()
{
    vTaskNotifyGiveFromISR(ConversorDA_task_handle, pdFALSE);
}

/** 
 * @fn FuncTimerConverAD()
 * @brief Función que envía una notificación para continuar la tarea de conversión D/A.
 * @return
 */
void FuncTimerConverAD()
{
    vTaskNotifyGiveFromISR(ConversorAD_task_handle, pdFALSE);
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    serial_config_t ControlarUart =
        {
            .port = UART_PC,
            .baud_rate = 115200,
            .func_p = NULL,
            .param_p = NULL,
        };
    UartInit(&ControlarUart);
  
    timer_config_t timer_converDA = {
        .timer = TIMER_A,
        .period = TIEMPOCONVERSION_DA,
        .func_p = FuncTimerConverDA,
        .param_p = NULL};
    TimerInit(&timer_converDA);

    timer_config_t timer_converAD = {
        .timer = TIMER_B,
        .period = TIEMPOCONVERSION_AD,
        .func_p = FuncTimerConverAD,
        .param_p = NULL};
    TimerInit(&timer_converAD);

    analog_input_config_t Analog_input = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0};
    AnalogInputInit(&Analog_input);
    AnalogOutputInit();

    xTaskCreate(&DA_conversor_task, "conversor DA", 512, NULL, 5, &ConversorDA_task_handle);
    xTaskCreate(&AD_conversor_task, "conversor AD", 4096, NULL, 5, &ConversorAD_task_handle);
    TimerStart(timer_converAD.timer);
    TimerStart(timer_converDA.timer);
}
/*==================[end of file]============================================*/
/*! @mainpage Template
 *
 * @section genDesc General Description
 * Alerta para Ciclistas
 * Este sistema mide la proximidad de vehículos mediante un sensor de ultrasonido y detecta caídas usando un acelerómetro.
 * Los LEDs y el buzzer indicanel estado del sistema y se envían notificaciones vía Bluetooth en caso de alerta.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral     |   ESP32          	  |
 * |:-----------------:|:---------------------|
 * | Ultrasonido       | GPIO_4 y GPIO_5      |
 * | Acelerómetro      | ADC canales 0, 3 y 6 |
 * | Buzzer            | GPIO_18              |
 * | Bluetooth (UART)  | UART_2               |
 * | 	+5V 	 	   |    +5V 	          |
 * | 	 GND	 	   | 	 GND		      |
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 4/11/2024 | Document creation		                         |
 * 
 * @section Consigna
 *Se pretende diseñar un dispositivo basado en la ESP-EDU que permita detectar eventos peligrosos para ciclistas.
 *El sistema está compuesto por un acelerómetro analógico montado sobre el casco y un HC-SR04 ubicado en la parte trasera de la bicicleta.
 *Por un lado se debe detectar la presencia de vehículos detrás de la bicicleta. Para ello se utiliza el sensor de ultrasonido. 
 *Se indicará mediante los leds de la placa la distancia de los vehículos a la bicicleta, encendiéndose de la siguiente manera:
 *● Led verde para distancias mayores a 5 metros
 *● Led verde y amarillo para distancias entre 5 y 3 metros
 *● Led verde, amariilo y rojo para distancias menores a 3 metros.
 *La medición con el HC-SR04 deberá realizarse dos veces por segundo. Además de los mensajes se deberá activar una alarma sonora mediante un buzzer activo. Este
 *dispositivo suena cuando uno de los GPIOs de la placa se pone en alto y se apaga cuando se pone en bajo. 
 *La alarma sonará con una frecuencia de 1 segundo en el caso de precaución y cada 0.5 segundos en el caso de peligro.
 *Además se deberá enviar una notificación a una Aplicación corriendo en un Smartphone. 
 *Esta notificación se envía utilizando un módulo bluetooth conectado al segundo puerto serie de la placa ESP-EDU. Se enviarán los siguientes mensajes:
 *● “Precaución, vehículo cerca”, para distancias entre 3 a 5 metros
 *● “Peligro, vehículo cerca”, para distancias menores a 3 metros
 *El acelerómetro ubicado en el casco tiene la finalidad de detectar golpes o caídas. Se trata de un acelerómetro analógico triaxial (3 canales), muestreado a 100Hz, con una
 *salida de 1.65 V para 0 G y una sensibilidad de 0.3V/G.
 *Si la sumatoria (escalar) de la aceleración en los tres ejes supera los 4G se deberá enviar el siguiente mensaje a la aplicación:
 *● “Caída detectada”
 * 
 * 
 *  @author maria victoria viganoni (maria.viganoni@ingeniera.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "analog_io_mcu.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "bluetooth_mcu.h"
#include "led.h"
/*==================[macros and definitions]=================================*/

/**
*@def LED_VERDE
*@brief Led verde para indicar seguridad
*/
#define LED_VERDE LED_1

/**
*@def LED_AMARILLO
*@brief Led amarillo para indicar advertencia
*/
#define LED_AMARILLO LED_2

/**
*@def LED_ROJO
*@brief Led rojo para indicar peligro
*/
#define LED_ROJO LED_3

/**
*@def DISTANCIA_ALERTA
*@brief Umbral de distancia en metros para alerta 
*/
#define DISTANCIA_ALERTA 5.0

/**
*@def  UMBRAL_CAIDA
*@brief Umbral de aceleración para detectar caída (en G)
*/
#define UMBRAL_CAIDA 4.0

/*==================[internal data definition]===============================*/
/** 
 * @struct config_perifericos_t
 * @brief Configuración para periféricos del sistema
 * @return  led_verde       
 * @return led_amarillo    
 * @return  led_rojo        
 * @return  buzzer_pin    
*/
typedef struct {
    int led_verde;       
    int led_amarillo;    
    int led_rojo;       
    int buzzer_pin;     
} config_perifericos_t;
/** 
 * @def alerta_peligro
 * @brief Variable global booleana que indica el estado de alerta de proximidad
 */
bool alerta_peligro = false;  

/** 
 * @def aceleracion_x, aceleracion_y, aceleracion_z
 * @brief Variables donde se vana  almacenar los datos de las aceleraciones en los ejes x, y, z
 * */
float aceleracion_x, aceleracion_y, aceleracion_z; 

/*==================[internal functions declaration]=========================*/

/** 
*@fn inicializar_leds
*@brief Inicializa los LEDs de alerta en función del estado de proximidad.
*@return 
*@param config Estructura de configuración de LEDs
*
*/
void inicializar_leds(config_perifericos_t* config) {
    LedInit(config->led_verde);
    LedInit(config->led_amarillo);
    LedInit(config->led_rojo);
}

/** 
*@fn inicializar_bluetooth()
*@brief Inicializa el módulo de Bluetooth utilizando las configuraciones predeterminadas.
*@return
*@param [in]
*
*/
void inicializar_bluetooth() {
    BluetoothInit(); 
}

/** 
*@fn inicializar_buzzer
*@brief Configura el buzzer para emitir sonido en caso de alerta.
*@return
*@param config Estructura de configuración que contiene el pin del buzzer.
*
*/
void inicializar_buzzer(config_perifericos_t* config) {
    GPIOConfig(config->buzzer_pin, GPIO_OUTPUT);
}

/** 
*@fn medir_distancia_ultrasonido()
*@brief Mide la distancia usando el sensor de ultrasonido y actualiza el estado de alerta de proximidad.
*@return
*@param [in]
*
*/
void medir_distancia_ultrasonido() {
    float distancia = UltrasonidoMeasureDistance();  // Función del repositorio
    
    if (distancia > DISTANCIA_ALERTA) {
        LedOn(LED_VERDE);
        LedOff(LED_AMARILLO);
        LedOff(LED_ROJO);
        alerta_peligro = false;
    } else if (distancia > 3.0) {
        LedOn(LED_VERDE);
        LedOn(LED_AMARILLO);
        LedOff(LED_ROJO);
        alerta_peligro = false;
    } else {
        LedOn(LED_VERDE);
        LedOn(LED_AMARILLO);
        LedOn(LED_ROJO);
        alerta_peligro = true;
    }
}

/** 
*@fn controlar_alarma
*@brief Tarea para controlar el buzzer y enviar notificaciones Bluetooth en caso de alerta.
*@return
*@param pvParameters Parámetro de la tarea.
*
*/
void controlar_alarma(void* pvParameters) {
    while (1) {
        if (alerta_peligro) {
            GPIOWrite(BUZZER_PIN, HIGH);
            enviar_notificacion_bluetooth("Peligro, vehículo cerca");
            alerta_peligro = false;
        } else {
            GPIOWrite(BUZZER_PIN, LOW);
        }
        vTaskDelay(pdMS_TO_TICKS(500));  
    }
}

/** 
*@fn enviar_notificacion_bluetooth
*@brief  Envía notificación a través de Bluetooth.
*@return
*@param mensaje Mensaje a enviar.
*
*/
void enviar_notificacion_bluetooth(const char* mensaje) {
    BluetoothSendData(mensaje);  o
}

/** 
*@fn detectar_caida
*@brief Tarea para detectar caídas usando el acelerómetro.
*@return 
*@param pvParameters Parámetro de la tarea.
*
*/
void detectar_caida(void* pvParameters) {
    while (1) {
        AnalogInputReadSingle(CH_X, &aceleracion_x);
        AnalogInputReadSingle(CH_Y, &aceleracion_y);
        AnalogInputReadSingle(CH_Z, &aceleracion_z);
        
        float magnitud_aceleracion = sqrt(aceleracion_x * aceleracion_x + aceleracion_y * aceleracion_y + aceleracion_z * aceleracion_z);
        
        if (magnitud_aceleracion > UMBRAL_CAIDA) {
            enviar_notificacion_bluetooth("Caída detectada");
        }
        vTaskDelay(pdMS_TO_TICKS(10));  // a 100 Hz
    }
}

/** 
*@fn configurar_tareas()
*@brief Configura e inicia las tareas del sistema.
*@return
*@param [in]
*
*/
void configurar_tareas() {
    xTaskCreate(&controlar_alarma, "ControlarAlarma", 2048, NULL, 5, NULL);
    xTaskCreate(&detectar_caida, "DetectarCaida", 2048, NULL, 6, NULL);
}

/*==================[external functions definition]==========================*/
void app_main(void) {
    config_perifericos_t config = {
        .led_verde = LED_VERDE,
        .led_amarillo = LED_AMARILLO,
        .led_rojo = LED_ROJO,
        .buzzer_pin = BUZZER_PIN
    };
    
    inicializar_leds(&config);
    inicializar_bluetooth();
    inicializar_buzzer(&config);
    configurar_tareas();
    
    while (1) {
        medir_distancia_ultrasonido();
        vTaskDelay(pdMS_TO_TICKS(500));  //  cada 0.5 segundos
    }
}

/*==================[end of file]============================================*/
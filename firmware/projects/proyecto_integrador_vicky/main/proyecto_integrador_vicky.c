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
 * @author maria victoria viganoni (maria.viganoni@ingeniera.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
/*==================[macros and definitions]=================================*/
// Definir pines de conexión de los motores y servo
#define X_STEP_PIN GPIO_NUM_16
#define X_DIR_PIN GPIO_NUM_17
#define Y_STEP_PIN GPIO_NUM_15
#define Y_DIR_PIN GPIO_NUM_13
#define SERVO_PIN GPIO_NUM_23
// Diccionario de Braille en matrices 3x2
const int diccionario_braille[26][6] = {
    {0, 1, 0, 0, 0, 0}, // A
    {0, 1, 0, 1, 0, 0}, // B
    {1, 1, 0, 0, 0, 0}, // C
    {1, 1, 1, 0, 0, 0}, // D
    {0, 1, 1, 0, 0, 0}, // E
    {1, 1, 0, 1, 0, 0}, // F
    {1, 1, 1, 1, 0, 0}, // G
    {0, 1, 1, 1, 0, 0}, // H
    {1, 0, 1, 0, 0, 0}, // I
    {1, 0, 1, 1, 0, 0}, // J
    {0, 1, 0, 0, 0, 1}, // K
    {0, 1, 0, 1, 0, 1}, // L
    {1, 1, 0, 0, 0, 1}, // M
    {1, 1, 1, 0, 0, 1}, // N
    {0, 1, 1, 0, 0, 1}, // O
    {1, 1, 0, 1, 0, 1}, // P
    {1, 1, 1, 1, 0, 1}, // Q
    {0, 1, 1, 1, 0, 1}, // R
    {1, 0, 0, 1, 0, 1}, // S
    {1, 0, 1, 1, 0, 1}, // T
    {0, 1, 0, 0, 1, 1}, // U
    {0, 1, 0, 1, 1, 1}, // V
    {1, 0, 1, 1, 1, 0}, // W
    {1, 1, 0, 0, 1, 1}, // X
    {1, 1, 1, 0, 1, 1}, // Y
    {0, 1, 1, 0, 1, 1}  // Z
};
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
// Función para controlar el movimiento de los motores
void mover_motor(int steps, int dirPin, int stepPin) {
    gpio_set_level(dirPin, steps > 0 ? 1 : 0);
    for (int i = 0; i < abs(steps); i++) {
        gpio_set_level(stepPin, 1);
        vTaskDelay(1 / portTICK_PERIOD_MS);
        gpio_set_level(stepPin, 0);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

// Función para mover el servo y troquelar el papel
void troquelar() {
    gpio_set_level(SERVO_PIN, 1); // Baja el punzón
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(SERVO_PIN, 0); // Sube el punzón
    vTaskDelay(500 / portTICK_PERIOD_MS);
}

// Función para traducir una palabra y troquelarla en Braille
void traducir_y_troquelar(const char* texto) {
    int espacio_entre_caracteres = 5; // Define el espacio entre caracteres en pasos
    
    for (int i = strlen(texto) - 1; i >= 0; i--) {
        char caracter = texto[i];
        if (caracter >= 'A' && caracter <= 'Z') {
            int letra_idx = caracter - 'A';
            int* matriz = diccionario_braille[letra_idx];
            for (int j = 0; j < 6; j++) {
                if (matriz[j] == 1) {
                    troquelar(); // Baja el punzón para troquelar el punto
                }
                mover_motor(espacio_entre_caracteres, X_DIR_PIN, X_STEP_PIN);
            }
            mover_motor(espacio_entre_caracteres, Y_DIR_PIN, Y_STEP_PIN); // Movemos a la siguiente letra
        }
    }
}
/*==================[external functions definition]==========================*/
void app_main(void) {
    gpio_set_direction(X_STEP_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(X_DIR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(Y_STEP_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(Y_DIR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(SERVO_PIN, GPIO_MODE_OUTPUT); // Inicialización del pin HC-RES84
    
    // Ejemplo de palabra a traducir
    const char* palabra = "HOLA";
    traducir_y_troquelar(palabra);
}
/*==================[end of file]============================================*/








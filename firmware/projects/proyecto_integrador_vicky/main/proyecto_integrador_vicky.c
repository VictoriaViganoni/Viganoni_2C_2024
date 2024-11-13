/*! @mainpage Sistema de Impresión en Braille "SenseDot"
 *
 * @section genDesc Descripción General
 * Este programa permite traducir texto en caracteres Braille y troquelarlos en papel utilizando un robot cartesiano.
 * Los motores controlan el movimiento en los ejes X e Y, mientras que un servo acciona el punzón para crear los puntos.
 *
 * @section hardConn Conexión de Hardware
 *
 * |    Peripheral   |   ESP32   |
 * |:---------------:|:----------|
 * | Motor X STEP    | GPIO_16   |
 * | Motor X DIR     | GPIO_17   |
 * | Motor Y STEP    | GPIO_15   |
 * | Motor Y DIR     | GPIO_22   |
 * | Servo           | GPIO_23   |
 * | UART TX         | GPIO_1    |
 * | UART RX         | GPIO_3    |
 * 
 * * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 1/11/2024 | Document creation		                         |
 *
 * @autor: María Victoria Viganoni (maria.viganoni@ingeniera.uner.edu.ar)
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
#include "esp_system.h"
#include "string.h"
#include "servo_sg90.h"

/*==================[macros and definitions]=================================*/
/**
 * @def X_STEP_PIN
 * @brief Pin GPIO utilizado para controlar los pasos del motor en el eje X.
 */
#define X_STEP_PIN 16

/**
 * @def  X_DIR_PIN
 * @brief Pin GPIO utilizado para controlar la dirección del motor en el eje X.
 */
#define X_DIR_PIN 17

/**
 * @def Y_STEP_PIN 
 * @brief Pin GPIO utilizado para controlar los pasos del motor en el eje Y.
 */
#define Y_STEP_PIN 15

/**
 * @def Y_DIR_PIN 
 * @brief Pin GPIO utilizado para controlar la dirección del motor en el eje Y.
 */
#define Y_DIR_PIN 22

/**
 * @def SERVO_PIN
 * @brief Pin GPIO utilizado para controlar el servo que acciona el punzón de troquelado.
 */
#define SERVO_PIN 03

/**
 * @def ESPACIO_ENTRE_PUNTOS
 * @brief Número de pasos entre puntos en la matriz Braille para ajustar el espaciado vertical.
 */
#define ESPACIO_ENTRE_PUNTOS 15

/**
 * @def ESPACIO_ENTRE_CARACTERES
 * @brief Número de pasos entre caracteres en la matriz Braille para ajustar el espaciado horizontal.
 */
#define ESPACIO_ENTRE_CARACTERES 30

/** 
 * @def diccionario_braille
 * @brief 
 */
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
    {1, 0, 1, 1, 0, 1}, // S
    {1, 0, 1, 1, 0, 1}, // T
    {0, 1, 0, 0, 1, 1}, // U
    {0, 1, 0, 1, 1, 1}, // V
    {1, 0, 1, 1, 1, 0}, // W
    {1, 1, 0, 0, 1, 1}, // X
    {1, 1, 1, 0, 1, 1}, // Y
    {0, 1, 1, 0, 1, 1}  // Z
};

/*==================[internal functions declaration]=========================*/

/** 
 * @fn  inicializar_servo()
 * @brief Inicializa el servo motor configurando su frecuencia de funcionamiento.
 * @return
 * @param 
 */
void inicializar_servo() {
    ServoInit(SERVO_0, SERVO_PIN);  
}

/** 
 * @fn  mover_motor()
 * @brief Controla el movimiento de un motor paso a paso en una dirección específica.
 * Esta función mueve el motor paso a paso en la cantidad de pasos indicada y en la dirección
 * establecida por el pin `dirPin`. Se controla el pin `stepPin` para generar los pulsos de
 * paso.
 * @return
 * @param steps Cantidad de pasos que debe realizar el motor.
 * @param dirPin Pin GPIO para definir la dirección del motor.
 * @param stepPin Pin GPIO para generar los pulsos de paso del motor.
 */
void mover_motor(int steps, gpio_t dirPin, gpio_t stepPin) {
    GPIOState(dirPin, steps > 0 ? true : false);
    printf("Moviendo motor en %s con %d pasos.\n", (dirPin == X_DIR_PIN ? "X" : "Y"), steps); 
    for (int i = 0; i < abs(steps); i++) {
        GPIOOn(stepPin);
        vTaskDelay(50 / portTICK_PERIOD_MS);
        GPIOOff(stepPin);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}


/** 
 * @fn  void troquelar()
 * @brief Activa el servo motor para simular el proceso de troquelado en Braille.
 * La función mueve el servo motor hacia una posición para simular el punzonado
 * y luego lo regresa a su posición original.
 * @return
 * @param 
 */
void troquelar() {
    printf("Troquelando...\n");
    ServoMove(SERVO_0, -45);  
    vTaskDelay(50/ portTICK_PERIOD_MS); 
    ServoMove(SERVO_0, 45); 
    vTaskDelay(50 / portTICK_PERIOD_MS); 
}

/** 
 * @fn traducir_y_troquelar()
 * @brief Traduce una cadena de texto a Braille y realiza el troquelado correspondiente.
 * Esta función toma una cadena de texto en formato de letras mayúsculas, la convierte a Braille,
 * y utiliza los motores y el servo para troquelar los caracteres en papel. Recorre cada letra
 * de la cadena, identificando los puntos de Braille activos en una matriz y ejecutando movimientos
 * en consecuencia.
 * @return
 * @param texto Puntero a la cadena de texto a traducir y troquelar en Braille.
 */
void traducir_y_troquelar(const char* texto) {
    printf("Traduciendo y troquelando la palabra: %s\n", texto); 
    for (int i = 0; i < strlen(texto); i++) {
        char caracter = texto[i];
        if (caracter >= 'A' && caracter <= 'Z') {
            int letra_idx = caracter - 'A';
            const int* matriz = diccionario_braille[letra_idx];
             printf("Troquelando letra: %c\n", caracter);


            for (int col = 0; col < 2; col++) {
                for (int row = 0; row < 3; row++) {
                    int punto_idx = row + col * 3;
                    if (matriz[punto_idx] == 1) {
                        troquelar();  
                    }
                    if (row < 2) {
                        mover_motor(ESPACIO_ENTRE_PUNTOS, Y_DIR_PIN, Y_STEP_PIN);
                    }
                }
                
                mover_motor(-ESPACIO_ENTRE_PUNTOS * 2, Y_DIR_PIN, Y_STEP_PIN); 
                if (col == 0) {
                    mover_motor(ESPACIO_ENTRE_PUNTOS, X_DIR_PIN, X_STEP_PIN); 
                }
            }
            mover_motor(-ESPACIO_ENTRE_PUNTOS, X_DIR_PIN, X_STEP_PIN); 
            mover_motor(ESPACIO_ENTRE_CARACTERES, X_DIR_PIN, X_STEP_PIN); 
        }
    }
}

void app_main(void) {
    GPIOInit(X_STEP_PIN, GPIO_OUTPUT);
    GPIOInit(X_DIR_PIN, GPIO_OUTPUT);
    GPIOInit(Y_STEP_PIN, GPIO_OUTPUT);
    GPIOInit(Y_DIR_PIN, GPIO_OUTPUT);
    GPIOInit(SERVO_PIN, GPIO_OUTPUT);
    inicializar_servo();

    char palabra[] = "HOLA";  
    printf("Iniciando troquelado de: %s\n", palabra);
    traducir_y_troquelar(palabra);
    printf("Troquelado completo.\n");
}


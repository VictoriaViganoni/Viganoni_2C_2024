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

#define X_STEP_PIN 16
#define X_DIR_PIN 17
#define Y_STEP_PIN 15
#define Y_DIR_PIN 22
#define SERVO_PIN 03
#define UART_PORT 0
#define BUF_SIZE 1024
#define ESPACIO_ENTRE_PUNTOS 15
#define ESPACIO_ENTRE_CARACTERES 30


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


//void configurar_uart() {
//    serial_config_t uart_config = {
//        .port = UART_PORT,
//        .baud_rate = 115200,
//        .func_p = UART_NO_INT,
//        .param_p = NULL
//    };
//    UartInit(&uart_config);
//}

//int leer_texto_uart(char* buffer, int max_len) {
//    uint8_t len = UartReadBuffer(UART_PORT, (uint8_t*)buffer, max_len - 1);
//    if (len > 0) {
//        buffer[len] = '\0';
//    }
//    return len;
//}

void inicializar_servo() {
    ServoInit(SERVO_0, SERVO_PIN);  
}

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

void troquelar() {
    printf("Troquelando...\n");
    ServoMove(SERVO_0, -45);  
    vTaskDelay(50/ portTICK_PERIOD_MS); 
    ServoMove(SERVO_0, 45); 
    vTaskDelay(50 / portTICK_PERIOD_MS); 
}

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
    
//    configurar_uart();

//    char palabra[BUF_SIZE];
//   UartSendString(UART_PORT, "Ingrese la palabra a traducir en Braille:\n");

  //    while (1) {
  //        int len = leer_texto_uart(palabra, sizeof(palabra)); 
  //        if (len > 0) {
  //            UartSendString(UART_PORT, "Traduciendo y troquelando: ");
  //            UartSendString(UART_PORT, palabra);
  //            UartSendString(UART_PORT, "\n");
  //            traducir_y_troquelar(palabra);
  //        }
  //    }
  //     int len = leer_texto_uart(palabra, sizeof(palabra));
  //
//     if (len > 0) {
//        UartSendString(UART_PORT, "Traduciendo y troquelando: ");
//        UartSendString(UART_PORT, palabra);
//        UartSendString(UART_PORT, "\n");
//        traducir_y_troquelar(palabra);  // Traduce y troquela la palabra
//     }

    char palabra[] = "HOLA";  
    printf("Iniciando troquelado de: %s\n", palabra);
    traducir_y_troquelar(palabra);
    printf("Troquelado completo.\n");
}


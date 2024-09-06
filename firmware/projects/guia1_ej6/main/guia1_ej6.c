/*! @mainpage Proyecto 1: ejercicio 6
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
 * | 23/08/2024 | Document creation		                         |
 * 
 * @section Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos de salida y dos vectores de 
 * estructuras del tipo  gpioConf_t. Uno  de estos vectores es igual al definido en el punto anterior y el otro vector 
 * mapea los puertos con el dígito del LCD a donde mostrar un dato:
 *	Dígito 1 -> GPIO_19
 *	Dígito 2 -> GPIO_18
 *	Dígito 3 -> GPIO_9
 *
 *La función deberá mostrar por display el valor que recibe. Reutilice las funciones creadas en el punto 4 y 5. 
 *Realice la documentación de este ejercicio usando Doxygen.
 *
 *No utilice la funcion del ej 4.
 *
 * @author maria victoria viganoni (maria.viganoni@ingeniera.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
#define N_BITS 4
#define LCD_DIGITS 3  
/*==================[macros and definitions]=================================*/
/** @struct gpioConfig_t
 * @brief struct para configurar cada pin GPIO
 * @return gpio_t pin; (numero de pin GPIO)
 * @return io_t dir; (direccion del pin de entrada o salida)
*/

/** @fn convertToBcdArray(int8_t)
 *  @brief Convierte un número de 32 bits a un array BCD.
 *  Esta función convierte un número de 32 bits en su representación BCD
 *  y lo almacena en un array.
 *  @param data El número a convertir.
 *  @param digits Cantidad de dígitos a mostrar en el LCD.
 *  @param bcd_number Array donde se almacenará el número convertido.
 *  @return 0 si la conversión fue exitosa, -1 en caso de error.
 */
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number);

/** @fn BCDtoGPIO(void)
 *  @brief Configura los pines GPIO según un dígito BCD.
 *  @param digit El dígito BCD que se mostrará.
 *  @param gpio_config Configuración de los pines GPIO.
 *  @return
 */
void BCDtoGPIO(uint8_t digit, gpioConfig_t *gpio_config);

/** @fn GPIOInit(void)
 *  @brief Inicializa los pines GPIO.
 *  Esta función inicializa un pin GPIO con una dirección de entrada o salida.
 *  @param pin El número del pin GPIO.
 *  @param dir Dirección del pin: entrada (0) o salida (1).
 *  @return
 */
void GPIOInit(gpio_t pin, io_t dir);

/** @fn GPIOOn(void)
 *  @brief Enciende un pin GPIO.
 *  Esta función enciende el pin GPIO especificado.
 *  @param pin El número del pin GPIO a encender.
 *  @return
 */
void GPIOOn(gpio_t pin);

/** @fn GPIOOff(void)
 *  @brief Apaga un pin GPIO.
 *  Esta función apaga el pin GPIO especificado.
 *  @param pin El número del pin GPIO a apagar.
 *  @return
 */
void GPIOOff(gpio_t pin);

/** @fn displayNumberOnLCD(void)
 *  @brief Muestra un número en el display LCD.
 *  Esta función recibe un número, lo convierte a BCD y lo muestra
 *  en una pantalla LCD controlando los pines GPIO correspondientes.
 *  @param data El número a mostrar.
 *  @param data_gpio_config Configuración de los pines GPIO para los datos.
 *  @param digit_gpio_config Configuración de los pines GPIO para los dígitos del LCD.
 *  @return
 */
void displayNumberOnLCD(uint32_t data, gpioConfig_t *data_gpio_config, gpioConfig_t *digit_gpio_config);

/** @fn app_main(void)
 *  @brief Función principal del programa.
 *  Inicializa las configuraciones de los pines GPIO y muestra un número
 *  en la pantalla LCD.
 *  @return
 */
void app_main(void);

/*==================[internal data definition]===============================*/
typedef struct {
    gpio_t pin;
    io_t dir;
} gpioConfig_t;
/*==================[internal functions declaration]=========================*/
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number);
void BCDtoGPIO(uint8_t digit, gpioConfig_t *gpio_config);
void GPIOInit(gpio_t pin, io_t dir);
void GPIOOn(gpio_t pin);
void GPIOOff(gpio_t pin);
void GPIOstate(gpio_t pin, uint8_t state);
/*==================[external functions definition]==========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number) 
{
	if (digits > 10) {
        return -1;  // Error
    }
    // Inicializar el array de salida a 0
    for (uint8_t i = 0; i < digits; i++) {
        bcd_number[i] = 0;
    }
    for (uint8_t i = 0; i < digits; i++) {
        bcd_number[digits - i - 1] = data % 10;  // Guardar el digito menos significativo
        data = data / 10;  // Eliminar el digito menos significativo 
    }
    
    if (data > 0) {
        return -1;  
    }

    return 0;
}

// Implementación de BCDtoGPIO
void BCDtoGPIO(uint8_t digit, gpioConfig_t *gpio_config) {
    for (uint8_t i = 0; i < N_BITS; i++) {
        GPIOInit(gpio_config[i].pin, gpio_config[i].dir);
    }
    for (uint8_t i = 0; i < N_BITS; i++) {
            if ((digit & (1 << i)) == 0) {
                GPIOOff(gpio_config[i].pin);
            } else {
                GPIOOn(gpio_config[i].pin);
            }
        }
}
// Función para mostrar un número en el display LCD
void displayNumberOnLCD(uint32_t data, gpioConfig_t *data_gpio_config, gpioConfig_t *digit_gpio_config) {
    uint8_t bcd_array[LCD_DIGITS];
    
    if (convertToBcdArray(data, LCD_DIGITS, bcd_array) != 0) {
        printf("Error: el número es demasiado grande para la cantidad de dígitos.\n");
        return;
    }

    for (uint8_t i = 0; i < LCD_DIGITS; i++) {
        GPIOInit(digit_gpio_config[i].pin, digit_gpio_config[i].dir);
    }

    for (uint8_t i = 0; i < LCD_DIGITS; i++) {
        
        for (uint8_t j = 0; j < LCD_DIGITS; j++) {
            GPIOOff(digit_gpio_config[j].pin);
        }
        
        GPIOOn(digit_gpio_config[i].pin);
        BCDtoGPIO(bcd_array[i], data_gpio_config);
    }
}

void app_main(void) {
    // Configuración de gpio de datos y dígitos
    gpioConfig_t data_gpio_config[N_BITS] = {
        {GPIO_20, 1},
        {GPIO_21, 1},
        {GPIO_22, 1},
        {GPIO_23, 1}
    };
    
    gpioConfig_t digit_gpio_config[LCD_DIGITS] = {
        {GPIO_19, 1}, 
        {GPIO_18, 1}, 
        {GPIO_9, 1}   
    };

    uint32_t number = 450; 

    displayNumberOnLCD(number, data_gpio_config, digit_gpio_config);
}
/*==================[end of file]============================================*/
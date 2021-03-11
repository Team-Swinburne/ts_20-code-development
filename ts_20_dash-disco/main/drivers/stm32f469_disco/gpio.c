/*
*   Dashboard Custom GPIO Driver Used For Easy GPIO Control.
*   
*   PROGRAM STRUCTURE
*   =================
*   Functions
*     GPIO_Init         - Enable the appropriate GPIO clocks used by our board.
*     GPIO_Pin_Init     - Initialise a specific GPIO pin.
*     GPIO_State        - Change the state of a GPIO pin (Low/False - High/True).
*     GPIO_Direction    - Change the direction of a GPIO pin (Input/False - Output/True).
*   -----------------
*/
#include "gpio.h"


/*
* GPIO Port Initiation.
* Param:
*   None.
* Usage:
*   Called by driver.c to initiate GPIO ports.
*/
void GPIO_Init() {
    // Initiate used GPIO Ports, others can be enabled using same layout, i.e. __GPIOx_CLK_ENABLE() where x is the desired port.
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOG_CLK_ENABLE();
    __GPIOH_CLK_ENABLE();  
}

/*
* GPIO Pin Initiation.
* Param:
*   GPIO_Struct *gpio_object, pointer to a certain GPIO object which is going to be overwritten within this initiation function.
*   char gpioPort, the GPIO port that the pin is used in. Currently only A, B, G & H are used.
*   bool direction, whether the pin is going to be an output/true or an input/false.
* Usage:
*   Called by driver.c to initiate led pins.
*   Can be used by any other GPIO if required.
*/
void GPIO_Pin_Init(GPIO_Struct *gpio_object, char gpioPort, int gpioPin, bool direction) {
    // Set the GPIO port of the gpio object according to the gpioPort parameter.
    switch (gpioPort) {
    case 'A':
        gpio_object->GPIO_Port = GPIOA;
        break;
    case 'B':
        gpio_object->GPIO_Port = GPIOB;
        break;
    case 'G':
        gpio_object->GPIO_Port = GPIOG;
        break;
    case 'H':
        gpio_object->GPIO_Port = GPIOH;
        break;
    }

    // Set the GPIO pin of the gpio object according to the gpioPin parameter.
    switch (gpioPin) {
    case 1:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_1;
        break;
    case 2:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_2;
        break;
    case 3:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_3;
        break;
    case 4:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_4;
        break;
    case 5:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_5;
        break;
    case 6:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_6;
        break;
    case 7:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_7;
        break;
    case 8:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_8;
        break;
    case 9:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_9;
        break;
    case 10:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_10;
        break;
    case 11:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_11;
        break;
    case 12:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_12;
        break;
    case 13:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_13;
        break;
    case 14:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_14;
        break;
    case 15:
        gpio_object->GPIO_InitSt.Pin = GPIO_PIN_15;
        break;
    }

    if (direction) { // If the pin is to be an output, set it so with a pull up.
        gpio_object->GPIO_InitSt.Mode = GPIO_MODE_OUTPUT_PP;
        gpio_object->GPIO_InitSt.Pull = GPIO_PULLUP; 
    } else { // If the pin is to be an input, set it so with no pull up/down.
        gpio_object->GPIO_InitSt.Mode = GPIO_MODE_INPUT;
        gpio_object->GPIO_InitSt.Pull = GPIO_NOPULL; 
    }
    // All GPIOs use the same speed.
    gpio_object->GPIO_InitSt.Speed = GPIO_SPEED_HIGH;

    // Use the HAL library to actually initiate the GPIO pin.
    HAL_GPIO_Init(gpio_object->GPIO_Port, &gpio_object->GPIO_InitSt);
}



/*
* Basic GPIO control procedure.
* Param:
*   GPIO_Struct *gpio_object, used to select the desired GPIO port and pin.
*   bool state, used to turn and led on/true or off/false.
* Usage:
*   Used to either turn a digital output on or off, e.g.
*   gpio_state(&BSP_led, true) // To turn on the BSP led.
*/
void GPIO_State(GPIO_Struct *gpio_object, bool state)
{
  if (state) {
    HAL_GPIO_WritePin(gpio_object->GPIO_Port, gpio_object->GPIO_InitSt.Pin, GPIO_PIN_SET);
  } else {
    HAL_GPIO_WritePin(gpio_object->GPIO_Port, gpio_object->GPIO_InitSt.Pin, GPIO_PIN_RESET);
  }
}


/*
* Changing GPIO direction.
* Param:
*   GPIO_Struct *gpio_object, used to change configurations.
*   bool direction, used to turn determine a direction (true/output, false/input).
* Usage:
*   Used by led_array_control() as certain GPIOs need to be set to inputs for other outputs to work.
*/
void GPIO_Direction(GPIO_Struct *gpio_object, bool direction) {
  HAL_GPIO_DeInit(gpio_object->GPIO_Port, gpio_object->GPIO_InitSt.Pin);
  if (direction) {
    gpio_object->GPIO_InitSt.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_object->GPIO_InitSt.Pull = GPIO_PULLUP;
  } else {
    gpio_object->GPIO_InitSt.Mode = GPIO_MODE_INPUT;
    gpio_object->GPIO_InitSt.Pull = GPIO_NOPULL;
  }
  HAL_GPIO_Init(gpio_object->GPIO_Port, &gpio_object->GPIO_InitSt);
}



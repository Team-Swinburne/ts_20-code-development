#define PIN_SERIAL_TX   PA_9
#define PIN_SERIAL_RX   PA_10

#define PIN_I2C_SDA     PB_7
#define PIN_I2C_SCL     PB_6

#define PIN_CAN_RXD     PB_8
#define PIN_CAN_TXD PB_9

#if defined(USE_UCM3) || defined(USE_UCM4)
    #define PIN_DIGITAL1_IN PB_10
    #define PIN_DIGITAL2_IN PB_11
    #define PIN_DIGITAL3_IN PB_12
    #define PIN_DIGITAL4_IN PB_13

    #define PIN_DRIVER1_GLV PB_0 //driver pins with PWM capability
    #define PIN_DRIVER2_GLV PB_1
#else
    #define PIN_DIGITAL1_IN PB_12
    #define PIN_DIGITAL2_IN PB_13

    #define PIN_DRIVER1_GLV PB_15 //only on off
    #define PIN_DRIVER2_GLV PB_14

    #define PIN_PWM1_5V PB_0 //pwm cabailibty
    #define PIN_PWM2_5V PB_1
#endif
#   Brake Module

The brake module is driven entirely by the circuitry on the PCB through logic gates so the purpose of this code is only to transfer broadcast those data on CAN. 

| Pin   | Function              |
|:----- |----------:            |
|PA0    | TEMP NTC              |
|PA2    | UARTTX                |
|PA3    | UARTRX                |
|PA4    | Sensor 1              |
|PA5    | Sensor 2              |
|PA6    | LOW PRESSURE (ISO)    |
|PA7    | HIGH PRESSURE (ISO)   |
|PA8    | 5KW (ISO)             |
|PA9    | BSPD OK (ISO)         |
|PA11   | USBDM                 |
|PA12   | USBDP                 |
|PA13   | SWDIO                 |
|PA14   | SWCLK                 |
|&#xfeff;                       |
|PB0    | CAN TX INDC           |
|PB1    | CAN RX INDC           |
|&#xfeff;|                      |
|PB8    | CAN1 RX               |
|PB9    | CAN1 TX               |
|&#xfeff;|                      |
|PB12   | BSPD DELAY (ISO)      |
|PB13   | CLOCK (ISO)           |   
|&#xfeff;|                      |
|PC13   | LED_BUILTIN           |
|PC14   | XTALC                 |
|PC15   | XTALD                 |
|&#xfeff;|                      |
|PD0    | XTALA                 |
|PD1    | XTALB                 |   

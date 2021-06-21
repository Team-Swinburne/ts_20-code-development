#   Brake Module

The brake module is driven entirely by the circuitry on the PCB through logic gates so the purpose of this code is only to transfer broadcast those data on CAN. 

| Pin   | Function              |
|:----- |----------:            |
|PA0    | PA0 TEMP              |
|PA2    | UARTTX                |
|PA3    | UARTRX                |
|&#xfeff|                       |
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
|PB6    | SENSOR2               |
|PB7    | SENSOR1               |
|PB8    | CAN1 RD               |
|PB9    | CAN1 TD               |
|&#xfeff;|                      |
|PB12   | BSPD DELAY (ISO)      |
|PB13   | CLOCK (ISO)           |   
|PB14   | HIGH PRESSURE (ISO)   |
|PB15   | LOW PRESSURE (ISO)    |
|&#xfeff;|                      |
|PC13   | LED_BUILTIN           |
|PC14   | XTALC                 |
|PC15   | XTALD                 |
|&#xfeff;|                      |
|PD0    | XTALA                 |
|PD1    | XTALB                 |   

## Authors
* **Michael Cochrane**
* **Thomas Bennett**
* **Nam Tran**

## Acknowledgments
* [LMV338 information](https://forum.arduino.cc/index.php?topic=285688.15)
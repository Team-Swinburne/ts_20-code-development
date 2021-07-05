/*  TEAM SWINBURNE - TS21 
    BRAKE MODULE - HARDWARE REVISION 5
    ETHAN JONES, MICHAEL COCHRANE, THOMAS BENNETT

    INFORMATION: This code controlls the Brake Module (hardware revision 5).
    The brake module monitors the brakes and BSPD signal and sends messages
    through CAN to notify other CAN nodes about the inputs.

    Potentiometers are used to set Low pressure and High pressure. (current calibration as of 04/07/2021) is
    Brake disconnected = 600mV
    Brake connected = 800mV
    Low pressure reference = 750mV
    High pressure reference = 1100mV
    
    REVISION 2.0 (04/07/2021)
    Revision     Date          Comments
    --------   ----------     ------------
    0.0        N/A            Arduino code
    1.0        21/06/2021     Ported code to mbed
    2.0        04/07/2021     Now supports the new CAN standard and rewrote the code
*/

#include <mbed.h>
#include <CAN.h>
#include "can_addresses.h"
#include "BrakeModule_info.h" //This header stores information about module and calibrations

//Functions
void BrakeModuleUpdate(); //This function reads the pins and updates all the values for the brake module.
uint8_t BrakePercent(uint16_t Brake_raw, uint16_t Brake_max, uint16_t Brake_min); //This function calculates the brake percentage

void CAN_brakeModule_TX_Heartbeat(); //This function transmits the Heartbeat CAN message
void CAN_brakeModule_TX_Error(); //This function transmits the Error/Warning CAN message
void CAN_brakeModule_TX_Digital_1(); //This function transmits the Digital 1 CAN message
void CAN_brakeModule_TX_Analog_1(); //This function transmits the Analog 1 CAN message

void CAN_RX(); //This function is for receiving on CAN

//void Serial_Print(); //Used for debugging

//PIN INITIALIZATION #######################################################
//Digital In:
DigitalIn LowPressure(PA_6); //high pressure
DigitalIn HighPressure(PA_7); //low pressure
DigitalIn CurrentSensor(PA_8); //current sensor 5KW
DigitalIn BSPD(PA_9); //BSPD_OK (no delay)
DigitalIn BSPD_Delay(PB_12); //BSPD_OK (10 second delay)

//Digital Out:
DigitalOut canTXLed(PB_0); //CAN TX LED
DigitalOut canRXLed(PB_1); //CAN RX LED
//DigitalOut debugLedOut(PC_13); //Debug LED

//Analog In:
AnalogIn sensor1(PA_4); //sensor 1
AnalogIn sensor2(PA_5); //sensor 2

//Others:
//Serial pc(PA_2, PA_3); //Serial UART (Debug)
CAN can1(PB_8, PB_9); //CANBUS
//#########################################################################

//Objects and structs
CANMessage can1_msg; //Object that formats the CAN message
HeartBeat_struct HeartBeat; //Struct contains the variables used for the HeartBeat
BrakeModule_struct BrakeModule; //Struct contains the variables used for the BrakeModule

int main() 
{
  can1.frequency(CANBUS_FREQUENCY); //Set canbus frequency
  
  //Creates tickers
  Ticker ticker_CAN_HeartBeat;
  Ticker ticker_CAN_Error;
  Ticker ticker_CAN_Digital_1;
  Ticker ticker_CAN_Analog_1;

  //Configure tickers
  ticker_CAN_HeartBeat.attach(&CAN_brakeModule_TX_Heartbeat, CAN_HEARTBEAT_PERIOD);
  ticker_CAN_Error.attach(&CAN_brakeModule_TX_Error, CAN_ERROR_PERIOD);
  ticker_CAN_Digital_1.attach(&CAN_brakeModule_TX_Digital_1, CAN_DIGITAL_1_PERIOD);
  ticker_CAN_Analog_1.attach(&CAN_brakeModule_TX_Analog_1, CAN_ANALOG_1_PERIOD);

  while(1) 
  {
    BrakeModuleUpdate();
    CAN_RX();
    //Serial_Print(); //Used for debugging.
  }

  return 0;
}

void BrakeModuleUpdate()
{
  BrakeModule.brake1_raw = (uint16_t)(1000*sensor1.read());
  BrakeModule.brake2_raw = (uint16_t)(1000*sensor2.read());
  BrakeModule.High_Pressure = !(HighPressure.read()); //Must be inverted
  BrakeModule.Low_Pressure = LowPressure.read();
  BrakeModule.five_kW = CurrentSensor.read();
  BrakeModule.BSPD_OK = BSPD.read();
  BrakeModule.BSPD_OK_delay = BSPD_Delay.read();

  BrakeModule.brake1_percent = BrakePercent(BrakeModule.brake1_raw, BRAKE1_MAX, BRAKE1_MIN);
  BrakeModule.brake2_percent = BrakePercent(BrakeModule.brake2_raw, BRAKE2_MAX, BRAKE2_MIN);
  BrakeModule.brake_avg_percent = (BrakeModule.brake1_percent + BrakeModule.brake2_percent)/2.0;
}

uint8_t BrakePercent(uint16_t Brake_raw, uint16_t Brake_max, uint16_t Brake_min)
{
  float brake_percent = (((float)Brake_raw - Brake_min)/(Brake_max - Brake_min))*100.0;
  // Filter data to ensure it is within 0-100 percent
  brake_percent = max(brake_percent - DEADZONE,brake_percent);
  brake_percent = ((brake_percent - DEADZONE) / (100 - DEADZONE)) * 100.0;
  brake_percent = min(brake_percent, float(100));
  return (uint8_t)brake_percent;
}

void CAN_brakeModule_TX_Heartbeat()
{
  (HeartBeat.Counter >= 255) ? HeartBeat.Counter = 0 : HeartBeat.Counter++;

  char TX_data[4] = { 0 };

  TX_data[CAN_HEARTBEAT_STATE] = HeartBeat.State;
  TX_data[CAN_HEARTBEAT_COUNTER] = HeartBeat.Counter;
  TX_data[CAN_HEARTBEAT_PCB_TEMP] = 0;
  TX_data[CAN_HEARTBEAT_HARDWARE_REVISION] = 5;
  //TX_data[CAN_HEARTBEAT_COMPILE_DATE] = 0;
  //TX_data[CAN_HEARTBEAT_COMPILE_TIME] = 0;

  canTXLed.write(1); //turn on CAN TX indicator
  can1.write(CANMessage((CAN_BRAKE_MODULE_BASE_ADDRESS + TS_HEARTBEAT_ID), TX_data, 4));
  canTXLed.write(0); //turn off CAN TX indicator
}

void CAN_brakeModule_TX_Error()
{
  char TX_data[1] = { 0 };

  TX_data[CAN_ERROR_CODE] |= (((BrakeModule.BSPD_OK == 1) ? 0 : 1) << 7); //If bspd = 1, error code = 0 (no error) else error code = 128 (error)

  canTXLed.write(1); //turn on CAN TX indicator
  can1.write(CANMessage((CAN_BRAKE_MODULE_BASE_ADDRESS + TS_ERROR_WARNING_ID), TX_data, 1));
  canTXLed.write(0); //turn off CAN TX indicator
}

void CAN_brakeModule_TX_Digital_1()
{
  char TX_data[1] = { 0 };

  //Send via big endian
  TX_data[CAN_DIGITAL_1_BRAKE_MODULE_STATUSES] |= (BrakeModule.High_Pressure << 7);
  TX_data[CAN_DIGITAL_1_BRAKE_MODULE_STATUSES] |= (BrakeModule.Low_Pressure << 6);
  TX_data[CAN_DIGITAL_1_BRAKE_MODULE_STATUSES] |= (BrakeModule.five_kW << 5);
  TX_data[CAN_DIGITAL_1_BRAKE_MODULE_STATUSES] |= (BrakeModule.BSPD_OK << 4);
  TX_data[CAN_DIGITAL_1_BRAKE_MODULE_STATUSES] |= (BrakeModule.BSPD_OK_delay << 3);

  canTXLed.write(1); //turn on CAN TX indicator
  can1.write(CANMessage((CAN_BRAKE_MODULE_BASE_ADDRESS + TS_DIGITAL_1_ID), TX_data, 1));
  canTXLed.write(0); //turn off CAN TX indicator
}

void CAN_brakeModule_TX_Analog_1()
{
  char TX_data[3] = { 0 };

  TX_data[CAN_ANALOG_1_BRAKE1_PERCENT] = BrakeModule.brake1_percent;
  TX_data[CAN_ANALOG_1_BRAKE2_PERCENT] = BrakeModule.brake2_percent;
  TX_data[CAN_ANALOG_1_BRAKE_AVG_PERCENT] = BrakeModule.brake_avg_percent;

  canTXLed.write(1); //turn on CAN TX indicator
  can1.write(CANMessage((CAN_BRAKE_MODULE_BASE_ADDRESS + TS_ANALOGUE_1_ID), TX_data, 3));
  canTXLed.write(0); //turn off CAN TX indicator
}

void CAN_RX()
{
  if (can1.read(can1_msg)) // poll for rx
  {
    canRXLed.write(1);        // turn on CAN RX indicator
    //RX_data[]=rxbytes;
    canRXLed.write(0);       // turn off CAN RX indicator
  }
}

/*
void Serial_Print()
{
  pc.printf("Brake Module Values:\r\n");
  pc.printf("Sensor 1 & 2 raw:     ");
  pc.printf("%d", BrakeModule.brake1_raw); 
  pc.printf("   ");
  pc.printf("%d\r\n", BrakeModule.brake2_raw); 
  pc.printf("Sensor 1 & 2 percent: ");
  pc.printf("%d", BrakeModule.brake1_percent); 
  pc.printf("   ");
  pc.printf("%d", BrakeModule.brake2_percent); 
  pc.printf("   ");
  pc.printf("%d\r\n", BrakeModule.brake_avg_percent); 
  pc.printf("High Pressure:        ");
  pc.printf("%d\r\n", BrakeModule.High_Pressure); 
  pc.printf("Low Pressure:         ");
  pc.printf("%d\r\n", BrakeModule.Low_Pressure); 
  pc.printf("5kW:                  ");
  pc.printf("%d\r\n", BrakeModule.five_kW);
  pc.printf("BSPD OK:              ");
  pc.printf("%d\r\n", BrakeModule.BSPD_OK);
  pc.printf("BSPD OK (Delay):      ");
  pc.printf("%d\r\n\r\n", BrakeModule.BSPD_OK_delay);

  wait(0.5);
}
*/
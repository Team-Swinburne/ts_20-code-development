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

//void Serial_Print(); //Used for debugging

/* -------------------------------------------------------------------------- */
/*                                  PIN INIT                                  */
/* -------------------------------------------------------------------------- */

DigitalIn LowPressure(PA_6); //high pressure
DigitalIn HighPressure(PA_7); //low pressure
DigitalIn CurrentSensor(PA_8); //current sensor 5KW
DigitalIn BSPD(PA_9); //BSPD_OK (no delay)
DigitalIn BSPD_Delay(PB_12); //BSPD_OK (10 second delay)

DigitalOut debugLedOut(PC_13); //Debug LED

AnalogIn sensor1(PA_4); //sensor 1
AnalogIn sensor2(PA_5); //sensor 2
AnalogIn lowRef(PB_1);
AnalogIn highRef(PB_0);

CAN can1(PB_8, PB_9); //CANBUS

/* -------------------------------------------------------------------------- */
/*                             OBJECTS AND STRUCTS                            */
/* -------------------------------------------------------------------------- */

//Objects and structs
CANMessage can1_msg; //Object that formats the CAN message
HeartBeat_struct HeartBeat; //Struct contains the variables used for the HeartBeat
BrakeModule_struct BrakeModule; //Struct contains the variables used for the BrakeModule
brake_calibration_s brake_calibration; //Struct contains the calibration variables

//Creates tickers
Ticker ticker_CAN_HeartBeat;
Ticker ticker_CAN_Error;
Ticker ticker_CAN_Digital_1;
Ticker ticker_CAN_Analog_1;

/* -------------------------------------------------------------------------- */
/*                               HANDY FUNCTIONS                              */
/* -------------------------------------------------------------------------- */
uint8_t raw_to_percent(float brake_raw, float brake_max, float brake_min)
{
  float brake_percent = ((brake_raw - brake_min)/(brake_max - brake_min))*100.0;

  // Filter data to ensure it is within 0-100 percent
  brake_percent = max(brake_percent - DEADZONE,brake_percent);
  brake_percent = ((brake_percent - DEADZONE) / (100 - DEADZONE)) * 100.0;
  brake_percent = min(brake_percent, 100.0f);
  return (uint8_t)brake_percent;
}

void BrakeModuleUpdate()
{
  BrakeModule.brake1_raw      = 3.3*sensor1.read();
  BrakeModule.brake2_raw      = 3.3*sensor2.read();
  BrakeModule.High_Pressure   = !(HighPressure.read()); //Must be inverted
  BrakeModule.Low_Pressure    = LowPressure.read();
  BrakeModule.five_kW         = CurrentSensor.read();
  BrakeModule.BSPD_OK         = BSPD.read();
  BrakeModule.BSPD_OK_delay   = BSPD_Delay.read();

  BrakeModule.brake1_percent    = raw_to_percent(BrakeModule.brake1_raw, brake_calibration.brake1_max, brake_calibration.brake1_min);
  BrakeModule.brake2_percent    = raw_to_percent(BrakeModule.brake2_raw, brake_calibration.brake2_max, brake_calibration.brake2_min);
  BrakeModule.brake_avg_percent = (BrakeModule.brake1_percent + BrakeModule.brake2_percent)/2.0;

  BrakeModule.brake_low_ref   = 3.3*lowRef.read();
  BrakeModule.brake_high_ref  = 3.3*highRef.read();
}


/* -------------------------------------------------------------------------- */
/*                                  CALLBACKS                                 */
/* -------------------------------------------------------------------------- */
void CAN_brakeModule_TX_Heartbeat()
{
  (HeartBeat.Counter >= 255) ? HeartBeat.Counter = 0 : HeartBeat.Counter++;

  char TX_data[4] = { 0 };

  TX_data[CAN_HEARTBEAT_STATE] = HeartBeat.State;
  TX_data[CAN_HEARTBEAT_COUNTER] = HeartBeat.Counter;
  TX_data[CAN_HEARTBEAT_PCB_TEMP] = 0;
  TX_data[CAN_HEARTBEAT_HARDWARE_REVISION] = 5;

  if (can1.write(CANMessage((CAN_BRAKE_MODULE_BASE_ADDRESS + TS_HEARTBEAT_ID), TX_data, 4))){
    debugLedOut = !debugLedOut;
  }
  else {
    can1.reset();
  }
}

void CAN_brakeModule_TX_Digital_1()
{
  char TX_data[5] = { 0 };

  TX_data[CAN_DIGITAL_1_BRAKE_HIGH_PRESSURE] = BrakeModule.High_Pressure;
  TX_data[CAN_DIGITAL_1_BRAKE_LOW_PRESSURE] = BrakeModule.Low_Pressure;
  TX_data[CAN_DIGITAL_1_BRAKE_5KW] = BrakeModule.five_kW;
  TX_data[CAN_DIGITAL_1_BRAKE_BSPD_OK] = BrakeModule.BSPD_OK;
  TX_data[CAN_DIGITAL_1_BRAKE_BSPD_OK_DELAY] = BrakeModule.BSPD_OK_delay;

  can1.write(CANMessage((CAN_BRAKE_MODULE_BASE_ADDRESS + TS_DIGITAL_1_ID), TX_data, 5));
}

void CAN_brakeModule_TX_Analog_1()
{
  char TX_data[7] = { 0 };

  TX_data[CAN_ANALOG_1_BRAKE1_PERCENT] = BrakeModule.brake1_percent;
  TX_data[CAN_ANALOG_1_BRAKE2_PERCENT] = BrakeModule.brake2_percent;
  TX_data[CAN_ANALOG_1_BRAKE_AVG_PERCENT]     = BrakeModule.brake_avg_percent;
  TX_data[CAN_ANALOG_1_BRAKE1_RAW]            = uint8_t(BrakeModule.brake1_raw*10);
  TX_data[CAN_ANALOG_1_BRAKE2_RAW]            = uint8_t(BrakeModule.brake2_raw*10);
  TX_data[CAN_ANALOG_1_BRAKE_LOW_REF]         = uint8_t(BrakeModule.brake_low_ref*10);
  TX_data[CAN_ANALOG_1_BRAKE_HIGH_REF]        = uint8_t(BrakeModule.brake_high_ref*10);
  //TX_data[CAN_TRAILBRAKE_PERCENT]             = BrakeModule.trailbrake_percent;
  
  can1.write(CANMessage((CAN_BRAKE_MODULE_BASE_ADDRESS + TS_ANALOGUE_1_ID), TX_data, 7));
}

void CAN_brakeModule_RX()
{
  if (can1.read(can1_msg)){
    switch(can1_msg.id){
      case (CAN_BRAKE_MODULE_BASE_ADDRESS+TS_SETPOINT_1_ID):
        brake_calibration.brake1_min = (float(can1_msg.data[0])/10.0);
        brake_calibration.brake1_max = (float(can1_msg.data[1])/10.0);
        brake_calibration.brake2_min = (float(can1_msg.data[2])/10.0);
        brake_calibration.brake2_max = (float(can1_msg.data[3])/10.0);
        break; 
    }
  }
}
/* -------------------------------------------------------------------------- */
/*                                    MAIN                                    */
/* -------------------------------------------------------------------------- */
int main() 
{
  // Disable interrupts for smooth startup routine.
	wait_ms(1000);
	
	__disable_irq();

  can1.frequency(CANBUS_FREQUENCY);
  can1.filter(CAN_BRAKE_MODULE_BASE_ADDRESS+TS_SETPOINT_1_ID, 0xFFF, CANStandard, 0); // set filter #0 to accept only standard messages with ID == RX_ID
	can1.attach(&CAN_brakeModule_RX);

  //Configure tickers
  ticker_CAN_HeartBeat.attach(&CAN_brakeModule_TX_Heartbeat, CAN_HEARTBEAT_PERIOD);
  ticker_CAN_Digital_1.attach(&CAN_brakeModule_TX_Digital_1, CAN_DIGITAL_1_PERIOD);
  ticker_CAN_Analog_1.attach(&CAN_brakeModule_TX_Analog_1, CAN_ANALOG_1_PERIOD);

  // Re-enable interrupts again, now that interrupts are ready.
	__enable_irq();

	// Allow some time to settle!
	wait_ms(1000);

  while(1) 
  {
    BrakeModuleUpdate();
    //Serial_Print(); //Used for debugging.
  }

  return 0;
}


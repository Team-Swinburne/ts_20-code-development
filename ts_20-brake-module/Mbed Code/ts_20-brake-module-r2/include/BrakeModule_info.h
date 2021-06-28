#ifndef __BRAKEMODULE_INFO_H__
#define __BRAKEMODULE_INFO_H__


//TS_19_2 MIN = 240, TS_19_2 MAX = 400
#define DEADZONE 30    // 30% Pedal deadzone
#define BRAKE1_MIN 240 // TS_19 = 980
#define BRAKE1_MAX 400 // TS_19 = 1755 
#define BRAKE2_MIN 240 // TS_19 = 400 
#define BRAKE2_MAX 400 // TS_19 = 1250
#define	CAN_HEARTBEAT_PERIOD 1 // Heartbeat message transmit period
#define CAN_ERROR_PERIOD 0.5 // Error message transmit period
#define CAN_DIGITAL_1_PERIOD 0.5 // Digital 1 message transmit period
#define CAN_ANALOG_1_PERIOD 0.02 //Analog 1 message transmit period

typedef struct BrakeModule_s{
  uint16_t     brake1_raw          = 0;
  uint8_t       brake1_percent      = 0;
  uint16_t     brake2_raw          = 0;
  uint8_t       brake2_percent      = 0;
  uint8_t     brake_avg_percent   = 0;
  uint8_t High_Pressure = 0;
  uint8_t Low_Pressure = 0;
  uint8_t five_kW = 0;
  uint8_t BSPD_OK = 0;
  uint8_t BSPD_OK_delay = 0;
}BrakeModule_struct;

typedef struct HeartBeat_s{
  uint8_t Counter = 0;
  uint8_t State = 1;
}HeartBeat_struct;

typedef enum CAN_HEARTBEAT_SIGNALS{
  CAN_HEARTBEAT_STATE,
  CAN_HEARTBEAT_COUNTER,
  CAN_HEARTBEAT_PCB_TEMP,
  CAN_HEARTBEAT_HARDWARE_REVISION,
  CAN_HEARTBEAT_COMPILE_DATE,
  CAN_HEARTBEAT_COMPILE_TIME
} can_HEARTBEAT_signals_t;

typedef enum CAN_ERROR_SIGNALS{
  CAN_ERROR_CODE
} can_ERROR_signals_t;

typedef enum CAN_DIGITAL_1_SIGNALS{
  CAN_DIGITAL_1_HIGH_PRESSURE,
  CAN_DIGITAL_1_LOW_PRESSURE,
  CAN_DIGITAL_1_FIVE_KW,
  CAN_DIGITAL_1_BSPD_OK,
  CAN_DIGITAL_1_BSPD_OK_DELAY
} can_DIGITAL_1_signals_t;

typedef enum CAN_ANALOG_1_SIGNALS{
  CAN_ANALOG_1_BRAKE1_PERCENT,
  CAN_ANALOG_1_BRAKE2_PERCENT,
  CAN_ANALOG_1_BRAKE_AVG_PERCENT,
} can_ANALOG_1_signals_t;

#endif //__BRAKEMODULE_INFO_H__
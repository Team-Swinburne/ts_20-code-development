// TEAM SWINBURNE - TS_22
// PRECHARGE CONTROLLER
// NATALIE NG
// Test File: test_HVSense.cpp (12/07/2022)

/*
BASIC FUNCTIONALITY TEST
Checks the precharge can detect an input voltage from the battery and/or motorcontroller
*/

//-----------------------------------------------
// Initialisation
//-----------------------------------------------

#include <mbed.h>
#include <CAN.h>
#include "Adafruit_ADS1015.h"
#include "precharge_pinout.h"

#include "can_addresses.h"
#include "ts_std_device.h"

#include "hv_tools.h"
#include "relays.h"
#include "precharge_discharge.h"

#include "imd.h"
#include "orion.h"
#include "watchdogs.h"

//-----------------------------------------------
// Device Parameters
//-----------------------------------------------

// HV Voltage Bridge Offset Resistors
#define MC_R_CAL 										5000
#define BATT_R_CAL 									5000
#define MINIMUM_PRECHARGE_VOLTAGE		400
#define MAXIMUM_PRECHARGE_VOLTAGE		600

// ADS1115 ADDR PINS: GND 48 - VDD 49 - SDA 4A - SCL 4B
#define PDOC_ADC_ADDR								0x4B
#define MC_HV_SENSE_ADC_ADDR				0x49
#define BATT_HV_SENSE_ADC_ADDR			0x48

// Interval & Periods
#define CAN_BROADCAST_INTERVAL      0.5
#define PRECHARGE_TIMEOUT           5

//-----------------------------------------------
// Precharge States
//-----------------------------------------------

typedef enum PREcHARGE_STATES {
	PRECHARGE_STATE_FAIL,
	PRECHARGE_STATE_IDLE,
	PRECHARGE_STATE_PRECHARGING,
	PRECHARGE_STATE_PRECHARGING_TIMER,
	PRECHARGE_STATE_PRECHARGED,
	PRECHARGE_STATE_DRIVE,
} precharge_states_t;

//-----------------------------------------------
// TS_STD_CAN_INTERPRETATIONS
//-----------------------------------------------

typedef enum CAN_ERROR_WARNING_SIGNALS{
	CAN_ERROR_1,
	CAN_ERROR_2,
	CAN_WARNING_1,
	CAN_WARNING_2,
	CAN_AMS_OK,
	CAN_PDOC_OK,
	CAN_IMD_OK,
	CAN_ERROR_SPARE,
} can_error_warning_flag_t;

typedef enum CAN_ANALOGUE_1_SIGNALS{
	CAN_ANALOGUE_1_PDOC_TEMPERATURE_1,
	CAN_ANALOGUE_1_PDOC_TEMPERATURE_2,
	CAN_ANALOGUE_1_PDOC_REF_TEMPERATURE_1,
	CAN_ANALOGUE_1_PDOC_REF_TEMPERATURE_2,
	CAN_ANALOGUE_1_HV_MC_SENSE_VOLTAGE_1,
	CAN_ANALOGUE_1_HV_MC_SENSE_VOLTAGE_2,
	CAN_ANALOGUE_1_HV_BATTERY_SENSE_VOLTAGE_1,
	CAN_ANALOGUE_1_HV_BATTERY_SENSE_VOLTAGE_2,
} can_analogue_1_signals_t;

typedef enum CAN_ANALOGUE_2_SIGNALS{
	CAN_ANALOGUE_2_IMD_PERIOD,
	CAN_ANALOGUE_2_IMD_FREQUENCY,
	CAN_ANALOGUE_2_IMD_DUTY_CYCLE,
	CAN_ANALOGUE_2_SPARE_3,
	CAN_ANALOGUE_2_SPARE_4,
	CAN_ANALOGUE_2_SPARE_5,
	CAN_ANALOGUE_2_SPARE_6,
	CAN_ANALOGUE_2_SPARE_7,
} can_analogue_2_signals_t;

typedef enum CAN_DIGITAL_1_SIGNALS{
	CAN_DIGITAL_1_AIR_POWER,
	CAN_DIGITAL_1_AIR_NEG_RELAY,
	CAN_DIGITAL_1_AIR_NEG_FEEDBACK,
	CAN_DIGITAL_1_AIR_POS_RELAY,
	CAN_DIGITAL_1_AIR_POS_FEEDBACK,
	CAN_DIGITAL_1_PRECHARGE_RELAY,
	CAN_DIGITAL_1_SPARE_6,
	CAN_DIGITAL_1_SPARE_7,
} can_digital_1_signals_t;
Orion af(PC_13);
//-----------------------------------------------
// Error/Warning Flags
//-----------------------------------------------

typedef enum ERROR_CODES_SUB_KEY {
  ERROR_AMS_FAIL,
  ERROR_PDOC_FAIL,
  ERROR_IMD_FAIL,
  ERROR_ORION_TIMEOUT,
  ERROR_ORION_LOW_VOTLAGE,
  ERROR_ORION_HIGH_VOLTAGE,
  ERROR_ORION_OVERTEMPERATURE,
  ERROR_PERIPHERALS,
} error_state_t;

typedef enum WARNING_CODES_SUB_KEY {
  WARNING_PCB_OVERTEMPERATURE,
  WARNING_DISCHARGE_PRECHARGE_MISMATCH,
  WARNING_AIR_NEG_FEEDBACK_MISMATCH,
  WARNING_AIR_POS_FEEDBACK_MISMATCH,
  WARNING_PDOC_SENSOR_FAILURE,
  WARNING_MC_ADC_SENSOR_FAILURE,
  WARNING_BATT_ADC_SENSOR_FAILURE,
  WARNING_PDOC_RELAY_FAILURE,
} warning_state_t;

//-----------------------------------------------
// Communications Interfaces
//-----------------------------------------------

// UART Interface
Serial pc(PIN_SERIAL_TX, PIN_SERIAL_RX);                 	//TX, RX

// I2C Interface
I2C i2c1(PIN_I2C_SDA, PIN_I2C_SCL);     					//SDA, SCL

HV_ADC hv_mc_sense(i2c1, MC_HV_SENSE_ADC_ADDR, MC_R_CAL);
HV_ADC hv_battery_sense(i2c1, BATT_HV_SENSE_ADC_ADDR, BATT_R_CAL);

int main(){
    pc.printf("Starting ts_20 Precharge Controller (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s\r\n",__TIMESTAMP__);

	pc.printf("Faults cleared, startup completed!\r\n");

	// Program loop. Error checking handled within state deamon.
	while(1) {

		hv_battery_sense.update_adc();
		hv_mc_sense.update_adc();
		pc.printf("Battery voltage: %d\n", hv_battery_sense.get_voltage());
        pc.printf("MC voltage: %d\n\n", hv_mc_sense.get_voltage());
        wait_us(1000000);
    }

	pc.printf("Is this a BSOD?");
	return 0;
}
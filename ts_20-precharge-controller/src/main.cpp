// TEAM SWINBURNE - TS_20
// PRECHARGE CONTROLLER
// PATRICK CURTAIN
// REVISION 1 (22/06/2021)

/* 
To correct limited 64k flash issue: https://github.com/platformio/platform-ststm32/issues/195. Ensure device is 128k model. 
Use the following platformIO initialisation:
	[env:genericSTM32F103C8]
	platform = ststm32@4.6.0
	board = genericSTM32F103C8
	framework = mbed
	board_upload.maximum_size = 120000

//-----------------------------------------------

The precharge controller is a PCB designed by Team Swinburne to control the precharge sequences of 
a Formula SAE vehicle, monitor the health of the accumulator based on various inputs and react accordingly. This PCB 
integrates the precharge resistor array, precharge relay, over-temperature safety control (PDOc), safety interlock for the AMS, 
an interface for the isolation monitoring device (IMD), and the ability to detect the voltage of the battery and motor
controller.

//-----------------------------------------------

Based on STM32F103C8 "Blue Pill".

PA_0/ADC0/CTS2/T2C1E/WKUP (PWM) (3.3V)
PA_1/ADC/RT52/T2C2 (PWM) (3.3V)
PA_2/ADC2/TX2/T2C3 (PWM) (3.3V)
PA_3/ADC3/RX2/T2C4 (PWM) (3.3V)
PA_4/ADC4/NSS1/CK2 (3.3V)
PA_5/ADC5/SCK1 (3.3V)
PA_6/ADC6/MISO1/T3C1/T1BKIN (PWM) (3.3V)
PA_7/ADC7/MOSI1/T3C2/T1C1N (PWM) (3.3V)

PA_8/CK1/T1C2/MCO (PWM)
PA_9/TX1/T1C2 (PWM)
PA_10/RX1/T1C3 (PWM)
PA_11/USB-/CTS1/T1C4/CANRX
PA_12/USB+/RTS1/T1ETR/CANTX
PA_13/JTMS/SWDIO
PA_14/JTMS/SWCLK
PA_15/JTD1/NSS/T2C1E

PB_0/ADC8/T3C3/T1C2N (PWM) (3.3V)
PB_1/ADC9/T3C4/T1C3N (PWM) (3.3V)
PB_2/BOOT1
PB_3/JTD0/SCK1/T2C2
PB_4/JTRST/MISO/T3C1
PB_5/SMBAI1/MOSI1/T2C2 (3.3V)

PB_8/T4C3/SCL1/CANRX (PWM)
PB_9/T4C4/SDA1/CANTX
PB_10/SCL2/TX3/T2C3N
PB_11/SDA2/RX3/T2C3N
PB_12/NSS2/T1BKIN/CK3
PB_13/SCK2/T1C1N/CTS3
PB_14/MISO2/T1C2N/RTS3
PB_15/MOSI2/T1C3N

PC_13/TAMPER/LED1 (3.3V)*
PC_14/OSC32IN (3.3V)*
PC_15/OSC32OUT (3.3V)*

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

// CANBUS Interface
CAN can1(PIN_CAN1_RXD, PIN_CAN1_TXD);     					// RXD, TXD

// CANBUS Message Format
CANMessage can1_msg;

//-----------------------------------------------
// Interfaces
//-----------------------------------------------

Heart heart(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS, PIN_HEART_LED1, PIN_PCB_TEMP);

Orion orion(PIN_AMS_OK);
PDOC pdoc(i2c1, PDOC_ADC_ADDR, PIN_PDOC_OK);
IMD imd(PIN_IMD_OK, PIN_IMD_DATA);

HV_ADC hv_mc_sense(i2c1, MC_HV_SENSE_ADC_ADDR, MC_R_CAL);
HV_ADC hv_battery_sense(i2c1, BATT_HV_SENSE_ADC_ADDR, BATT_R_CAL);

Discharge_Module discharge_module;

InterruptIn air_power(PIN_AIR_POWER);

Relay precharge_relay(PIN_PRECHARGE_RELAY);
AIR AIR_neg_relay(PIN_AIR_NEG_RELAY, PIN_AIR_NEG_RELAY_FB);
AIR AIR_pos_relay(PIN_AIR_POS_RELAY, PIN_AIR_POS_RELAY_FB);

Watchdogs UCM4_Inverter;
Watchdogs UCM5_Accumulator;
Watchdogs Motor_Controller;

DigitalOut can1_rx_led(PIN_CAN1_RX_LED);
DigitalOut can1_tx_led(PIN_CAN1_TX_LED);

//-----------------------------------------------
// Real Time Operations
//-----------------------------------------------

// Programmable Interrupt Timer Instances
Ticker ticker_heartbeat;
Ticker ticker_can_transmit1;
Ticker ticker_can_transmit2;
Timeout timeout_precharge;

//-----------------------------------------------
// Functions
//-----------------------------------------------

uint8_t array_to_uint8(bool arr[], int count){
    int ret = 0;
    int tmp;
    for (int i = 0; i < count; i++) {
        tmp = arr[i];
        ret |= tmp << (count - i - 1);
    }
    return ret;
}

void relay_state_safe(){
    AIR_neg_relay.disable_relay();
    precharge_relay.disable_relay();
    AIR_pos_relay.disable_relay();
}

void relay_state_precharging(){
    AIR_neg_relay.activate_relay();
    precharge_relay.activate_relay();
    AIR_pos_relay.disable_relay();
}

void relay_state_precharged_transition(){
    AIR_neg_relay.activate_relay();
    precharge_relay.activate_relay();
    AIR_pos_relay.activate_relay();
}

void relay_state_precharged(){
    AIR_neg_relay.activate_relay();
    precharge_relay.disable_relay();
    AIR_pos_relay.activate_relay();
}
	
	/** check_precharged()
	 * 
	 * checks if the precharge voltages are within 95% of each other.
	 * This should be included in the state machine to bypass the timeout, and advance the state.
     * 
	 * @returns precharge_successful flag if true.
	 * 
     */
bool check_precharged(){
	int hv_battery_voltage = hv_battery_sense.get_voltage();
	int hv_mc_voltage = hv_mc_sense.get_voltage();

    if (hv_mc_voltage > hv_battery_voltage*0.95){
		// Don't need additional checks, should be moved back into the object. 
		// Realisitically only needs to check one side is working.
		if (hv_mc_voltage > MINIMUM_PRECHARGE_VOLTAGE && hv_battery_voltage > MINIMUM_PRECHARGE_VOLTAGE){
			if (hv_mc_voltage < MAXIMUM_PRECHARGE_VOLTAGE && hv_battery_voltage < MAXIMUM_PRECHARGE_VOLTAGE){
				relay_state_precharged_transition(); 
				pc.printf("Precharge within 95%, safe to close postive contactor\r\n");
				return true;
			}
		}
    } 
	return false;
}

//-----------------------------------------------
// Handlers
//-----------------------------------------------

// Flash the LED!
bool can_transmission_h(CANMessage _can_message){
    if(can1.write(_can_message)) {
		can1_tx_led = !can1_tx_led;
        return true;
    } else {
        return false;
	}
}

//-----------------------------------------------
// Callback Functions
//-----------------------------------------------

	/*
Heartbeat Callback
	Called every second and transmits the heartbeat message on the CANBUS. Will flash LED1 if 
	successful.
	*/
void heartbeat_cb(){
    if (can_transmission_h(heart.heartbeat())){
        pc.printf("Heartbeat Success! State: %d Counter: %d\r\n", heart.get_heartbeat_state(), heart.get_heartbeat_counter());
    } else {
        pc.printf("Hearts dead :(\r\n");
    }
}

	/*
AIR Power Lost Callback
	If the power to the AIR power supply (Passed from Green Loop), then the 
	vehicle's state should reset into fault, which should clear as soon as possible.
	Set the AMS_ok to zero just to make sure nothing strange is happening.
	*/
void air_power_lost_cb(){
    heart.set_heartbeat_state(PRECHARGE_STATE_IDLE);
	pc.printf("AIR Power Lost!\r\n");
}
	
	/*
Precharge Timeout Callback
	Called after 5 seconds of the precharge. Performs quick check to make sure the 
	precharge hasn't completed yet. This callback is set within the start_precharge_sequence_cb()
	call, which is itself called from the receive callback if the charger or precharge button is pressed.
	...
	timeout_precharge.attach(&precharge_timeout_cb, PRECHARGE_TIMEOUT);
	*/
void precharge_timeout_cb(){
	if (heart.get_heartbeat_state() != PRECHARGE_STATE_PRECHARGED){
		if (!check_precharged()){
			heart.set_heartbeat_state(PRECHARGE_STATE_IDLE);
			pc.printf("Precharge timed out!\r\n");
		}
	}
}

	/*
Start Precharge Sequence Callback
	Called by the CAN receive callback, attaches precharge_timeout_cb() function to 
	check if the precharge has been successful after 5 seconds
	otherwise, the precharge will reset to the idle state.

	Will reject precharge request if precharge voltage below or above defined minimum and maximum values.
	*/
void start_precharge_sequence_cb(){
	if (air_power.read() == true){
		//if (hv_battery_sense.get_voltage() > MINIMUM_PRECHARGE_VOLTAGE && hv_battery_sense.get_voltage() < MAXIMUM_PRECHARGE_VOLTAGE){
			relay_state_precharging();
			heart.set_heartbeat_state(PRECHARGE_STATE_PRECHARGING);
			timeout_precharge.attach(&precharge_timeout_cb, PRECHARGE_TIMEOUT);
			pc.printf("Starting precharge!\r\n");
		//}
	}
}

	/*
CAN Transmit 1
	Sent CAN messages relevant to the device. These should follow the Team Swinburne
	Standard format for ease of debugging. They are split in 2 to ensure callback executing time
	is not too long for the interupt.
	*/
void can1_trans_cb1(){
	// can1_tx_led = !can1_tx_led;
	char TX_data[8] = {0};
	int dlc = 7;
	TX_data[CAN_ERROR_1] 		= heart.get_error_code(0);
	TX_data[CAN_ERROR_2] 		= heart.get_error_code(1);
	TX_data[CAN_WARNING_1] 		= heart.get_warning_code(0);
	TX_data[CAN_WARNING_2] 		= heart.get_warning_code(1);
	TX_data[CAN_AMS_OK] 		= orion.get_AMS_ok();
	TX_data[CAN_PDOC_OK] 		= pdoc.get_pdoc_ok();
	TX_data[CAN_IMD_OK] 		= imd.get_IMD_ok();
	//TX_data[CAN_ERROR_SPARE] 	= 0;
	can_transmission_h(CANMessage(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + TS_ERROR_WARNING_ID, &TX_data[0], dlc));
	
	dlc = 6;
	TX_data[CAN_DIGITAL_1_AIR_POWER] 			= air_power.read();
	TX_data[CAN_DIGITAL_1_AIR_NEG_RELAY] 		= AIR_neg_relay.get_relay();
	TX_data[CAN_DIGITAL_1_AIR_NEG_FEEDBACK] 	= AIR_neg_relay.get_feedback();
	TX_data[CAN_DIGITAL_1_AIR_POS_RELAY] 		= AIR_pos_relay.get_relay();
	TX_data[CAN_DIGITAL_1_AIR_POS_FEEDBACK] 	= AIR_pos_relay.get_feedback();
	TX_data[CAN_DIGITAL_1_PRECHARGE_RELAY] 		= precharge_relay.get_relay();
	//TX_data[CAN_DIGITAL_1_SPARE_6] 				= 0;
	//TX_data[CAN_DIGITAL_1_SPARE_7] 				= 0;
	can_transmission_h(CANMessage(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + TS_DIGITAL_1_ID, &TX_data[0], dlc));

	dlc = 8;
	TX_data[CAN_ANALOGUE_1_PDOC_TEMPERATURE_1] 			= (char)(pdoc.get_pdoc_temperature() >> 8);
	TX_data[CAN_ANALOGUE_1_PDOC_TEMPERATURE_2] 			= (char)(pdoc.get_pdoc_temperature() & 0xFF);
	TX_data[CAN_ANALOGUE_1_PDOC_REF_TEMPERATURE_1] 		= (char)(pdoc.get_pdoc_ref_temperature() >> 8);
	TX_data[CAN_ANALOGUE_1_PDOC_REF_TEMPERATURE_2] 		= (char)(pdoc.get_pdoc_ref_temperature() & 0xFF);
	TX_data[CAN_ANALOGUE_1_HV_MC_SENSE_VOLTAGE_1] 		= (char)(hv_mc_sense.get_voltage()*10 >> 8);
	TX_data[CAN_ANALOGUE_1_HV_MC_SENSE_VOLTAGE_2] 		= (char)(hv_mc_sense.get_voltage()*10 & 0xFF);
	TX_data[CAN_ANALOGUE_1_HV_BATTERY_SENSE_VOLTAGE_1]	= (char)(hv_battery_sense.get_voltage()*10 >> 8);
	TX_data[CAN_ANALOGUE_1_HV_BATTERY_SENSE_VOLTAGE_2]	= (char)(hv_battery_sense.get_voltage()*10 & 0xFF);
	can_transmission_h(CANMessage(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + TS_ANALOGUE_1_ID, &TX_data[0], dlc));
}

	/*
CAN Transmit 2
	Sent CAN messages relevant to the device. These should follow the Team Swinburne
	Standard format for ease of debugging. They are split in 2 to ensure callback executing time
	is not top long for the interupt.
	*/
void can1_trans_cb2(){
	char TX_data[8] = {0};
	int dlc = 3;

	TX_data[CAN_ANALOGUE_2_IMD_PERIOD] 		= (char)(imd.get_period());
	TX_data[CAN_ANALOGUE_2_IMD_FREQUENCY] 	= imd.get_frequency();
	TX_data[CAN_ANALOGUE_2_IMD_DUTY_CYCLE] 	= imd.get_duty_cycle();
	TX_data[CAN_ANALOGUE_2_SPARE_3] 		= 0;
	TX_data[CAN_ANALOGUE_2_SPARE_4] 		= 0;
	TX_data[CAN_ANALOGUE_2_SPARE_5] 		= 0;
	TX_data[CAN_ANALOGUE_2_SPARE_6] 		= 0;
	TX_data[CAN_ANALOGUE_2_SPARE_7] 		= 0;
	can_transmission_h(CANMessage(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + TS_ANALOGUE_2_ID, &TX_data[0], dlc));

	can_transmission_h(CANMessage(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + TS_ANALOGUE_1_ID, &TX_data[0], dlc));

	can_transmission_h(CANMessage(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + TS_ANALOGUE_3_ID, &TX_data[0], dlc));
}
	/*
CAN Receive
	Message box for CANBUS. Switch handles which message is being serviced.
	*/
void can1_recv_cb(){
	can1_rx_led = !can1_rx_led;

	if (can1.read(can1_msg)){
		switch(can1_msg.id){
			// Secret method to bypass voltage checks. UNSAFE! Use in case of emergency!
			case (CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + 0x0F):
				heart.set_heartbeat_state(PRECHARGE_STATE_PRECHARGING_TIMER);
				break;

			// Set discharge state for checking mismatch.
			case (CAN_DISCHARGE_MODULE_BASE_ADDRESS):
				discharge_module.set_precharge_discharge_state(heart.get_heartbeat_state(), (int)can1_msg.data[0]);
				break;
			
			// Use precharge button to begin precharge sequence.
			case (CAN_MOTEC_THROTTLE_CONTROLLER_BASE_ADDRESS + TS_DIGITAL_1_ID):
				// check if precharge button is pressed
				if (can1_msg.data[0] == 1)
					if (heart.get_heartbeat_state() == PRECHARGE_STATE_IDLE){
                    // pc.printf("Precharge button pressed, starting precharge routine\r\n");
					start_precharge_sequence_cb();
                }

				if (can1_msg.data[1] == 1)
					if (heart.get_heartbeat_state() == PRECHARGE_STATE_PRECHARGED){
                    // pc.printf("Precharge button pressed, starting precharge routine\r\n");
					heart.set_heartbeat_state(PRECHARGE_STATE_DRIVE);
                }

				break;
      case (CAN_UCM4_BASE_ADDRESS+TS_ERROR_WARNING_ID):
				UCM4_Inverter.connect(can1_msg.data[1]);

			case (CAN_UCM5_BASE_ADDRESS+TS_ERROR_WARNING_ID):
				UCM5_Accumulator.connect(can1_msg.data[1]);

			// Use charger presense to begin precharge sequence.
            case (CAN_TC_CHARGER_STATUS_ID):
                if (heart.get_heartbeat_state() == 1){
                    // pc.printf("Charger detected, starting precharge routine\r\n");
                    start_precharge_sequence_cb();
                }
				break;

			// Set Orion BMS state and check safe to use.
			case (CAN_ORION_BMS_BASE_ADDRESS + TS_HEARTBEAT_ID):
				// Big endian & MSB
				// 0b000000, 1000000	Discharge Relay Enabled
				// 0b000000, 0100000	Charge Relay Enabled
				// 0b000000, 0010000	Charge Safety Enabled
				// Use bitwise operator to mask out all except relevent statuses.

				// 1. Connect Orion
				// 2. Check Relay Status disable ams ok if this fails. 
				orion.connect_orion(can1_msg.data[0] & 0b11100000);
				break;
			
			// Set Orion BMS voltages and check safe to use.
			case (CAN_ORION_BMS_BASE_ADDRESS + TS_ANALOGUE_1_ID):
				orion.set_low_voltage(can1_msg.data[0]*50);
				orion.set_high_voltage(can1_msg.data[1]*50);
				break;

			// Set orion temperatures and check safe to use.
			case (CAN_ORION_BMS_BASE_ADDRESS + TS_ANALOGUE_2_ID):
				orion.set_high_temperature(can1_msg.data[1]);
				break;			
		}
	}
}

	/*
Check Errors
	Checks for critical errors. Resultant value should be assigned to the Heart object; the heart
	will send the state of the device into fail as soon as possible, and disable the vehicle.
	*/
uint8_t check_errors(){
	bool error_code[8];

	error_code[ERROR_AMS_FAIL] 					= !orion.get_AMS_ok();
	error_code[ERROR_PDOC_FAIL] 				= !pdoc.get_pdoc_ok();
	error_code[ERROR_IMD_FAIL] 					= !imd.get_IMD_ok();
	error_code[ERROR_ORION_TIMEOUT] 		= !orion.get_orion_connection_ok();
	
	error_code[ERROR_ORION_LOW_VOTLAGE] 	= orion.check_low_voltage();
	error_code[ERROR_ORION_HIGH_VOLTAGE] 	= orion.check_high_voltage();
	error_code[ERROR_ORION_OVERTEMPERATURE] = orion.check_overtemperature();
	error_code[ERROR_PERIPHERALS] = 0;
	//!UCM4_Inverter.get_device_ok() && !UCM5_Accumulator.get_device_ok();;

	return array_to_uint8(error_code, 8);
}

	/*
Check Warnings
	Checks for non-critical errors. Resultant value should be assigned to the Heart object.
	*/
uint8_t check_warnings(){
	bool warning_code[8];

	warning_code[WARNING_PCB_OVERTEMPERATURE] 			= !heart.pcb_temperature.pcb_temperature_ok();
	warning_code[WARNING_DISCHARGE_PRECHARGE_MISMATCH] 	= !discharge_module.precharge_discharge_match();
	warning_code[WARNING_AIR_NEG_FEEDBACK_MISMATCH] 	= !AIR_neg_relay.relay_ok();
	warning_code[WARNING_AIR_POS_FEEDBACK_MISMATCH] 	= !AIR_pos_relay.relay_ok();

	warning_code[WARNING_PDOC_SENSOR_FAILURE] 			= !pdoc.get_sensor_ok();
	warning_code[WARNING_MC_ADC_SENSOR_FAILURE] 		= !hv_mc_sense.get_sensor_ok();
	warning_code[WARNING_BATT_ADC_SENSOR_FAILURE]		= !hv_battery_sense.get_sensor_ok();
	warning_code[WARNING_PDOC_RELAY_FAILURE] 			= !pdoc.check_pdoc_relay_fail();
	
	return array_to_uint8(warning_code, 8);
}

	/*
Update Precharge
	Updates the device as required, and checks for errors.
	*/
void update_precharge(){
	// Update analogue measurements to get latest information.
	pdoc.update_adc();
	hv_battery_sense.update_adc();
	hv_mc_sense.update_adc();

	// Perform basic error checking. Sets state to FAIL if error found.
	heart.set_error_code(check_errors(), 0);
	heart.set_warning_code(check_warnings(), 0);
}

	/*
State Deamon
	Deamon responsible for managing the state of the device.
	NB. Many of the functions are handled by interrupt routines, 
	care should be taken to ensure that this simply manages errors and the 
	relays. 
	*/
void state_d(){
	update_precharge();
	if (heart.get_error_code(0) > 0){
		heart.set_heartbeat_state(PRECHARGE_STATE_FAIL);
	}

	switch (heart.get_heartbeat_state()){
		case PRECHARGE_STATE_FAIL:
			// Do nothing. 
			relay_state_safe();

			// In order for the device to latch into a fail, this must be disabled.
			// and an infinite loop included to force the latch. 
			// For testing, this is unnessesary. 
			while(1){
				heart.set_heartbeat_state(PRECHARGE_STATE_FAIL);
			}

			// ENSURE THIS IS REMOVED!
			// if (check_errors() == 0){
			// 	heart.set_heartbeat_state(PRECHARGE_STATE_IDLE);
			// }
			break;

		case PRECHARGE_STATE_IDLE:
			// The idle state is advanced by callbacks from either the throttle's precharge button
			// or the charger becoming visible.
			relay_state_safe();
			break;

		case PRECHARGE_STATE_PRECHARGING:
			// After the precharge is advanced, the device has 5 seconds before timing out. 
			// A timeout event will cause the device to reset to the failed state.
			if (air_power.read() == false){
				heart.set_heartbeat_state(PRECHARGE_STATE_IDLE);
			}
			
			relay_state_precharging();
            //if (check_precharged()){
				wait(3);
				heart.set_heartbeat_state(PRECHARGE_STATE_PRECHARGED);
			//}
			break;

		case PRECHARGE_STATE_PRECHARGING_TIMER:
			// Brute force state used to bypass the voltage checks and force a precharge sequence.
			// Not to be used unless ABSOLUTELY necessary.
			if (air_power.read() == false){
				heart.set_heartbeat_state(PRECHARGE_STATE_IDLE);
			}

			if (check_errors() == 0){
				relay_state_precharging();
				wait(3);
				heart.set_heartbeat_state(PRECHARGE_STATE_PRECHARGED);
			}
			break;
			
		case PRECHARGE_STATE_PRECHARGED:
			// Precharged state, waiting for drive button or charging state.This state is deactivated by breaking the 
			// green loop. The intended way being the cockpit E-Stop, or by power cycling the vehicle...
			// Otherwise something's gone wrong.
			if (air_power.read() == false){
				heart.set_heartbeat_state(PRECHARGE_STATE_IDLE);
			}
			
			relay_state_precharged();
			break;
		case PRECHARGE_STATE_DRIVE:
			// The drive of the vehicle. This state is deactivated by breaking the 
			// green loop. The intended way being the cockpit E-Stop, or by power cycling the vehicle...
			// Otherwise something's gone wrong.
			relay_state_precharged();
			break;
	}
}
//-----------------------------------------------
// Initialisations
//-----------------------------------------------

	/*
SETUP
	Initialisation of CANBUS, IMD, and HEARTBEAT. Mostly interrupt routines, callback
	documented above.
	*/
void setup(){
	// Disable interrupts for smooth startup routine.
	wait(1);
	
	__disable_irq();

	ticker_heartbeat.attach(&heartbeat_cb, HEARTRATE);
	heart.set_heartbeat_state(PRECHARGE_STATE_FAIL);

	can1.frequency(CANBUS_FREQUENCY);
	can1.attach(&can1_recv_cb);
	ticker_can_transmit1.attach(&can1_trans_cb1, CAN_BROADCAST_INTERVAL);

	//wait(5);

	//ticker_can_transmit2.attach(&can1_trans_cb2, CAN_BROADCAST_INTERVAL);
	
  air_power.fall(&air_power_lost_cb);

	imd.start();
	// Re-enable interrupts again, now that interrupts are ready.
	__enable_irq();

	// Allow some time to settle!
	wait(1);

	// Assume device in failure mode, hold until all start up faults then 
	// force into idle mode. Mandated by EV.8.2.3.
	do {
		update_precharge();
	} while (heart.get_error_code(0) > 0);
	heart.set_heartbeat_state(PRECHARGE_STATE_IDLE);
}

int main(){
    pc.printf("Starting ts_20 Precharge Controller (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s\r\n",__TIMESTAMP__);

	setup();

	pc.printf("Faults cleared, startup completed!\r\n");

	// Program loop. Error checking handled within state deamon.
	while(1)
		state_d();

	pc.printf("Is this a BSOD?");
	return 0;
}
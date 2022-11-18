// TEAM SWINBURNE - TS_22
// PRECHARGE CONTROLLER
// NATALIE NG
// Test File: test_AIR.cpp (22/06/2022)

/*
BASIC FUNCTIONALITY TEST
Checks that AIRs can be activated/deactivated by the precharge
*/

// #include <mbed.h>
// #include <CAN.h>
// #include "precharge_pinout.h"

// #include "can_addresses.h"
// #include "ts_std_device.h"

// #include "hv_tools.h"
// #include "relays.h"
// #include "precharge_discharge.h"

// #include "imd.h"
// #include "orion.h"
// #include "watchdogs.h"
// // #include "USBSerial.h"

// // UART Interface
// Serial pc(PIN_SERIAL_TX, PIN_SERIAL_RX);                 	//TX, RX
// // USBSerial serial;
// AIR AIR_neg_relay(PIN_AIR_NEG_RELAY, PIN_AIR_NEG_RELAY_FB);
// AIR AIR_pos_relay(PIN_AIR_POS_RELAY, PIN_AIR_POS_RELAY_FB);

// int main() {

// 	DigitalIn feedbackAIRMinus(PB_10);
// 	DigitalIn feedbackAIRPlus(PB_11);
// 	DigitalOut feedbackAIRMinusLed(PA_6);
// 	DigitalOut feedbackAIRPlusLed(PA_7);
// 	DigitalOut AIRMinusSignal(PA_8);
// 	DigitalOut AIRPlusSignal(PA_10);

// 	while(1) {

// 		AIRMinusSignal = 1;
// 		AIRPlusSignal = 1;
// 		feedbackAIRMinusLed = feedbackAIRMinus.read();
// 		feedbackAIRPlusLed = feedbackAIRPlus.read();
// 		pc.printf("The AIR- feedback signal is: %d\n", AIR_neg_relay.get_feedback());
// 		pc.printf("The AIR+ feedback signal is: %d\n\n", AIR_pos_relay.get_feedback());
// 		wait_us(5000000);

// 		AIRMinusSignal = 0;
// 		AIRPlusSignal = 0;
// 		feedbackAIRMinusLed = feedbackAIRMinus.read();
// 		feedbackAIRPlusLed = feedbackAIRPlus.read();
// 		pc.printf("The AIR- feedback signal is: %d\n", AIR_neg_relay.get_feedback());
// 		pc.printf("The AIR+ feedback signal is: %d\n\n", AIR_pos_relay.get_feedback());
// 		wait_us(5000000);
// 	}

// 	return 0;
// }

/*
TEST WITH INTEGRATED PRECHARGE CODE
Final test with integrated TS_20 Precharge code
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
CAN Transmit 1
	Sent CAN messages relevant to the device. These should follow the Team Swinburne
	Standard format for ease of debugging. They are split in 2 to ensure callback executing time
	is not too long for the interupt.
	*/
void can1_trans_cb1(){
	// can1_tx_led = !can1_tx_led;
	char TX_data[8] = {0};
	int dlc = 6;
	TX_data[CAN_DIGITAL_1_AIR_POWER] 			= air_power.read();
	TX_data[CAN_DIGITAL_1_AIR_NEG_RELAY] 		= AIR_neg_relay.get_relay();
	TX_data[CAN_DIGITAL_1_AIR_NEG_FEEDBACK] 	= AIR_neg_relay.get_feedback();
	TX_data[CAN_DIGITAL_1_AIR_POS_RELAY] 		= AIR_pos_relay.get_relay();
	TX_data[CAN_DIGITAL_1_AIR_POS_FEEDBACK] 	= AIR_pos_relay.get_feedback();
	TX_data[CAN_DIGITAL_1_PRECHARGE_RELAY] 		= precharge_relay.get_relay();
	//TX_data[CAN_DIGITAL_1_SPARE_6] 				= 0;
	//TX_data[CAN_DIGITAL_1_SPARE_7] 				= 0;
	can_transmission_h(CANMessage(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + TS_DIGITAL_1_ID, &TX_data[0], dlc));
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
	wait_us(1000000);
	
	__disable_irq();

	// ticker_heartbeat.attach(&heartbeat_cb, HEARTRATE);
	heart.set_heartbeat_state(PRECHARGE_STATE_FAIL);

	// can1.frequency(CANBUS_FREQUENCY);
	// ticker_can_transmit1.attach(&can1_trans_cb1, CAN_BROADCAST_INTERVAL);

	//wait(5);
	
  air_power.fall(&air_power_lost_cb);

	imd.start();
	// Re-enable interrupts again, now that interrupts are ready.
	__enable_irq();

	// Allow some time to settle!
	wait_us(1000000);

	// Assume device in failure mode, hold until all start up faults then 
	// force into idle mode. Mandated by EV.8.2.3.
	do {
		update_precharge();
	} while (heart.get_error_code(0) > 0);
	heart.set_heartbeat_state(PRECHARGE_STATE_IDLE);
}

	DigitalOut feedbackAIRMinusLed(PA_6);
	DigitalOut feedbackAIRPlusLed(PA_7);

int main(){
    pc.printf("Starting ts_20 Precharge Controller (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s\r\n",__TIMESTAMP__);

	setup();

	pc.printf("Faults cleared, startup completed!\r\n");

	// Program loop. Error checking handled within state deamon.
	while(1) {

		AIR_neg_relay.activate_relay();
		AIR_pos_relay.disable_relay();
		wait_us(1000000);
		/* Displays the feedback relay status on LEDs A6 and A7*/
		feedbackAIRMinusLed = AIR_neg_relay.get_feedback();
		feedbackAIRPlusLed = AIR_pos_relay.get_feedback();

		/*Uncomment to check if AIR signal and AIR feedback are consistent*/
		// if (AIR_neg_relay.relay_ok()) {

		// 	pc.printf("Negative Relay is okay\n");
		// }
		// else {

		// 	pc.printf("Negative Relay is not okay\n");
		// }
		// if (AIR_pos_relay.relay_ok()) {

		// 	pc.printf("Positive Relay is okay\n");
		// }
		// else {

		// 	pc.printf("Positive Relay is not okay\n");
		// }
		
		AIR_neg_relay.activate_relay();
		AIR_pos_relay.disable_relay();
		wait_us(1000000);
		/* Displays the feedback relay status on LEDs A6 and A7*/
		feedbackAIRMinusLed = AIR_neg_relay.get_feedback();
		feedbackAIRPlusLed = AIR_pos_relay.get_feedback();

		/*Uncomment to check if AIR signal and AIR feedback are consistent*/
		// if (AIR_neg_relay.relay_ok()) {

		// 	pc.printf("Negative Relay is okay\n");
		// }
		// else {

		// 	pc.printf("Negative Relay is not okay\n");
		// }
		// if (AIR_pos_relay.relay_ok()) {

		// 	pc.printf("Positive Relay is okay\n");
		// }
		// else {

		// 	pc.printf("Positive Relay is not okay\n");
		// }
		wait_us(4000000);
	}

	pc.printf("Is this a BSOD?");
	return 0;
}
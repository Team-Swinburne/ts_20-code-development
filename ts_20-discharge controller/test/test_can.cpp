// TEAM SWINBURNE - TS_22
// PRECHARGE CONTROLLER
// NATALIE NG
// Test File: test_can.cpp (22/06/2022)

/*
Checks all required information can be broadcasted on CAN 
Note: this test file does not actually broadcast any information on CAN
To test broadcasting actual information on CAN use the relevant test file
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

// typedef enum PREcHARGE_STATES {
// 	PRECHARGE_STATE_FAIL,
// 	PRECHARGE_STATE_IDLE,
// 	PRECHARGE_STATE_PRECHARGING,
// 	PRECHARGE_STATE_PRECHARGING_TIMER,
// 	PRECHARGE_STATE_PRECHARGED,
// 	PRECHARGE_STATE_DRIVE,
// } precharge_states_t;

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

// typedef enum ERROR_CODES_SUB_KEY {
//   ERROR_AMS_FAIL,
//   ERROR_PDOC_FAIL,
//   ERROR_IMD_FAIL,
//   ERROR_ORION_TIMEOUT,
//   ERROR_ORION_LOW_VOTLAGE,
//   ERROR_ORION_HIGH_VOLTAGE,
//   ERROR_ORION_OVERTEMPERATURE,
//   ERROR_PERIPHERALS,
// } error_state_t;

// typedef enum WARNING_CODES_SUB_KEY {
//   WARNING_PCB_OVERTEMPERATURE,
//   WARNING_DISCHARGE_PRECHARGE_MISMATCH,
//   WARNING_AIR_NEG_FEEDBACK_MISMATCH,
//   WARNING_AIR_POS_FEEDBACK_MISMATCH,
//   WARNING_PDOC_SENSOR_FAILURE,
//   WARNING_MC_ADC_SENSOR_FAILURE,
//   WARNING_BATT_ADC_SENSOR_FAILURE,
//   WARNING_PDOC_RELAY_FAILURE,
// } warning_state_t;

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
CAN Transmit 1
	Sent CAN messages relevant to the device. These should follow the Team Swinburne
	Standard format for ease of debugging. They are split in 2 to ensure callback executing time
	is not too long for the interupt.
	*/
void can1_trans_cb1(){
	can1_tx_led = !can1_tx_led;
	char TX_data[8] = {0};
	int dlc = 7;
	TX_data[CAN_ERROR_1] 		= heart.get_error_code(0);
	TX_data[CAN_ERROR_2] 		= heart.get_error_code(1);
	TX_data[CAN_WARNING_1] 		= heart.get_warning_code(0);
	TX_data[CAN_WARNING_2] 		= heart.get_warning_code(1);
	TX_data[CAN_AMS_OK] 		= orion.get_AMS_ok();
	TX_data[CAN_PDOC_OK] 		= pdoc.get_pdoc_ok();
	TX_data[CAN_IMD_OK] 		= imd.get_IMD_ok();
	TX_data[CAN_ERROR_SPARE] 	= 0;
	can_transmission_h(CANMessage(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + TS_ERROR_WARNING_ID, &TX_data[0], dlc));
	
	dlc = 6;
	TX_data[CAN_DIGITAL_1_AIR_POWER] 			= air_power.read();
	TX_data[CAN_DIGITAL_1_AIR_NEG_RELAY] 		= AIR_neg_relay.get_relay();
	TX_data[CAN_DIGITAL_1_AIR_NEG_FEEDBACK] 	= AIR_neg_relay.get_feedback();
	TX_data[CAN_DIGITAL_1_AIR_POS_RELAY] 		= AIR_pos_relay.get_relay();
	TX_data[CAN_DIGITAL_1_AIR_POS_FEEDBACK] 	= AIR_pos_relay.get_feedback();
	TX_data[CAN_DIGITAL_1_PRECHARGE_RELAY] 		= precharge_relay.get_relay();
	TX_data[CAN_DIGITAL_1_SPARE_6] 				= 0;
	TX_data[CAN_DIGITAL_1_SPARE_7] 				= 0;
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
// void can1_recv_cb(){
// 	can1_rx_led = !can1_rx_led;

// 	if (can1.read(can1_msg)){
// 		switch(can1_msg.id){
// 			// Secret method to bypass voltage checks. UNSAFE! Use in case of emergency!
// 			case (CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + 0x0F):
// 				heart.set_heartbeat_state(PRECHARGE_STATE_PRECHARGING_TIMER);
// 				break;

// 			// Set discharge state for checking mismatch.
// 			case (CAN_DISCHARGE_MODULE_BASE_ADDRESS):
// 				discharge_module.set_precharge_discharge_state(heart.get_heartbeat_state(), (int)can1_msg.data[0]);
// 				break;
			
// 			// Use precharge button to begin precharge sequence.
// 			case (CAN_MOTEC_THROTTLE_CONTROLLER_BASE_ADDRESS + TS_DIGITAL_1_ID):
// 				// check if precharge button is pressed
// 				if (can1_msg.data[0] == 1 && heart.get_heartbeat_state() == PRECHARGE_STATE_IDLE) {
//           // pc.printf("Precharge button pressed, starting precharge routine\r\n");
// 					start_precharge_sequence_cb();
// 				}

// 				if (can1_msg.data[1] == 1 && heart.get_heartbeat_state() == PRECHARGE_STATE_PRECHARGED) {
//         	// pc.printf("Precharge button pressed, starting precharge routine\r\n");
// 					heart.set_heartbeat_state(PRECHARGE_STATE_DRIVE);
//         } 	
// 				break;

// 			// Break relay flow is not detected.	
//       //case (CAN_UCM4_BASE_ADDRESS+TS_ERROR_WARNING_ID):
// 			//	UCM4_Inverter.connect(can1_msg.data[1]);

// 			//case (CAN_UCM5_BASE_ADDRESS+TS_ERROR_WARNING_ID):
// 			//	UCM5_Accumulator.connect(can1_msg.data[1]);

// 			// Use charger presense to begin precharge sequence.
//             case (CAN_TC_CHARGER_STATUS_ID):
//                 if (heart.get_heartbeat_state() == 1){
//                     // pc.printf("Charger detected, starting precharge routine\r\n");
//                     start_precharge_sequence_cb();
//                 }
// 				break;

// 			// Set Orion BMS state and check safe to use.
// 			case (CAN_ORION_BMS_BASE_ADDRESS + TS_HEARTBEAT_ID):
// 				// Big endian & MSB
// 				// 0b000000, 1000000	Discharge Relay Enabled
// 				// 0b000000, 0100000	Charge Relay Enabled
// 				// 0b000000, 0010000	Charge Safety Enabled
// 				// Use bitwise operator to mask out all except relevent statuses.

// 				// 1. Connect Orion
// 				// 2. Check Relay Status disable ams ok if this fails. 
// 				orion.connect_orion(can1_msg.data[0] & 0b11100000);
// 				break;
			
// 			// Set Orion BMS voltages and check safe to use.
// 			case (CAN_ORION_BMS_BASE_ADDRESS + TS_ANALOGUE_1_ID):
// 				orion.set_low_voltage(can1_msg.data[0]*50);
// 				orion.set_high_voltage(can1_msg.data[1]*50);
// 				break;

// 			// Set orion temperatures and check safe to use.
// 			case (CAN_ORION_BMS_BASE_ADDRESS + TS_ANALOGUE_2_ID):
// 				orion.set_high_temperature(can1_msg.data[1]);
// 				break;			
// 		}
// 	}
// }

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

	ticker_heartbeat.attach(&heartbeat_cb, HEARTRATE);
	// heart.set_heartbeat_state(PRECHARGE_STATE_FAIL);

	can1.frequency(CANBUS_FREQUENCY);
	// can1.attach(&can1_recv_cb);
	ticker_can_transmit1.attach(&can1_trans_cb1, CAN_BROADCAST_INTERVAL);

	//wait(5);

	ticker_can_transmit2.attach(&can1_trans_cb2, CAN_BROADCAST_INTERVAL);

	// Re-enable interrupts again, now that interrupts are ready.
	__enable_irq();

	// Allow some time to settle!
	wait_us(1000000);
}

int main(){
    pc.printf("Starting ts_20 Precharge Controller (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s\r\n",__TIMESTAMP__);

	setup();

	pc.printf("Faults cleared, startup completed!\r\n");

	// Program loop. Error checking handled within state deamon.
	while(1) {}

	pc.printf("Is this a BSOD?");
	return 0;
} 
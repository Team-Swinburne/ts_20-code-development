// // TEAM SWINBURNE - TS_22
// // PRECHARGE CONTROLLER
// // NATALIE NG
// // Test File: test_PDOC.cpp (22/06/2022)

// /*
// BASIC FUNCTIONALITY TEST
// Checks that temperature sensors are functional and reading correctly
// Checks that PDOC relay reacts to temperature detected by temp sensors
// */

// //-----------------------------------------------
// // Initialisation
// //-----------------------------------------------

// #include <mbed.h>
// #include <CAN.h>
// #include "Adafruit_ADS1015.h"
// #include "precharge_pinout.h"

// #include "can_addresses.h"
// #include "ts_std_device.h"

// #include "hv_tools.h"
// #include "relays.h"
// #include "precharge_discharge.h"

// #include "imd.h"
// #include "orion.h"
// #include "watchdogs.h"

// //-----------------------------------------------
// // Device Parameters
// //-----------------------------------------------

// // HV Voltage Bridge Offset Resistors
// #define MC_R_CAL 										5000
// #define BATT_R_CAL 									5000
// #define MINIMUM_PRECHARGE_VOLTAGE		400
// #define MAXIMUM_PRECHARGE_VOLTAGE		600

// // ADS1115 ADDR PINS: GND 48 - VDD 49 - SDA 4A - SCL 4B
// #define PDOC_ADC_ADDR								0x4B
// #define MC_HV_SENSE_ADC_ADDR				0x49
// #define BATT_HV_SENSE_ADC_ADDR			0x48

// // Interval & Periods
// #define CAN_BROADCAST_INTERVAL      0.5
// #define PRECHARGE_TIMEOUT           5

// //-----------------------------------------------
// // Precharge States
// //-----------------------------------------------

// typedef enum PRECHARGE_STATES {
// 	PRECHARGE_STATE_FAIL,
// 	PRECHARGE_STATE_IDLE,
// 	PRECHARGE_STATE_PRECHARGING,
// 	PRECHARGE_STATE_PRECHARGING_TIMER,
// 	PRECHARGE_STATE_PRECHARGED,
// 	PRECHARGE_STATE_DRIVE,
// } precharge_states_t;

// //-----------------------------------------------
// // TS_STD_CAN_INTERPRETATIONS
// //-----------------------------------------------
// typedef enum CAN_ERROR_WARNING_SIGNALS{
// 	CAN_ERROR_1,
// 	CAN_ERROR_2,
// 	CAN_WARNING_1,
// 	CAN_WARNING_2,
// 	CAN_AMS_OK,
// 	CAN_PDOC_OK,
// 	CAN_IMD_OK,
// 	CAN_ERROR_SPARE,
// } can_error_warning_flag_t;

// typedef enum CAN_ANALOGUE_1_SIGNALS{
// 	CAN_ANALOGUE_1_PDOC_TEMPERATURE_1,
// 	CAN_ANALOGUE_1_PDOC_TEMPERATURE_2,
// 	CAN_ANALOGUE_1_PDOC_REF_TEMPERATURE_1,
// 	CAN_ANALOGUE_1_PDOC_REF_TEMPERATURE_2,
// 	CAN_ANALOGUE_1_HV_MC_SENSE_VOLTAGE_1,
// 	CAN_ANALOGUE_1_HV_MC_SENSE_VOLTAGE_2,
// 	CAN_ANALOGUE_1_HV_BATTERY_SENSE_VOLTAGE_1,
// 	CAN_ANALOGUE_1_HV_BATTERY_SENSE_VOLTAGE_2,
// } can_analogue_1_signals_t;

// typedef enum CAN_ANALOGUE_2_SIGNALS{
// 	CAN_ANALOGUE_2_IMD_PERIOD,
// 	CAN_ANALOGUE_2_IMD_FREQUENCY,
// 	CAN_ANALOGUE_2_IMD_DUTY_CYCLE,
// 	CAN_ANALOGUE_2_SPARE_3,
// 	CAN_ANALOGUE_2_SPARE_4,
// 	CAN_ANALOGUE_2_SPARE_5,
// 	CAN_ANALOGUE_2_SPARE_6,
// 	CAN_ANALOGUE_2_SPARE_7,
// } can_analogue_2_signals_t;

// typedef enum CAN_DIGITAL_1_SIGNALS{
// 	CAN_DIGITAL_1_AIR_POWER,
// 	CAN_DIGITAL_1_AIR_NEG_RELAY,
// 	CAN_DIGITAL_1_AIR_NEG_FEEDBACK,
// 	CAN_DIGITAL_1_AIR_POS_RELAY,
// 	CAN_DIGITAL_1_AIR_POS_FEEDBACK,
// 	CAN_DIGITAL_1_PRECHARGE_RELAY,
// 	CAN_DIGITAL_1_SPARE_6,
// 	CAN_DIGITAL_1_SPARE_7,
// } can_digital_1_signals_t;
// Orion af(PC_13);
// //-----------------------------------------------
// // Error/Warning Flags
// //-----------------------------------------------

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

// //-----------------------------------------------
// // Communications Interfaces
// //-----------------------------------------------

// // UART Interface
// Serial pc(PIN_SERIAL_TX, PIN_SERIAL_RX);                 	//TX, RX

// // I2C Interface
// I2C i2c1(PIN_I2C_SDA, PIN_I2C_SCL);     					//SDA, SCL

// // CANBUS Interface
// CAN can1(PIN_CAN1_RXD, PIN_CAN1_TXD);     					// RXD, TXD

// // CANBUS Message Format
// CANMessage can1_msg;

// //-----------------------------------------------
// // Interfaces
// //-----------------------------------------------

// Heart heart(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS, PIN_HEART_LED1, PIN_PCB_TEMP);

// Orion orion(PIN_AMS_OK);
// PDOC pdoc(i2c1, PDOC_ADC_ADDR, PIN_PDOC_OK);
// IMD imd(PIN_IMD_OK, PIN_IMD_DATA);

// HV_ADC hv_mc_sense(i2c1, MC_HV_SENSE_ADC_ADDR, MC_R_CAL);
// HV_ADC hv_battery_sense(i2c1, BATT_HV_SENSE_ADC_ADDR, BATT_R_CAL);

// Discharge_Module discharge_module;

// InterruptIn air_power(PIN_AIR_POWER);

// Relay precharge_relay(PIN_PRECHARGE_RELAY);
// AIR AIR_neg_relay(PIN_AIR_NEG_RELAY, PIN_AIR_NEG_RELAY_FB);
// AIR AIR_pos_relay(PIN_AIR_POS_RELAY, PIN_AIR_POS_RELAY_FB);

// Watchdogs UCM4_Inverter;
// Watchdogs UCM5_Accumulator;
// Watchdogs Motor_Controller;

// DigitalOut can1_rx_led(PIN_CAN1_RX_LED);
// DigitalOut can1_tx_led(PIN_CAN1_TX_LED);

// //-----------------------------------------------
// // Real Time Operations
// //-----------------------------------------------

// // Programmable Interrupt Timer Instances
// Ticker ticker_heartbeat;
// Ticker ticker_can_transmit1;
// Ticker ticker_can_transmit2;
// Timeout timeout_precharge;

// //-----------------------------------------------
// // Functions
// //-----------------------------------------------

// uint8_t array_to_uint8(bool arr[], int count){
//     int ret = 0;
//     int tmp;
//     for (int i = 0; i < count; i++) {
//         tmp = arr[i];
//         ret |= tmp << (count - i - 1);
//     }
//     return ret;
// }

// //-----------------------------------------------
// // Handlers
// //-----------------------------------------------

// // Flash the LED!
// bool can_transmission_h(CANMessage _can_message){
//     if(can1.write(_can_message)) {
// 		can1_tx_led = !can1_tx_led;
//         return true;
//     } else {
//         return false;
// 	}
// }

// //-----------------------------------------------
// // Callback Functions
// //-----------------------------------------------

// 	/*
// Heartbeat Callback
// 	Called every second and transmits the heartbeat message on the CANBUS. Will flash LED1 if 
// 	successful.
// 	*/
// void heartbeat_cb(){
//     if (can_transmission_h(heart.heartbeat())){
//         pc.printf("Heartbeat Success! State: %d Counter: %d\r\n", heart.get_heartbeat_state(), heart.get_heartbeat_counter());
//     } else {
//         // pc.printf("Hearts dead :(\r\n");
//     }
// }

// 	/*
// CAN Transmit 1
// 	Sent CAN messages relevant to the device. These should follow the Team Swinburne
// 	Standard format for ease of debugging. They are split in 2 to ensure callback executing time
// 	is not too long for the interupt.
// 	*/
// void can1_trans_cb1(){
// 	// can1_tx_led = !can1_tx_led;
// 	char TX_data[8] = {0};
// 	int dlc = 7;
// 	TX_data[CAN_ERROR_1] 		= heart.get_error_code(0);
// 	TX_data[CAN_ERROR_2] 		= heart.get_error_code(1);
// 	TX_data[CAN_WARNING_1] 		= heart.get_warning_code(0);
// 	TX_data[CAN_WARNING_2] 		= heart.get_warning_code(1);
// 	TX_data[CAN_AMS_OK] 		= orion.get_AMS_ok();
// 	TX_data[CAN_PDOC_OK] 		= pdoc.get_pdoc_ok();
// 	TX_data[CAN_IMD_OK] 		= imd.get_IMD_ok();
// 	//TX_data[CAN_ERROR_SPARE] 	= 0;
// 	can_transmission_h(CANMessage(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + TS_ERROR_WARNING_ID, &TX_data[0], dlc));
	
// 	dlc = 6;
// 	TX_data[CAN_DIGITAL_1_AIR_POWER] 			= air_power.read();
// 	TX_data[CAN_DIGITAL_1_AIR_NEG_RELAY] 		= AIR_neg_relay.get_relay();
// 	TX_data[CAN_DIGITAL_1_AIR_NEG_FEEDBACK] 	= AIR_neg_relay.get_feedback();
// 	TX_data[CAN_DIGITAL_1_AIR_POS_RELAY] 		= AIR_pos_relay.get_relay();
// 	TX_data[CAN_DIGITAL_1_AIR_POS_FEEDBACK] 	= AIR_pos_relay.get_feedback();
// 	TX_data[CAN_DIGITAL_1_PRECHARGE_RELAY] 		= precharge_relay.get_relay();
// 	//TX_data[CAN_DIGITAL_1_SPARE_6] 				= 0;
// 	//TX_data[CAN_DIGITAL_1_SPARE_7] 				= 0;
// 	can_transmission_h(CANMessage(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + TS_DIGITAL_1_ID, &TX_data[0], dlc));
// }

// 	/*
// Check Errors
// 	Checks for critical errors. Resultant value should be assigned to the Heart object; the heart
// 	will send the state of the device into fail as soon as possible, and disable the vehicle.
// 	*/
// uint8_t check_errors(){
// 	bool error_code[8];

// 	error_code[ERROR_AMS_FAIL] 					= !orion.get_AMS_ok();
// 	error_code[ERROR_PDOC_FAIL] 				= !pdoc.get_pdoc_ok();
// 	error_code[ERROR_IMD_FAIL] 					= !imd.get_IMD_ok();
// 	error_code[ERROR_ORION_TIMEOUT] 		= !orion.get_orion_connection_ok();
	
// 	error_code[ERROR_ORION_LOW_VOTLAGE] 	= orion.check_low_voltage();
// 	error_code[ERROR_ORION_HIGH_VOLTAGE] 	= orion.check_high_voltage();
// 	error_code[ERROR_ORION_OVERTEMPERATURE] = orion.check_overtemperature();
// 	error_code[ERROR_PERIPHERALS] = 0;
// 	//!UCM4_Inverter.get_device_ok() && !UCM5_Accumulator.get_device_ok();;

// 	return array_to_uint8(error_code, 8);
// }

// 	/*
// Check Warnings
// 	Checks for non-critical errors. Resultant value should be assigned to the Heart object.
// 	*/
// uint8_t check_warnings(){
// 	bool warning_code[8];

// 	warning_code[WARNING_PCB_OVERTEMPERATURE] 			= !heart.pcb_temperature.pcb_temperature_ok();
// 	warning_code[WARNING_DISCHARGE_PRECHARGE_MISMATCH] 	= !discharge_module.precharge_discharge_mismatch();
// 	warning_code[WARNING_AIR_NEG_FEEDBACK_MISMATCH] 	= !AIR_neg_relay.relay_ok();
// 	warning_code[WARNING_AIR_POS_FEEDBACK_MISMATCH] 	= !AIR_pos_relay.relay_ok();

// 	warning_code[WARNING_PDOC_SENSOR_FAILURE] 			= !pdoc.get_sensor_ok();
// 	warning_code[WARNING_MC_ADC_SENSOR_FAILURE] 		= !hv_mc_sense.get_sensor_ok();
// 	warning_code[WARNING_BATT_ADC_SENSOR_FAILURE]		= !hv_battery_sense.get_sensor_ok();
// 	warning_code[WARNING_PDOC_RELAY_FAILURE] 			= pdoc.check_pdoc_relay_fail();
	
// 	return array_to_uint8(warning_code, 8);
// }

// 	/*
// Update Precharge
// 	Updates the device as required, and checks for errors.
// 	*/
// void update_precharge(){
// 	// Update analogue measurements to get latest information.
// 	pdoc.update_adc();

// 	// Perform basic error checking. Sets state to FAIL if error found.
// 	heart.set_error_code(check_errors(), 0);
// 	heart.set_warning_code(check_warnings(), 0);
// }

// //-----------------------------------------------
// // Initialisations
// //-----------------------------------------------

// 	/*
// SETUP
// 	Initialisation of CANBUS, IMD, and HEARTBEAT. Mostly interrupt routines, callback
// 	documented above.
// 	*/
// void setup(){
// 	// Disable interrupts for smooth startup routine.
// 	wait(1);
	
// 	__disable_irq();

// 	ticker_heartbeat.attach(&heartbeat_cb, HEARTRATE);
// 	heart.set_heartbeat_state(PRECHARGE_STATE_FAIL);

// 	can1.frequency(CANBUS_FREQUENCY);
// 	// can1.attach(&can1_recv_cb);
// 	ticker_can_transmit1.attach(&can1_trans_cb1, CAN_BROADCAST_INTERVAL);

// 	//wait(5);

// 	// Re-enable interrupts again, now that interrupts are ready.
// 	__enable_irq();

// 	// Allow some time to settle!
// 	wait(1);

// 	// Assume device in failure mode, hold until all start up faults then 
// 	// force into idle mode. Mandated by EV.8.2.3.
// 	// do {
// 	// 	update_precharge();
// 	// } while (heart.get_error_code(0) > 0);
// }

int main(){
    // pc.printf("Starting ts_20 Precharge Controller (STM32F103C8T6 128k) \
	// \r\nCOMPILED: %s\r\n",__TIMESTAMP__);

	// setup();

	// pc.printf("Faults cleared, startup completed!\r\n");

	// // Program loop. Error checking handled within state deamon.
	// while(1){

    //     pdoc.update_adc();
	// 	// pc.printf("Is the pdoc temp values in the correct range? %d\n", pdoc.get_sensor_ok());
    //     // pc.printf("PDOC Temp is: %d  ", pdoc.get_pdoc_temperature());
    //     // pc.printf("PDOC Ref voltage is: %f  ", pdoc.get_pdoc_channel_voltage(PDOC_ADC_REF_CHANNEL));
	// 	// pc.printf("PDOC Temp1 voltage is: %f  ", pdoc.get_pdoc_channel_voltage(PDOC_ADC_SENSOR1_CHANNEL));
	// 	pc.printf("PDOC Ref Temp is: %d\n", pdoc.get_pdoc_ref_temperature()); //set at 180C
	// 	// pc.printf("PDOC OK: %d\n", pdoc.get_pdoc_ok());
	// 	//pc.printf("PDOC Test %f\n", pdoc.test_setpoint());
	// 	wait_us(1000000);
    // } 	

	// pc.printf("Is this a BSOD?");
	// return 0;
}
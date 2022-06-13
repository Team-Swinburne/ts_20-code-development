// TEAM SWINBURNE - TS_20
// DISCHARGE MODULE
// PATRICK CURTAIN
// REVISION 1 (03/07/2021)

/* 
To correct limited 64k flash issue: https://github.com/platformio/platform-ststm32/issues/195. Ensure device is 128k model. 
Use the following platformIO initialisation:
	[env:genericSTM32F103C8]
	platform = ststm32@4.6.0
	board = genericSTM32F103C8
	framework = mbed
	board_upload.maximum_size = 120000

//-----------------------------------------------

The Discharge Module is a custom PCB developed for use by Team Swinburne in ts_20. It is intended to complement the functionality
of the Precharge Controller. At its core it is a relay and a resistor that disengages when the negative AIR shuts, and high
voltage becomes active, the state of which controlled by the Precharge Controller. Additionally, the module also integrates a 
resistor divider and additional circuitry for the TSAL (Tractive System Active Light). For the ts_20 Discharge Module, a
microcontroller has been added to relay the state of the discharge to avoid a voltage divider situation (Precharge and discharge
resistors both active).

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
#include "discharge_pinout.h"

#include "can_addresses.h"
#include "ts_std_device.h"

#include "hv_tools.h"
#include "relays.h"
#include "precharge_discharge.h"

//-----------------------------------------------
// Device Parameters
//-----------------------------------------------

// HV Voltage Bridge Offset Resistors
#define MC_R_CAL 					5000

// Interval & Periods
#define CAN_BROADCAST_INTERVAL      0.5

//-----------------------------------------------
// Precharge States
//-----------------------------------------------

typedef enum DISCHARGE_STATES {
	DISCHARGE_STATE_FAIL,
	DISCHARGE_STATE_ACTIVE,
	DISCHARGE_STATE_DISABLED,
} discharge_states_t;

//-----------------------------------------------
// TS_STD_CAN_INTERPRETATIONS
//-----------------------------------------------

typedef enum CAN_ERROR_WARNING_SIGNALS{
	CAN_ERROR_1,
	CAN_ERROR_2,
	CAN_WARNING_1,
	CAN_WARNING_2,
	CAN_DISCHARGE_STATE,
	CAN_PDOC_OK,
	CAN_ERROR_SPARE_3,
	CAN_ERROR_SPARE_4,
} can_error_warning_flag_t;

typedef enum CAN_ANALOGUE_1_SIGNALS{
	CAN_ANALOGUE_1_PDOC_TEMPERATURE_1,
	CAN_ANALOGUE_1_PDOC_TEMPERATURE_2,
	CAN_ANALOGUE_1_PDOC_REF_TEMPERATURE_1,
	CAN_ANALOGUE_1_PDOC_REF_TEMPERATURE_2,
	CAN_ANALOGUE_1_HV_MC_SENSE_VOLTAGE_1,
	CAN_ANALOGUE_1_HV_MC_SENSE_VOLTAGE_2,
	CAN_ANALOGUE_1_TSAL_REFERENCE_1,
	CAN_ANALOGUE_1_TSAL_REFERENCE_2,
} can_analogue_1_signals_t;

//-----------------------------------------------
// Error/Warning Flags
//-----------------------------------------------

typedef enum ERROR_CODES_SUB_KEY {
  ERROR_SPARE_0,
  ERROR_PDOC_FAIL,
  ERROR_SPARE_2,
  ERROR_SPARE_3,
  ERROR_SPARE_4,
  ERROR_SPARE_5,
  ERROR_SPARE_6,
  ERROR_SPARE_7,
} error_state_t;

typedef enum WARNING_CODES_SUB_KEY {
  WARNING_PCB_OVERTEMPERATURE,
  WARNING_DISCHARGE_PRECHARGE_MISMATCH,
  WARNING_SPARE_2,
  WARNING_SPARE_3,
  WARNING_SPARE_4,
  WARNING_SPARE_5,
  WARNING_SPARE_6,
  WARNING_SPARE_7,
} warning_state_t;

//-----------------------------------------------
// Communications Interfaces
//-----------------------------------------------

// UART Interface
Serial pc(PIN_SERIAL_TX, PIN_SERIAL_RX);    // TX, RX

// I2C Interface
I2C i2c1(PIN_I2C_SDA, PIN_I2C_SCL);     	// SDA, SCL

// CANBUS Interface
CAN can1(PIN_CAN1_RXD, PIN_CAN1_TXD);     	// RXD, TXD

// CANBUS Message Format
CANMessage can1_msg;

//-----------------------------------------------
// Interfaces
//-----------------------------------------------

Heart heart(CAN_DISCHARGE_MODULE_BASE_ADDRESS, PIN_HEART_LED1, PIN_PCB_TEMP);

PDOC pdoc(i2c1, PDOC_ADC_ADDR, PIN_PDOC_OK);

HV_ADC hv_mc_sense(i2c1, MC_HV_SENSE_ADC_ADDR, MC_R_CAL);

Precharge_Controller precharge_controller;

DigitalIn discharge_relay(PIN_DISCHARGE_RELAY);

DigitalOut can1_rx_led(PIN_CAN1_RX_LED);
DigitalOut can1_tx_led(PIN_CAN1_TX_LED);

//-----------------------------------------------
// Real Time Operations
//-----------------------------------------------

// Programmable Interrupt Timer Instances
Ticker ticker_heartbeat;
Ticker ticker_can_transmit;
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
CAN Transmit
	Sent CAN messages relevant to the device. These should follow the Team Swinburne
	Standard format for ease of debugging.
	*/
void can1_trans_cb(){
	// can1_tx_led = !can1_tx_led;
	char TX_data[8] = {0};
	int dlc = 8;
	TX_data[CAN_ERROR_1] 			= heart.get_error_code(0);
	TX_data[CAN_ERROR_2] 			= heart.get_error_code(1);
	TX_data[CAN_WARNING_1] 			= heart.get_error_code(0);
	TX_data[CAN_WARNING_2] 			= heart.get_warning_code(1);
	TX_data[CAN_DISCHARGE_STATE] 	= discharge_relay.read();
	TX_data[CAN_PDOC_OK] 			= pdoc.get_pdoc_ok();
	TX_data[CAN_ERROR_SPARE_3] 		= 0;
	TX_data[CAN_ERROR_SPARE_4] 		= 0;
	can_transmission_h(CANMessage(CAN_DISCHARGE_MODULE_BASE_ADDRESS + TS_ERROR_WARNING_ID, &TX_data[0], dlc));
	
	TX_data[CAN_ANALOGUE_1_PDOC_TEMPERATURE_1] 			= (char)(pdoc.get_pdoc_temperature() >> 8);
	TX_data[CAN_ANALOGUE_1_PDOC_TEMPERATURE_2] 			= (char)(pdoc.get_pdoc_temperature() & 0xFF);
	TX_data[CAN_ANALOGUE_1_PDOC_REF_TEMPERATURE_1] 		= (char)(pdoc.get_pdoc_ref_temperature() >> 8);
	TX_data[CAN_ANALOGUE_1_PDOC_REF_TEMPERATURE_2] 		= (char)(pdoc.get_pdoc_ref_temperature() & 0xFF);
	TX_data[CAN_ANALOGUE_1_HV_MC_SENSE_VOLTAGE_1] 		= (char)(hv_mc_sense.get_voltage()*10 >> 8);
	TX_data[CAN_ANALOGUE_1_HV_MC_SENSE_VOLTAGE_2] 		= (char)(hv_mc_sense.get_voltage()*10 & 0xFF);
	TX_data[CAN_ANALOGUE_1_TSAL_REFERENCE_1] 	= (char)(hv_mc_sense.get_tsal_reference()*10 >> 8);
	TX_data[CAN_ANALOGUE_1_TSAL_REFERENCE_2] 	= (char)(hv_mc_sense.get_tsal_reference()*10 & 0xFF);
	can_transmission_h(CANMessage(CAN_DISCHARGE_MODULE_BASE_ADDRESS + TS_ANALOGUE_1_ID, &TX_data[0], dlc));
}

	/*
CAN Receive
	Message box for CANBUS. Switch handles which message is being serviced.
	*/
void can1_recv_cb(){
	can1_rx_led = !can1_rx_led;

	if (can1.read(can1_msg)){
		switch(can1_msg.id){
			// Check for precharge state, report if states do not align.
			case (CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS):
				precharge_controller.set_precharge_discharge_state(heart.get_heartbeat_state(), (int)can1_msg.data[0]);
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

	error_code[ERROR_SPARE_0] 		= 0;
	error_code[ERROR_PDOC_FAIL] 	= !pdoc.get_pdoc_ok();
	error_code[ERROR_SPARE_2] 		= 0;
	error_code[ERROR_SPARE_3] 		= 0;
	
	error_code[ERROR_SPARE_4] 		= 0;
	error_code[ERROR_SPARE_5] 		= 0;
	error_code[ERROR_SPARE_6] 		= 0;
	error_code[ERROR_SPARE_7] 		= 0;

	return array_to_uint8(error_code, 8);
}

	/*
Check Warnings
	Checks for non-critical errors. Resultant value should be assigned to the Heart object.
	*/
uint8_t check_warnings(){
	bool warning_code[8];

	warning_code[WARNING_PCB_OVERTEMPERATURE] 			= !heart.pcb_temperature.pcb_temperature_ok();
	warning_code[WARNING_DISCHARGE_PRECHARGE_MISMATCH] 	= precharge_controller.precharge_discharge_mismatch();
	warning_code[WARNING_SPARE_2] 	= 0;
	warning_code[WARNING_SPARE_3] 	= 0;

	warning_code[WARNING_SPARE_4] 	= 0;
	warning_code[WARNING_SPARE_5] 	= 0;
	warning_code[WARNING_SPARE_6]	= 0;
	warning_code[WARNING_SPARE_7]	= 0;
	
	return array_to_uint8(warning_code, 8);
}

	/*
State Deamon
	Deamon responsible for managing the state of the device.
	NB. Many of the functions are handled by interrupt routines, 
	care should be taken to ensure that this simply manages errors and the 
	relays. 
	*/
void state_d(){
	// Update analogue measurements to get latest information.
	pdoc.update_adc();
	hv_mc_sense.update_adc();

	// Perform basic error checking. Sets state to FAIL if error found.
	heart.set_error_code(check_errors(), 0);
	heart.set_warning_code(check_warnings(), 0);

	switch (heart.get_heartbeat_state()){
		case DISCHARGE_STATE_FAIL:
			if (check_errors() == 0){
				heart.set_heartbeat_state(DISCHARGE_STATE_ACTIVE);
			}
			break;

		case DISCHARGE_STATE_ACTIVE:
			// The idle state is advanced by callbacks from either the throttle's precharge button
			// or the charger becoming visible.
			heart.set_heartbeat_state(discharge_relay+1);

			break;

		case DISCHARGE_STATE_DISABLED:
			// After the precharge is advanced, the device has 5 seconds before timing out. 
			// A timeout event will cause the device to reset to the failed state.
			heart.set_heartbeat_state(discharge_relay+1);

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
	heart.set_heartbeat_state(DISCHARGE_STATE_FAIL);

	can1.frequency(CANBUS_FREQUENCY);
	can1.attach(&can1_recv_cb);
	ticker_can_transmit.attach(&can1_trans_cb, CAN_BROADCAST_INTERVAL);

	// Re-enable interrupts again, now that startup has competed.
	__enable_irq();

	wait(1);
}

int main(){
    pc.printf("Starting ts_20 Discharge Module (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s\r\n",__TIMESTAMP__);

	setup();

	pc.printf("Startup completed!\r\n");

	// Program loop. Error checking handled within state deamon.
	while(1){
		state_d();
	}

	pc.printf("Is this a BSOD?");
	return 0;
}
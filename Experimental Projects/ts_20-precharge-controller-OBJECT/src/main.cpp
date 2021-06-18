// TEAM SWINBURNE - TS_20
// PRECHARGE CONTROLLER
// PATRICK CURTAIN
// REVISION 0 (29/02/2020)

// TEST MODES
// 0 Normal	
// 1 Test Relays
// 2 Test Max Voltage 592
// 3 Test Max Voltage 250
// 4 Print Current Status
// 5 Print Raw ADC Value
#define TEST_MODE 0

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
a Formula SAE vehicle, mointor the health of the accumulator based on various inputs and react accordingly. This PCB 
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
#include <bitset>

#include "can_addresses.h"
#include "hv_tools.h"
#include "imd.h"
#include "pch_perf.h"
#include "relays.h"
#include "ts_std_device.h"

// #if TEST_MODE > 0
//     #include ".\unit_test.h"
// #endif

//-----------------------------------------------
// Calibration Factors
//-----------------------------------------------

// HV Voltage Bridge Offset Resistors
#define MC_R_CAL 5000
#define BATT_R_CAL 5000

// Voltage Limits
#define MINIMUM_CELL_VOLTAGE 27
#define MAXIMUM_CELL_VOLTAGE 43
#define MAXIMUM_CELL_TEMPERATURE 65

//-----------------------------------------------
// Communications Interfaces
//-----------------------------------------------

// UART Interface
Serial pc(PA_2, PA_3);                 		//TX, RX

// I2C Interface
I2C i2c1(PB_7, PB_6);     					//SDA, SCL

// CANBUS Interface
CAN can1(PB_8, PB_9);     					// RXD, TXD

// CANBUS Message Format
CANMessage can1_msg;

//-----------------------------------------------
// HW/Communication Interfaces
//-----------------------------------------------

// ADS1115 ADDR PINS: GND 48 - VDD 49 - SDA 4A - SCL 4B
#define PDOC_ADC_ADDR								0x4B
#define MC_HV_SENSE_ADC_ADDR						0x49
#define BATT_HV_SENSE_ADC_ADDR						0x48

PDOC pdoc(i2c1, PDOC_ADC_ADDR);
HV_ADC hv_mc_sense(i2c1, MC_HV_SENSE_ADC_ADDR, MC_R_CAL);
HV_ADC hv_battery_sense(i2c1, BATT_HV_SENSE_ADC_ADDR, BATT_R_CAL);

//-----------------------------------------------
// HW Interfaces
//-----------------------------------------------

Heart heart(PRECHARGE_CONTROLLER_HEARTBEAT_ID, PC_13, PA_0);
IMD_Data IMD_interface(PA_15);

Orion orion;
Discharge_Module discharge_module;

InterruptIn AIR_power(PB_3);

DigitalOut AMS_ok(PB_13);
DigitalIn IMD_ok(PB_12);
DigitalIn PDOC_ok(PB_15);

Relay precharge_relay(PA_9);
AIR AIR_neg_relay(PA_8, PB_10);
AIR AIR_pos_relay(PA_10, PB_11);

DigitalOut can1_rx_led(PB_1);
DigitalOut can1_tx_led(PB_0);

//-----------------------------------------------
// Error/Warning Definitions
//-----------------------------------------------

static std::bitset<8> error_code;
static std::bitset<8> warning_code;

typedef enum ERROR_CODES_kEY {
  AMS_OK,
  PDOC_OK,
  IMD_OK,
  ORION_OK,
  ORION_CONNECTED,
  LOW_CELL_VOLTAGE,
  HIGH_CELL_VOLTAGE,
  HIGH_CELL_TEMPERATURE,
} error_state_t;

typedef enum WARNING_CODES_kEY {
  PCB_OVERTEMPERATURE,
  DISCHARGE_PRECHARGE_MISMATCH,
  AIR_NEG_FEEDBACK_MISMATCH,
  AIR_POS_FEEDBACK_MISMATCH,
  WARNING_SPARE_4,
  WARNING_SPARE_5,
  WARNING_SPARE_6,
  WARNING_SPARE_7,
} warning_state_t;

//-----------------------------------------------
// Real Time Operations
//-----------------------------------------------

// Interval & Periods
#define	HEARTRATE			 				1
#define CAN_BROADCAST_INTERVAL              0.2
#define ORION_TIMEOUT_INTERVAL				0.25
#define PRECHARGE_TIMEOUT                   5

// Programmable Interrupt Timer Instances
Ticker ticker_heartbeat;
Ticker ticker_can_transmit;
Timeout timeout_precharge;

//-----------------------------------------------
// Functions
//-----------------------------------------------

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

void relay_state_precharged_1(){
    AIR_neg_relay.activate_relay();
    precharge_relay.activate_relay();
    AIR_pos_relay.activate_relay();
}

void relay_state_precharged_2(){
    AIR_neg_relay.activate_relay();
    precharge_relay.disable_relay();
    AIR_pos_relay.activate_relay();
}

bool check_precharged(){
    if (hv_mc_sense.get_voltage() > hv_battery_sense.get_voltage()*0.95){
        relay_state_precharged_1();
        // pc.printf("Precharge within 95%, safe to close postive contactor\r\n");
        heart.set_heartbeat_state(3);
        return true;
    } else {
        return false;
    }
}

//-----------------------------------------------
// Handlers
//-----------------------------------------------

bool can_transmission_handler(CANMessage _can_message){
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

void heartbeat_cb(){
    if (can_transmission_handler(heart.heartbeat())){
        pc.printf("Heartbeat Success! State: %d Counter: %d\r\n", heart.get_heartbeat_state(), heart.get_heartbeat_counter());
    } else {
        pc.printf("Hearts dead :(\r\n");
    }
}

void air_power_lost_cb(){
    heart.set_heartbeat_state(1);
}

void precharge_timeout_cb(){
	if (!check_precharged()){
		heart.set_heartbeat_state(1);
	}
}

void start_precharge_cb(){
    relay_state_precharging();
    timeout_precharge.attach(&precharge_timeout_cb, PRECHARGE_TIMEOUT);
}

void can1_trans_cb(){
	can1_tx_led = !can1_tx_led;

}

	/*
CAN RECEIVE
	Message box for CANBUS.
	*/
void can1_recv_cb(){
	can1_rx_led = !can1_rx_led;

	if (can1.read(can1_msg)){
		switch(can1_msg.id){
			case DISCHARGE_MODULE_HEARTBEAT_ID:
				discharge_module.set_discharge_state((int)can1_msg.data[0]);
				break;

			case THROTTLE_CONTROLLER_PERIPERAL_ID:
				if (heart.get_heartbeat_state() == 1){
                    start_precharge_cb();
                    // pc.printf("Precharge button pressed, starting precharge routine\r\n");
                }
				break;
            
            case TC_CHARGER_STATUS_ID:
                if (heart.get_heartbeat_state() == 1){
                    // pc.printf("Charger detected, starting precharge routine\r\n");
                    start_precharge_cb();
                }
				break;
            // Plenty to disassemble here.  

			case ORION_BMS_STATUS_ID:
				// Big endian & MSB
				// 0b000000, 0000001	Discharge Relay Enabled
				// 0b000000, 0000010	Charge Relay Enabled
				// 0b000000, 0000100	Charge Safety Enabled
				// Use bitwise operator to mask out all except relevent statuses.

				// 1. Connect Orion
				// 2. Check Relay Status disable ams ok if this fails. 

				break;
			
			case ORION_BMS_VOLTAGE_ID:
				// 2. Stash these values for later.
				// 3. Stash 
				break;

			case ORION_BMS_TEMPERATURE_ID:
				// orion_high_temperature = can1_msg.data[2];
				break;			
		}
	}
}

void stated(){
	switch (heart.get_heartbeat_state()){
		case 0:	// Fail State (Default){
			relay_state_safe();
			break;

		case 1: // Idle State
			relay_state_safe();
			break;

		case 2: // Precharging State
			relay_state_precharging();
            check_precharged();
			break;
			
		case 3: // Precharged State
			relay_state_precharged_2();
			break;
	}
}

bool errord(){
	error_state_t error_code_key[8];

	error_code.set(error_code_key[AMS_OK], AMS_ok.read());
	error_code.set(error_code_key[PDOC_OK], PDOC_ok.read());
	error_code.set(error_code_key[IMD_OK], IMD_ok.read());
	error_code.set(error_code_key[ORION_OK], orion.get_orion_safe_status());

	error_code.set(error_code_key[ORION_CONNECTED], orion.orion_connected());
	error_code.set(error_code_key[LOW_CELL_VOLTAGE], 0);
	error_code.set(error_code_key[HIGH_CELL_VOLTAGE], 0);
	error_code.set(error_code_key[HIGH_CELL_TEMPERATURE], 0);

	// Needs to be two beause of the internal status of AMS_ok
	if (error_code == 0 || error_code == 1){
		AMS_ok = true;
		return true;
	} else {
		AMS_ok = false;
		return false;
	}
}

bool warnd(){
	warning_state_t warning_code_key[8];

	warning_code.set(warning_code_key[PCB_OVERTEMPERATURE], heart.pcb_temperature.pcb_temperature_ok());
	warning_code.set(warning_code_key[DISCHARGE_PRECHARGE_MISMATCH], discharge_module.check_precharge_discharge_mismatch(heart.get_heartbeat_state()));
	warning_code.set(warning_code_key[AIR_NEG_FEEDBACK_MISMATCH], AIR_neg_relay.relay_ok());
	warning_code.set(warning_code_key[AIR_POS_FEEDBACK_MISMATCH], AIR_pos_relay.relay_ok());

	warning_code.set(warning_code_key[WARNING_SPARE_4], 0);
	warning_code.set(warning_code_key[WARNING_SPARE_5], 0);
	warning_code.set(warning_code_key[WARNING_SPARE_6], 0);
	warning_code.set(warning_code_key[WARNING_SPARE_7], 0);
	
	if (warning_code == 0){
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------
// Initialisations
//-----------------------------------------------

	/*
SETUP
	Initialisation of CANBUS, ADC, and HEARTBEAT. 
	*/
void setup(){
	can1.frequency(CANBUS_FREQUENCY);
	can1.attach(&can1_recv_cb);
	
	ticker_heartbeat.attach(&heartbeat_cb, HEARTRATE);

    AIR_power.fall(air_power_lost_cb);

	ticker_can_transmit.attach(&can1_trans_cb, CAN_BROADCAST_INTERVAL);

	IMD_interface.start();
}


int main(){
	// #if TEST_MODE == 1
	// 	test_relays(2, 1);
	// #elif TEST_MODE == 2
	// 	test_precharge_sequence(592);
	// #elif TEST_MODE == 3
	// 	test_precharge_sequence(250);
	// #elif TEST_MODE == 4
	// 	print_current_status(1);
	// #elif TEST_MODE == 5
	// 	print_raw_adc(2);
	// #else

	__disable_irq();
    pc.printf("Starting ts_20 Precharge Controller (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s\r\n",__TIMESTAMP__);
	setup();

	// pc.printf("Finished Startup\r\n");
	wait(1);
	__enable_irq();

	while(1){
		stated();
		if (errord()){
			heart.set_heartbeat_state(0);
		}
		warnd();
	}
	
	// #endif

	pc.printf("Is this a BSOD?");
	return 0;
}
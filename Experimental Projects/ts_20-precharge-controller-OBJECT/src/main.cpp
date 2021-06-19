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

#include "ts_std_device.h"
#include "can_addresses.h"
#include "hv_tools.h"
#include "imd.h"
#include "precharge_peripheral_devices.h"
#include "relays.h"

// #if TEST_MODE > 0
//     #include ".\unit_test.h"
// #endif

//-----------------------------------------------
// Device Parameters
//-----------------------------------------------

// HV Voltage Bridge Offset Resistors
#define MC_R_CAL 5000
#define BATT_R_CAL 5000

// ADS1115 ADDR PINS: GND 48 - VDD 49 - SDA 4A - SCL 4B
#define PDOC_ADC_ADDR								0x4B
#define MC_HV_SENSE_ADC_ADDR						0x49
#define BATT_HV_SENSE_ADC_ADDR						0x48

// Interval & Periods
#define	HEARTRATE			 				1
#define CAN_BROADCAST_INTERVAL              0.2
#define PRECHARGE_TIMEOUT                   5

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
// Interfaces
//-----------------------------------------------

Heart heart(PRECHARGE_CONTROLLER_HEARTBEAT_ID, PC_13, PA_0);

Orion orion(PB_13);
PDOC pdoc(i2c1, PDOC_ADC_ADDR, PB_15);
IMD imd(PB_12, PA_15);

HV_ADC hv_mc_sense(i2c1, MC_HV_SENSE_ADC_ADDR, MC_R_CAL);
HV_ADC hv_battery_sense(i2c1, BATT_HV_SENSE_ADC_ADDR, BATT_R_CAL);

Discharge_Module discharge_module;

InterruptIn air_power(PB_3);

Relay precharge_relay(PA_9);
AIR AIR_neg_relay(PA_8, PB_10);
AIR AIR_pos_relay(PA_10, PB_11);

DigitalOut can1_rx_led(PB_1);
DigitalOut can1_tx_led(PB_0);

//-----------------------------------------------
// Precharge States
//-----------------------------------------------

typedef enum PREcHARGE_STATES {
	PRECHARGE_STATE_FAIL,
	PRECHARGE_STATE_IDLE,
	PRECHARGE_STATE_PRECHARGING,
	PRECHARGE_STATE_PRECHARGED,
} precharge_states_t;

//-----------------------------------------------
// Error/Warning Definitions
//-----------------------------------------------

typedef enum ERROR_CODES_KEY {
  ERROR_AMS_OK,
  ERROR_PDOC_OK,
  ERROR_IMD_OK,
  ERROR_ORION_OK,
  ERROR_SPARE_4,
  ERROR_SPARE_5,
  ERROR_SPARE_6,
  ERROR_SPARE_7,
} error_state_t;

typedef enum WARNING_CODES_KEY {
  WARNING_PCB_OVERTEMPERATURE,
  WARNING_DISCHARGE_PRECHARGE_MISMATCH,
  WARNING_AIR_NEG_FEEDBACK_MISMATCH,
  WARNING_AIR_POS_FEEDBACK_MISMATCH,
  WARNING_SPARE_4,
  WARNING_SPARE_5,
  WARNING_SPARE_6,
  WARNING_SPARE_7,
} warning_state_t;

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

uint8_t array_to_uint8(bool arr[])
{	
	int count = 8;
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
	
	/** check_precharged()
	 * 
	 * checks if the precharge voltages are within 95% of each other.
	 * This should be included in the state machine to bypass the timeout, and advance the state.
     * 
	 * @returns precharge_successful flag if true.
	 * 
     */
bool check_precharged(){
    if (hv_mc_sense.get_voltage() > hv_battery_sense.get_voltage()*0.95){
        relay_state_precharged_1();
        // pc.printf("Precharge within 95%, safe to close postive contactor\r\n");
        heart.set_heartbeat_state(PRECHARGE_STATE_PRECHARGED);
        return true;
    } else {
        return false;
    }
}

//-----------------------------------------------
// Handlers
//-----------------------------------------------

// Flash the LED!
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

	/*
Heartbeat Callback
	Called every second and transmits the heartbeat message on the CANBUS. Will flash LED1 if 
	successful.
	*/
void heartbeat_cb(){
    if (can_transmission_handler(heart.heartbeat())){
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
	orion.set_AMS_ok(0);
    heart.set_heartbeat_state(PRECHARGE_STATE_FAIL);
}
	
	/*
Precharge Timeout Callback
	Called after 5 seconds of the precharge. Performs quick check to make sure the 
	precharge hasn't completed yet. This callback is set within the start_precharge_cb()
	call, which is itself called from the receive callback if the charger or precharge button is pressed.
	...
	timeout_precharge.attach(&precharge_timeout_cb, PRECHARGE_TIMEOUT);
	*/
void precharge_timeout_cb(){
	if (!check_precharged()){
		heart.set_heartbeat_state(PRECHARGE_STATE_FAIL);
		orion.set_AMS_ok(0);
	}
}

	/*
Start Precharge Callback
	Called by the CAN receive callback, attaches precharge_timeout_cb() function to 
	check if the precharge has been successful, otherwise, the precharge will be disabled.
	*/
void start_precharge_cb(){
    relay_state_precharging();
    timeout_precharge.attach(&precharge_timeout_cb, PRECHARGE_TIMEOUT);
}

	/*
CAN Transmit
	Sent CAN messages relevant to the device. These should follow the standard,
	Team Swinburne format for simple interpretation.
	*/
void can1_trans_cb(){
	can1_tx_led = !can1_tx_led;
}

	/*
CAN Receive
	Message box for CANBUS. Switch handles which message is being serviced.
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
                    // pc.printf("Precharge button pressed, starting precharge routine\r\n");
					start_precharge_cb();
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
				orion.connect_orion((int)can1_msg.data[0]);

				break;
			
			case ORION_BMS_VOLTAGE_ID:
				// 1. Stash these values for later.
				break;

			case ORION_BMS_TEMPERATURE_ID:
				// orion_high_temperature = can1_msg.data[2];
				break;			
		}
	}
}

	/*
Error Deamon
	Checks for critical errors. Should be assigned to the Heart object; the heart
	will send the state of the device into fail as soon as possible, and disable the vehicle.
	*/
uint8_t error_d(){
	bool error_code[8];

	error_code[ERROR_AMS_OK] = orion.get_AMS_ok();
	error_code[ERROR_PDOC_OK] = pdoc.get_pdoc_ok();
	error_code[ERROR_IMD_OK] = imd.get_IMD_ok();
	error_code[ERROR_ORION_OK] = orion.check_orion_safe();
	error_code[ERROR_SPARE_4] = false;
	error_code[ERROR_SPARE_5] = false;
	error_code[ERROR_SPARE_6] = false;
	error_code[ERROR_SPARE_7] = false;

	return array_to_uint8(error_code);
}

	/*
Warning Deamon
	Checks for non-critical errors. Should be assigned to the Heart object.
	*/
uint8_t warn_d(){
	bool warning_code[8];

	warning_code[WARNING_PCB_OVERTEMPERATURE] = heart.pcb_temperature.pcb_temperature_ok();
	warning_code[WARNING_DISCHARGE_PRECHARGE_MISMATCH] = discharge_module.check_precharge_discharge_mismatch(heart.get_heartbeat_state());
	warning_code[WARNING_AIR_NEG_FEEDBACK_MISMATCH] = AIR_neg_relay.relay_ok();
	warning_code[WARNING_AIR_POS_FEEDBACK_MISMATCH] = AIR_pos_relay.relay_ok();

	warning_code[WARNING_SPARE_4] = 0;
	warning_code[WARNING_SPARE_5] = 0;
	warning_code[WARNING_SPARE_6] = 0;
	warning_code[WARNING_SPARE_7] = 0;
	
	return array_to_uint8(warning_code);
}
	/*
State Deamon
	Deamon responsible for managing the state of the device.
	NB. Many of the functions are handled by interrupt routines, 
	care should be taken to ensure that this simply manages errors and the 
	relays. 
	*/
void state_d(){
	// Perform basic error checking. Sets state to 0 if error
	// found.
	heart.set_error_code(error_d(), 0);
	heart.set_warning_code(warn_d(), 0);

	switch (heart.get_heartbeat_state()){
		case PRECHARGE_STATE_FAIL:
			// Do nothing. 
			relay_state_safe();
			break;

		case PRECHARGE_STATE_IDLE:
			// The idle state is advanced by callbacks from either the throttle's precharge button
			// or the charger becoming visible.
			relay_state_safe();
			break;

		case PRECHARGE_STATE_PRECHARGING:
			// After the precharge is advanced, the device has 5 seconds before timing out. 
			// A timeout event will cause the device to reset to the failed state.
			relay_state_precharging();
            check_precharged();
			break;
			
		case PRECHARGE_STATE_PRECHARGED:
			// The drive or charging state fo the vehicle. This state is deactivated by breaking the 
			// green loop. The intended way being the cockpit E-Stop, or by power cycling the vehicle...
			// Otherwise something's gone wrong.
			relay_state_precharged_2();
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
	ticker_heartbeat.attach(&heartbeat_cb, HEARTRATE);

	can1.frequency(CANBUS_FREQUENCY);
	can1.attach(&can1_recv_cb);
	
    air_power.fall(&air_power_lost_cb);

	ticker_can_transmit.attach(&can1_trans_cb, CAN_BROADCAST_INTERVAL);

	imd.start();
}


int main(){
	// Disable interrupts for smooth startup routine.
	__disable_irq();

    pc.printf("Starting ts_20 Precharge Controller (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s\r\n",__TIMESTAMP__);
	setup();

	wait(1);
	
	// Re-enable interrupts again, now that startup has competed.
	__enable_irq();

	// Program loop. Error checking handled within state deamon.
	while(1){
		state_d();
	}

	pc.printf("Is this a BSOD?");
	return 0;
}
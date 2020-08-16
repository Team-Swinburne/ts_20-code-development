// TEAM SWINBURNE - TS_20
// GENERIC CAN READER
// PATRICK CURTAIN
// REVISION 0 (16/8/2020)

/* 
To correct limited 64k flash issue: https://github.com/platformio/platform-ststm32/issues/195. Ensure device is 128k model. 
Use the following platformIO initialisation:
	[env:genericSTM32F103C8]
	platform = ststm32@4.6.0
	board = genericSTM32F103C8
	framework = mbed
	board_upload.maximum_size = 120000

//-----------------------------------------------

Just a generic program that will output CAN messages in a simple format. Pair this up with the Python script that can log this to a
CSV and this is a pretty powerful tool.

//-----------------------------------------------

Based on STM32F103T6R8 "Blue Pill".

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

//-----------------------------------------------
// INTERFACES
//-----------------------------------------------

// UART Interface
Serial pc(PA_2, PA_3);              	  	 //TX, RX

// CANBUS Interface
CAN can1(PB_8, PB_9);     					// RXD, TXD

// CANBUS Addresses
#define GENERIC_HEARTBEAT_ID              	0x10

// Time Interface
Timer t;

//-----------------------------------------------
// Globals
//-----------------------------------------------

int8_t  heartbeat_state 			               = 0;
int	    heartbeat_counter 			               = 0;

// Program Interval Timer Instances
Ticker ticker_heartbeat;
Ticker ticker_can_transmit;

// Interval Periods
#define	HEARTRATE			                        1
#define CAN_BROADCAST_INTERVAL                  	0.05

//-----------------------------------------------
// GPIO 
//-----------------------------------------------
DigitalOut led1(PC_13);
DigitalOut can1_rx_led(PB_1);
DigitalOut can1_tx_led(PB_0);

//-----------------------------------------------
// Functions
//-----------------------------------------------

	/*
HEARTBEAT
	The heartbeat is used to relay the state of the device and time since device epoch to the CANBUS, allowing 
	for the logging of faulure events. 
	*/
void heartbeat(){
	heartbeat_counter++;
	led1 = !led1;
	char TX_data[2] = {(char)heartbeat_state, (char)heartbeat_counter};
	if(can1.write(CANMessage(GENERIC_HEARTBEAT_ID, &TX_data[0], 2))) {
    // pc.printf("Heartbeat Success! State: %d Counter: %d\r\n", heartbeat_state, heartbeat_counter);
  }
  else {
		// pc.printf("Hearts dead :(\r\n");
	}
}

	/*
CAN RECEIVE
	Message box for CAN messages to be interpreted.
	*/
void CAN1_receive(){
	can1_rx_led = !can1_rx_led;

  CANMessage can1_msg;
  if (can1.read(can1_msg)){
    pc.printf("%f, ", t.read());

    pc.printf("%X, ", can1_msg.id);
    pc.printf("%u, ", can1_msg.len);

    for (int i=1; i<can1_msg.len; i++){
      pc.printf("%02X, ", can1_msg.data[i]);
    }
	pc.printf("%02X", can1_msg.data[can1_msg.len]);

    pc.printf("\r\n");
	}
}

  	/* 
CAN TRANSMIT
	Outgoing messages sent periodically (CAN_BROADCAST_INTERVAL).
	*/
void CAN1_transmit(){
  // can1_tx_led = !can1_tx_led;
}

	/* 
SEERIAL RECEIVE
	Check incoming characters from the serial.
	*/
void serial_receive(){
	if (pc.readable()){
		char c = pc.getc();
		
		// Use this to reset the programmer if required.
		// if (c == 'R'){
			__disable_irq();
			pc.printf("\r\n\r\n\r\n");
			NVIC_SystemReset();
		// }
	}
}

	/*
SETUP
	Initialisation of CANBUS, ADC, and PIT. 
	*/
void setup(){
	can1.frequency(250000);
	can1.attach(&CAN1_receive);

	pc.attach(&serial_receive);
	
	ticker_heartbeat.attach(&heartbeat, HEARTRATE);
	ticker_can_transmit.attach(&CAN1_transmit, CAN_BROADCAST_INTERVAL);
 	t.start();
}

//-----------------------------------------------
// Program Loop
//-----------------------------------------------

int main() {
	__disable_irq();
    pc.printf("Starting ts_20 Generic CAN Reader (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);
	setup();

	pc.printf("Finished Startup\r\n");
	wait(1);

	pc.printf("Time, ID (HEX), Length, Data[0], Data[1], Data[2], Data[3], Data[4], Data[5], Data[6], Data[7]\r\n");
	__enable_irq();

	while(1) {}

	printf("Is this a BSOD?");
    return 0;
}

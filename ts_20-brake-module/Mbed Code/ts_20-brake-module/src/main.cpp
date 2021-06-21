#include <mbed.h>
#include <CAN.h>

// UART interface
Serial pc(PA_2, PA_3);

//MACRO FUNCTIONS
//#define MAX(a, b) (a > b) ? a : b;
//#define MIN(a, b) (a > b) ? b : a;

// Brake Characteristics -------------------------
#define Deadzone        30    // 30% Pedal deadzone

//FUNCTION PROTOTYPES
void heartBeat_TX();
void CAN_TX();
void CAN_RX();
void updateValues();
void updateBrakeOne();
void updateBrakeTwo();
//void SerialPrint();

//Initialize pins
AnalogIn readSensor1(PA_4); //sensor 1
AnalogIn readSensor2(PA_5); //sensor 2
DigitalIn readLowPressure(PA_6); //high pressure
DigitalIn readHighPressure(PA_7); //low pressure
DigitalIn readCurrentSensor(PA_8); //current sensor 5KW
DigitalIn readBSPD(PA_9); //BSPD_OK (no delay)
DigitalIn readBSPD_Delay(PB_12); //BSPD_OK (10 second delay)

DigitalOut canRXLed(PB_0);
DigitalOut canTXLed(PB_1);
DigitalOut debugLedOut(PC_13);

// Brake limits -----------------------------------
int16_t     brake1_min          = 0;   // TS_19 = 980
int16_t     brake1_max          = 0;   // TS_19 = 1755
int16_t     brake2_min          = 0;   // TS_19 = 400
int16_t     brake2_max          = 0;   // TS_19 = 1250

// Brake readings ---------------------------------
int16_t     brake1_raw          = 0;
float       brake1_percent      = 0;
int16_t     brake2_raw          = 0;
float       brake2_percent      = 0;
int         brake_avg_percent   = 0;

// CAN Initialization ------------------------------
int BRAKE_HEARTBEAT_ID = 0x308;
int BRAKE_DATA_ID    = 0x309;
int rxMsgID = 0x306;    // ID of brake sensor data -- for rx filtering
int       id;
int       fltIdx;
uint8_t   rxbytes[8];
uint8_t   rxData[8];
uint8_t heartbeat_cnt = 0;
uint8_t heartbeat_state = 1;
uint8_t High_Pressure = 0;
uint8_t Low_Pressure = 0;
uint8_t five_kW = 0;
uint8_t BSPD_OK = 0;
CAN can1(PB_8, PB_9);
#define CANBUS_FREQUENCY 							500000
CANMessage can1_msg;

// Program Interval Timer Instances
Ticker ticker_heartbeat;
Ticker ticker_can_transmit;

// Interval Periods
#define	HEARTRATE			                        1
#define CAN_BROADCAST_INTERVAL                  	0.2

int main() {

  // put your setup code here, to run once:
  pc.printf("Starting ts_20 Brake Module (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);

	pc.printf("Finished Startup\r\n");
	//wait(1);

  can1.frequency(CANBUS_FREQUENCY);
	
	ticker_heartbeat.attach(&heartBeat_TX, HEARTRATE);
	ticker_can_transmit.attach(&CAN_TX, CAN_BROADCAST_INTERVAL);

  while(1) {
    // put your main code here, to run repeatedly:
    updateValues();
    CAN_RX();

  }
}

void heartBeat_TX() 
{
  if (heartbeat_cnt >= 255) {
    heartbeat_cnt = 0;
  }
  heartbeat_cnt++;
  uint8_t txData[2] = {heartbeat_state, heartbeat_cnt};
  uint8_t txDataLen = 2;
  can1.write(CANMessage(BRAKE_HEARTBEAT_ID, txData, txDataLen));
  //digitalToggle(pin_LED_tx);
  debugLedOut = !debugLedOut;
}

void CAN_TX()
{
  // Brake Data
  uint8_t txData[5] = {(uint8_t)brake1_percent, (uint8_t)brake2_percent, (uint8_t)brake_avg_percent, BSPD_OK, five_kW};

  canTXLed.write(1); //turn on CAN TX indicator
  can1.write(CANMessage(BRAKE_DATA_ID, txData, 5));
  canTXLed.write(0); //turn off CAN TX indicator
}

void CAN_RX()
{
  if (can1.read(can1_msg)) // poll for rx
  {
    canRXLed.write(1);        // turn on CAN RX indicator
    //rxData[]=rxbytes;
    pc.printf("Receiving...\r\n");
    canRXLed.write(0);       // turn off CAN RX indicator
  }
}

void updateValues() 
{
  brake1_raw = readSensor1;
  brake2_raw = readSensor2;
  High_Pressure = !readHighPressure;
  Low_Pressure = readLowPressure;
  five_kW = readCurrentSensor;
  BSPD_OK = readBSPD;
  //BSPD_OK = readBSPD_Delay; //uncomment to include 10 second delay after BSPD is tripped

  updateBrakeOne(); 
  updateBrakeTwo();   // inverted
  brake_avg_percent = (brake1_percent + brake2_percent)/2.0;
}

void updateBrakeOne()
{   
  brake1_percent = ((brake1_raw - brake1_min)/(brake1_max - brake1_min))*100;
  // Filter data to ensure it is within 0-100 percent and ignore the deadzone
  brake1_percent = max(brake1_percent - Deadzone,brake1_percent);
  brake1_percent = ((brake1_percent - Deadzone) / (100- Deadzone)) * 100;
  brake1_percent = min(brake1_percent, float(100));
}

void updateBrakeTwo(){   
  brake2_percent = ((brake2_raw - brake2_min)/(brake2_max - brake2_min))*100;
  // Filter data to ensure it is within 0-100 percent
  brake2_percent = max(brake2_percent - Deadzone,brake2_percent);
  brake2_percent = ((brake2_percent - Deadzone) / (100- Deadzone)) * 100;
  brake2_percent = min(brake2_percent, float(100));
  // Invert brake input
  brake2_percent = 100 - brake2_percent;
}

/*
void SerialPrint() 
{
  pc.printf("Sensor 1 & 2:      ");
  pc.printf("%d", brake1_raw); 
  pc.printf("     ");
  pc.printf("%d\r\n", brake2_raw); 
  pc.printf("High Pressure: ");
  pc.printf("%d\r\n", High_Pressure); 
  pc.printf("Low  Pressure: ");
  pc.printf("%d\r\n", Low_Pressure); 
  pc.printf("5kW:       ");
  pc.printf("%d\r\n", five_kW);
  pc.printf("BSPD OK:       ");
  pc.printf("%d\r\n", BSPD_OK);
  pc.printf(" ");
  wait(0.5);
}
*/
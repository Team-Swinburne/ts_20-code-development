/*  TEAM SWINBURNE - TS21 
    UNIVERSAL CONTROL MODULE - HARDWARE REVISION 0
    BEN MCINNES, NAM TRAN, BRAD
    REVISION 1 (23/06/2021)
*/

//-----------------------------------------------
// Libraries
//-----------------------------------------------
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <eXoCAN.h>
#include <can_addresses.h>

//-----------------------------------------------
// INTERFACES
//-----------------------------------------------

// I2C Interface
Adafruit_ADS1115 ads;
#define Serial1_UART_INSTANCE    1 //ex: 2 for Serial2 (USART2)
#define PIN_Serial1_RX           PA10
#define PIN_Serial1_TX           PA9

// CANBus Interface
eXoCAN can;

//-----------------------------------------------
// GPIO
//-----------------------------------------------
#define pin_D1                          PB10  // Digital Input 1
#define pin_D2                          PB11  // Digital Input 2
#define pin_Driver1                     PB15  // Driver 1 (24V)
#define pin_Driver2                     PB14  // Driver 2 (24V)
#define pin_PWM_Driver1                 PB0   // PWM Driver 1 (5V)
#define pin_PWM_Driver2                 PB1   // PWM Driver 2 (5V)

//-----------------------------------------------
// Structs, typedef and enum
//-----------------------------------------------
typedef void (*fpointer)(); // function pointer for passing ticker callback

struct txFrame
{
  uint8_t len = 8;
  uint8_t bytes[8] = {0};
};
//-----------------------------------------------
// Globals
//-----------------------------------------------
int cnt = 0;
static txFrame heartFrame {.len = 6},
               errorFrame, 
               digitalFrame1,
               analogFrame1,
               rxFrame;

//-----------------------------------------------
// Classes
//-----------------------------------------------
// Ticker class, only support 5 objects at most
class Ticker {
private:
  fpointer callback;
  uint32_t freq_us;
  uint8_t timerIdx;
  void timerInit(HardwareTimer *timer) {
    timer->setOverflow(freq_us,MICROSEC_FORMAT); // 10 Hz
    timer->attachInterrupt(callback);
    timer->resume();
  }
public:
  static uint8_t objectCount;
  Ticker (fpointer f, uint16_t freq_ms) {
    timerIdx = objectCount;
    callback = f;
    freq_us = freq_ms*1000;
    objectCount++;
  }
  void start();
};

// initialize static attribute objectCount
uint8_t Ticker::objectCount = 0; 

// Ticker.start method definition
void Ticker::start() {
  cnt = timerIdx;
  switch (timerIdx) {
    case 0: {
      HardwareTimer *Timer1 = new HardwareTimer(TIM1);
      timerInit(Timer1);
      break;
    }
    case 1: {
      HardwareTimer *Timer2 = new HardwareTimer(TIM2);
      timerInit(Timer2);
      break;
    }
    case 2: {
      HardwareTimer *Timer3 = new HardwareTimer(TIM3);
      timerInit(Timer3);
      break;
    }
    case 3: {
      HardwareTimer *Timer4 = new HardwareTimer(TIM4);
      timerInit(Timer4);
      break;
    }
  }
}
class PCB_Temp {
public:
  PCB_Temp(uint8_t pin) {
    _pin = pin;
  }
  int read(){
		return resistanceToTemperature(voltageToResistance(3.3*analogRead(_pin)));
	}
private:
  const float r1 = 10000;
	const float vin = 4.35;

	const float BETA = 3430;
	const float R2 = 10000;
	const float T2 = 25 + 270;

	float _resistance;
	int _temperature;
  uint8_t _pin;

	float voltageToResistance(float vout) {
		return r1/((vin/vout)-1);
	}

	uint8_t resistanceToTemperature(float R1){
		return ((BETA * T2)/(T2 * log(R1/R2) + BETA))-270;
	}
};

PCB_Temp pcb_temperature(PA0);
//-----------------------------------------------
// Functions
//-----------------------------------------------

/*
  Provi
*/
void heartbeat() {
  heartFrame.bytes[HEART_COUNTER]++;
  can.transmit(CAN_UCM_BASE_ADDRESS+TS_HEARTBEAT_ID, heartFrame.bytes, heartFrame.len);
  //digitalToggle(PC13);
}

void canTX_lowPriority() {
  can.transmit(CAN_UCM_BASE_ADDRESS+TS_DIGITAL_1_ID, digitalFrame1.bytes, digitalFrame1.len);
  delayMicroseconds(250);
  can.transmit(CAN_UCM_BASE_ADDRESS+TS_ANALOGUE_1_ID, analogFrame1.bytes, analogFrame1.len);
}

void canTX_criticalError() {
  can.transmit(CAN_UCM_BASE_ADDRESS+TS_ERROR_WARNING_ID, errorFrame.bytes, errorFrame.len);
}

void canISR() {
  can.rxMsgLen = can.receive(can.id, can.fltIdx, can.rxData.bytes);
}

void GPIO_Init() {
    // Debug LED.
  pinMode(PC13, OUTPUT);
  
  // Configuring the Digital Input Pins.
  pinMode(pin_D1, INPUT);
  pinMode(pin_D2, INPUT);

  // Configuring Driver Pins.
  pinMode(pin_Driver1, OUTPUT);
  pinMode(pin_Driver2, OUTPUT);
  pinMode(pin_PWM_Driver1, OUTPUT);
  pinMode(pin_PWM_Driver2, OUTPUT);
}

//-----------------------------------------------
// Daemons
//-----------------------------------------------

void canRX() {
  if (can.rxMsgLen > -1) {
    if (can.id == 0x309) {
      digitalWrite(PC13,HIGH);
      rxFrame.bytes[0] = can.rxData.bytes[0];
      rxFrame.bytes[1] = can.rxData.bytes[1];
    }
  }
}

void updateDigital() {

  digitalFrame1.bytes[0] = (byte)(digitalRead(pin_D1));
  digitalFrame1.bytes[0] |= (byte)(digitalRead(pin_D2) << 1);
}

void updateAnalog() {
  // use map(value, fromLow, fromHigh, toLow, toHigh) as needed
  // use bitwise operation to split the data
  int16_t adc[4] = {0};
  for (int i = 0; i <= 3; i++) {
    adc[i] = ads.readADC_SingleEnded(i);
  }
  int16_t mapFrom[4][2]  = {{0,1023}, {0,1023}, {0,1023}, {0,1023}};
  int16_t mapTo[4][2] = {{0,100}, {0,100}, {0,500}, {0, 500}};
  for (int i =0; i <= 3; i++) {
    adc[i] = map(adc[i], mapFrom[i][0], mapFrom[i][1], mapTo[i][0], mapTo[i][1]);
  }
  // if the transmited value is to be < 255, for example, 0 -> 100%
  // analogFrame1.bytes[i] = map(adc[i], 0, 1023, 0, 100)
  // i.e. as above

  // if a higher resolution is needed, split the data into 2 bytes
  // analogFrame1.bytes[firstByte] = (byte)(adc[] & 0xFF);
  // analogFrame1.bytes[secondByte] = (byte)(adc[] >> 8);
  analogFrame1.bytes[0] = analogRead(PA0);
  analogFrame1.bytes[1] = adc[1];
  analogFrame1.bytes[2] = (byte)(adc[2] & 0xFF);
  analogFrame1.bytes[3] = (byte)(adc[2] >> 8);
  analogFrame1.bytes[4] = (byte)(adc[3] & 0xFF);
  analogFrame1.bytes[5] = (byte)(adc[3] >> 8);
}
void updateHeartdata() {
  // error warning also goes in here
  heartFrame.bytes[HEART_HARDWARE_REV] = 1;
  heartFrame.bytes[HEART_PCB_TEMP] = pcb_temperature.read();
}
void updateDrivers() {
  // logic to determine the driver goes here
  analogWrite(pin_PWM_Driver1,rxFrame.bytes[0]);
  analogWrite(pin_PWM_Driver1,rxFrame.bytes[1]);
}

void SerialPrint () {
  //Digital Input 
  Serial1.print("Digital Input 1: ");  Serial1.println(digitalRead(pin_D1)); 
  Serial1.print("Digital Input 2: ");  Serial1.println(digitalRead(pin_D2)); 
}

Ticker ticker_hearbeat          (heartbeat,           INTERVAL_HEARTBEAT);
Ticker ticker_canTX_critical    (canTX_criticalError, INTERVAL_ERROR_WARNING_CRITICAL);
Ticker ticker_canTX_lowPriority (canTX_lowPriority,   INTERVAL_ERROR_WARNING_LOW_PRIORITY);

void setup()
{
  Serial1.begin(250000);
  Serial1.print("Start");

  GPIO_Init();

  // Start I2C communication with the ADC
  ads.begin(0x49);

  // Initiallising CAN
  can.begin(STD_ID_LEN, CANBUS_FREQUENCY, PORTB_8_9_XCVR);   //11 Bit Id, 500Kbps
  can.filterMask16Init(0, 0x309, 0x7ff);
  can.attachInterrupt(canISR);

  ticker_hearbeat.start();
  delay(5);
  ticker_canTX_critical.start();
  delay(5);
  ticker_canTX_lowPriority.start();
}

void loop()
{
  canRX();
  updateHeartdata();
  updateAnalog();
  updateDigital();
  //updateDrivers();
  //SerialPrint();
}
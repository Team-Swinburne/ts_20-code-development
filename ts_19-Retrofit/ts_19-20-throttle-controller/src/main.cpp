// TEAM SWINBURNE - TS19-20 RETRO FIT
// THROTTLE CONTORL UNIT - TCU
// MICHAEL COCHRANE & PATRICK CURTAIN & NAM TRAN
// Use chrono time as a new mbed standard
//-----------------------------------------------
//Pins
//5: MOSI
//6: MISO
//7: SCK
//8: GPIO
//9: SDA/TX			- CAN1 RX
//10: SCL/RX		- CAN1 TX
//11: MOSI2
//12: MISO2
//13: SCK/TX
//14: RX
//15: ADC1
//16: ADC2 
//17: ADC3
//18: ADC4/AOut
//19: ADC5 
//20: ADC6
//21: PWM1			- RTDS (Ready to Drive Sound)
//22: PWM2
//23: PWM3			- Drive
//24: PWM4			- Precharge
//25: PWM5
//26: PWM6
//27: SCL/RX		- I2C SCI (APPS)
//28: SDA/TX		- I2C SDA (APPS)
//29: CANTX			- CAN2 TX
//30: CANRX			- CAN2 RX
//-----------------------------------------------
#include <mbed.h>
#include <CAN.h>
#include <CAN_devices.h>
#include <CAN_config.h>
#include <analog_devices.h>
#include <digital_devices.h>

CAN 			    	  can1(p30, p29);
DigitalOut				can1_rx_led(LED1);
DigitalOut 				can1_tx_led(LED2); 
CANMessage 				can1_msg;

// Intervals
#define 				THROTTLE_HEARTRATE			  1s			// 1 Hz		
#define 				CAN_BROADCAST_INTERVAL		5ms		// 200 Hz

enum {
  THROTTLE_STATE_ERROR,
  THROTTLE_STATE_IDLE,
  THROTTLE_STATE_CAR_GO_BRUHHH,
};

//Objects 
Heart heart(LED3);
Button drive_button(p23);
Button precharge_button(p24);
RTDS rtds(p21);
Brake brakes;
APPS apps(p28, p27);
MotorController motor;
AMS ams;

Ticker          ticker_heartbeat;
Ticker          ticker_buttons;
Ticker          ticker_can_transmit;

//-----------------------------------------------
void checkButtons_cb() {
  drive_button.update_value(); // change to update after 
  precharge_button.update_value();
}

// Implement 25% Rule, Trailbraking
void check_trailbraking() {
  //EV.2.4 APPS / Brake Pedal Plausibility Check
	//EV.2.4.1 The power to the motors must be immediately shut down completely, if the mechanical
	//brakes are actuated and the APPS signals more than 25% pedal travel at the same time.
	//Cannot turn back on unless throttle below 5%
  // turn on trail braking
  if ((apps.get_avg_percent() >= TRAILBRAKING_ACTIVE_PERCENT) && brakes.isActive()) {
    apps.set_error_trailbraking(true);
  }

  // reset
  if ((apps.get_avg_percent() <= TRAILBRAKING_RESET_PERCENT) && !brakes.isActive()) {
    apps.set_error_trailbraking(false);
  }
}
// -----------------------------------------------------------------
// CAN Function
// -----------------------------------------------------------------

//Heartbeat - Broadcasts that the throttle is still alive and functional with a counter so we know when it has failed if it does
void heartbeat_cb(){
  can1_tx_led = !can1_tx_led;
  heart.update_counter();

	char TX_data[2];
  int dlc = 2;
  TX_data[0] = 1;//(char)heart.get_heartbeat_state();
  TX_data[1] = 2;//(char)heart.get_heartbeat_counter();

	if (can1.write(CANMessage(THROTTLE_HEARTBEAT_ID, &TX_data[0], dlc))) {
    //can1_tx_led = !can1_tx_led;
  }
}

void canTX_cb(){
  can1_tx_led = !can1_tx_led;

  // Broadcast devices state
  char TX_data[8] = {0};
  int dlc = 6;
  TX_data[0] = rtds.get_sounded();
	TX_data[1] = precharge_button.isPressed() && brakes.isActive();
	TX_data[2] = drive_button.isPressed();
	TX_data[3] = apps.get_avg_percent();
  //New stuff added
  TX_data[4] = precharge_button.isPressed();
  TX_data[5] = brakes.isActive();
  can1.write(CANMessage(THROTTLE_OUTPUT_ID, &TX_data[0], dlc)); 

  // Broadcast error states
  dlc = 2;
  TX_data[0] = apps.get_error_trailbraking();
	TX_data[1] = apps.check_apps_disagree();
	can1.write(CANMessage(THROTTLE_ERRORS_ID, &TX_data[0], dlc));

  // Broadcast Motor Setting
  dlc = 6;
  TX_data[0] = motor.get_motor_torque() & 0xFF; //Torque lower byte. value for [0] and [1] is the desired N.m times 10. e.g. for 240Nm send the value 2400
  TX_data[1] = motor.get_motor_torque() >> 8;   //Torque upper byte
  TX_data[2] = 0;                               //Speed lower byte. Not in use because we're using torque
  TX_data[3] = 0;                               //Speed upper byte
  TX_data[4] = motor.get_motor_direction();     //Direction. 0 is clockwise, 1 is anti clockwise
  TX_data[5] = motor.get_inverter_power();      //Inverter. 0 is off, 1 is on
}

// Receive CAN data
void canRX_cb(){
  can1_rx_led = !can1_rx_led;
  if (can1.read(can1_msg)) {
    switch (can1_msg.id) {
    
    // Receive Brake Pressure from CAN (Brake Module)
    case BRAKE_SENSORS_ID:
      brakes.set_pressure(can1_msg.data[0],can1_msg.data[1]);
      break;

    // Receive AMS State
    case AMS_DATA_ID:
      //ams.connect();
      ams.set_ams_state(can1_msg.data[0]);
      //ams.set_critical_error(can1_msg.data[1]);
      break;
    }
  }
}

bool check_crittical_error() {
  return apps.get_error_trailbraking() || apps.check_apps_disagree();
}

void update_throttle() {
  apps.update_apps(0,0x30); //Update apps 1
  apps.update_apps(1,0x10); //Update apps 2
  check_trailbraking(); // check if trailbraking is active

}

//state machine 
void state_d() {
  update_throttle();

  if (check_crittical_error() || (ams.get_ams_state() == 0)) {
    heart.set_heartbeat_state(THROTTLE_STATE_ERROR);
  }

  switch (heart.get_heartbeat_state()) {
    case THROTTLE_STATE_ERROR:
      // turn off the motor
      motor.set_motor_torque(0); 
      motor.set_motor_direction(0);
      motor.set_inverter_power(0);
      
      if (!check_crittical_error()) {
        heart.set_heartbeat_state(THROTTLE_STATE_IDLE);
      }
      break;

    case THROTTLE_STATE_IDLE:
      // don't do anything to the motor
      // move to drive state if ams state = 5
      if (ams.get_ams_state() == 5) {
        heart.set_heartbeat_state(THROTTLE_STATE_CAR_GO_BRUHHH);
      }
      break;
      // brake + drive button is handled in the transmit function, NOT HERE

    case THROTTLE_STATE_CAR_GO_BRUHHH:
      if (!rtds.get_sounded()) {
        rtds.activate(1500ms);
        rtds.set_sounded(true);
      }

      //spin motor
      motor.set_motor_torque(apps.get_avg_percent()); 
      motor.set_motor_direction(1);
      motor.set_inverter_power(1);
      break;
      
  }
}
//setup on power
void setup() {
  can1_rx_led = 1;
  can1_tx_led = 1;
  wait_us(1000000);
  //__disable_irq(); // disable for smooth startup routine 
  can1.frequency(500000);
  //can1.attach(&canRX_cb);

  ticker_heartbeat.attach(&heartbeat_cb, THROTTLE_HEARTRATE);
  //ticker_buttons.attach(&checkButtons_cb,BUTTON_CHECK_INTERVAL_CHRONO);
  ticker_can_transmit.attach(&canTX_cb, CAN_BROADCAST_INTERVAL);

	//__enable_irq(); // Re-enable interrupts again, now that interrupts are ready.

  //rtds.activate(100ms); //indicate throttle finished setup
}

int main() {
  setup();

  while(1) {
    //state_d();
  }

  return 0;
}
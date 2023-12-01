#include <mbed.h>
#include <CAN.h>

#define 				MAX_TORQUE_LIMIT 			700 		// Factor of 10 so 2400 for 240Nm
// Classes for the objects to be transmitted on CAN

//Heartbeat
class Heart {
public:
    /** Heart Contructor
     * 
     * @param heart_led_pin
     * 
     */
    Heart(PinName heart_led_pin) : heart_LED(heart_led_pin){
        heartbeat_state = 0;
        heartbeat_counter = 0;
    }

    void    update_counter() {heartbeat_counter++;}
    int     get_heartbeat_counter() {return heartbeat_counter;}
    int8_t  get_heartbeat_state() {return heartbeat_state;}

    void    set_heartbeat_state(int16_t _heartbeat_state) {heartbeat_state = _heartbeat_state;}

private:
    int8_t  heartbeat_state;
    int     heartbeat_counter;

    DigitalOut heart_LED;
};

/* Brake receives brake percent from the Brake Module through CANBUS and save that value for trail braking checking
*/
class Brake {
public:
	// init
	Brake() {
		brake_value[0] = 0;
        brake_value[1] = 0;
	}
	
    /** Set value for brake 1 and 2 
     * 
     * @param _brake_value1 value of brake 1
     * @param _brake_value2 value of brake 2
     */
    void set_pressure(int16_t _brake_value1,int16_t _brake_value2) {
        brake_value[0] = _brake_value1;
        brake_value[1] = _brake_value2;
    }

    /** Get brake value
     * 
     * @param _brake_idx index of which brake to retrieve value from, start with 0
     */ 
    int16_t get_pressure(int16_t _brake_idx) {
        return brake_value[_brake_idx];
    }

    /** @return true if brake is active
     */
    bool isActive() {
        if ((brake_value[0] > 5) || (brake_value[1] > 5)) {
            return true;
        }
        else {
            return false;
        }
    }
private:
    int16_t brake_value[2];
};

class MotorController {
public:
    MotorController() {
        motor_direction = 0x00;
        inverter_power = 0x00;
        motor_torque = 0;
    }

    void set_motor_direction(char _motor_dir) {motor_direction = _motor_dir;}
    void set_inverter_power(char _inverter_pwr) {inverter_power = _inverter_pwr;}
    void set_motor_torque (float apps_percent) {motor_torque = (apps_percent * 0.01) * MAX_TORQUE_LIMIT;}

    char get_motor_direction() {return motor_direction;}
    char get_inverter_power() {return inverter_power;}
    int16_t get_motor_torque() {return motor_torque;}

private:
    char motor_direction;
    char inverter_power;
    int16_t motor_torque;
};

class AMS {
public:
    AMS() {
        ams_state = 0;
    }

    void set_ams_state(int16_t _ams_state) {ams_state = _ams_state;}
    int16_t get_ams_state() {return ams_state;}
private:
    int16_t ams_state;
};

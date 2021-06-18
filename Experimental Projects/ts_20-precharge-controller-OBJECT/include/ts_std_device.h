#include <mbed.h>
#include <can.h>

#define MAX_PCB_TEMPERATURE 60

class PCB_Temp_Sensor {
public:
	PCB_Temp_Sensor(PinName pin) : _sensor(pin){
		_temperature = _sensor.read();
	}

	int read(){
		return resistanceToTemperature(voltageToResistance(3.3*_sensor.read()));
	}

	bool pcb_temperature_ok(){
		if (PCB_Temp_Sensor::read() < MAX_PCB_TEMPERATURE){
			return true;
		} else {
			return false;
		}
	}

private:
	int pcb_temperature = 0;

	const float r1 = 10000;
	const float vin = 5;

	const float BETA = 3430;
	const float R2 = 10000;
	const float T2 = 25 + 270;

	float _resistance;
	int _temperature;
	AnalogIn _sensor;

	float voltageToResistance(float vout){
		return r1/((vin/vout)-1);
	}

	int resistanceToTemperature(float R1){
		return ((BETA * T2)/(T2 * log(R1/R2) + BETA))-270;
	}
};

class Heart {
public:
	Heart(int _heartbeat_ID, PinName heart_led_pin) : Heart_LED(heart_led_pin), pcb_temperature(NC){
		heartbeat_state = 0;
		heartbeat_counter = 0;
		heartbeat_ID = _heartbeat_ID;
	}

	Heart(int _heartbeat_ID, PinName heart_led_pin, PinName pcb_temperature_pin) : Heart_LED(heart_led_pin), pcb_temperature(pcb_temperature_pin){
		heartbeat_state = 0;
		heartbeat_counter = 0;
		heartbeat_ID = _heartbeat_ID;
	}

	// Returns type CANMessage for output to be handled by the timer and message handlers.
	CANMessage heartbeat(){
		heartbeat_counter++;
		!Heart_LED;

		const int dlc = 3;
		char TX_data[dlc] = {(char)heartbeat_state, (char)heartbeat_counter, (char)pcb_temperature.read()};
		return CANMessage(heartbeat_ID, &TX_data[0], dlc);
	}

	void set_heartbeat_state(int _heartbeat_state){heartbeat_state = _heartbeat_state;}
	int get_heartbeat_state() {return heartbeat_state;}
	int get_heartbeat_counter() {return heartbeat_counter;}

	PCB_Temp_Sensor pcb_temperature;

private:
	int heartbeat_state;
	int heartbeat_counter;
	int heartbeat_ID;

	DigitalOut Heart_LED;
};
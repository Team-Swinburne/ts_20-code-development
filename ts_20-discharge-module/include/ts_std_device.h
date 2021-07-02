#include <mbed.h>
// #include <can.h>

#define MAX_PCB_TEMPERATURE 60

	/*
PCB_Temp_Sensor
	Generic 10k NTC thermistor attached to PCB for telemetry puroses.
	*/
class PCB_Temp_Sensor {
public:
	/** PCB_Temp_Sensor Constructor
     *
	 * @param sensor_pin position of the measurement circuit.
	 * 
     */
	PCB_Temp_Sensor(PinName sensor_pin) : _sensor(sensor_pin){
		_temperature = _sensor.read();
	}

	/** read()
     * 
	 * Converts from voltage to resistance, then resistance to temperature.
	 * 
	 * @return sensor_temperature
	 *  
     */
	int read(){
		return resistanceToTemperature(voltageToResistance(3.3*_sensor.read()));
	}

	/** pcb_temperature_ok()
	 * 
	 * @return pcb_temperature_ok whether the tempearture is below the 
	 * MAX_PCB_TEMPERATURE.
	 *  
     */
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

	/*
Heart
	The Heartbeat is a 1 second ticker that returns the current state of 
	the device. This must be common across all TS devices. The timing for
	this object must be provided by an external source, such that it 
	can interract with the devices CANBUS. 

----------------------------------------------------------------

Use this generic tempalte for how to setup the Heart.

void heartbeat_cb(){
    if (can_transmission_handler(heart.heartbeat())){
        pc.printf("Heartbeat Success! State: %d Counter: %d\r\n", heart.get_heartbeat_state(), heart.get_heartbeat_counter());
    } else {
        pc.printf("Hearts dead :(\r\n");
    }
}

...

Heart heart(@CAN_ADDRESS, @LED1, @TEMP_SENSOR);
Ticker ticker_heartbeat;

...

ticker_heartbeat.attach(&heartbeat_cb, HEARTRATE);

----------------------------------------------------------------

	Within the heart exists a set of generic error codes. These are 
	to be assigned with the use of an enumeration. An example is included 
	below.

 ----------------------------------------------------------------
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

...

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

...

uint8_t errord(){
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

uint8_t warnd(){
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

...

heart.set_error_code(errord(), 0);
heart.set_warning_code(warnd(), 0);

	*/
class Heart {
public:
	/** Heart Constructor
     * 
	 * @param heartbeat_ID is the root address of the device on the CANBUS. 
	 * @param heat_led_pin is the location of LED1 on the device, which will
	 * flash at a 1 Hz period.
	 * 
     */
	Heart(int _heartbeat_ID, PinName heart_led_pin) : heart_LED(heart_led_pin), pcb_temperature(NC){
		heartbeat_state = 0;
		heartbeat_counter = 0;
		heartbeat_ID = _heartbeat_ID;
	}

	/** Heart Constructor
     * 
	 * @param heartbeat_ID is the root address of the device on the CANBUS. 
	 * @param heat_led_pin is the location of LED1 on the device, which will
	 * flash at a 1 Hz period.
	 * @param pcb_temperature_pin is the location of the temperature sensor. Inherrits
	 * temperature object from PCB_Temp_Sensor.
	 * 
     */
	Heart(int _heartbeat_ID, PinName heart_led_pin, PinName pcb_temperature_pin) : heart_LED(heart_led_pin), pcb_temperature(pcb_temperature_pin){
		heartbeat_state = 0;
		heartbeat_counter = 0;
		heartbeat_ID = _heartbeat_ID;
	}

	/** heartbeat()
	 * 
	 * Associates all the required information into a CANBUS dataframe ready 
	 * for transmisssion.
     * 
	 * @returns CANMessage to be transmitted showing the state of the
	 * device.
	 * 
     */
	CANMessage heartbeat(){
		heartbeat_counter++;
		heart_LED = !heart_LED;

		const int dlc = 3;
		char TX_data[dlc] = {(char)heartbeat_state, (char)heartbeat_counter, (char)pcb_temperature.read()};
		return CANMessage(heartbeat_ID, &TX_data[0], dlc);
	}

	/** set_error_code(error_code, index)
	 * 
	 * Assigns the error code to the array, assigns heartbeat_state to 
	 * 0 if an error is found.
	 * 
	 * @param error_code The code of the errors that may be present.
	 * @param i Index of the error. THERE ARE ONLY TWO POSITIONS, DO NOT
	 * OVERFLOW!
	 * 
     * 
	 * @returns error_present if an error is present, this will return true.
	 * 
     */
	bool set_error_code(uint8_t _error_code, int i){
		error_code[i] = _error_code;
		if (error_code[i] > 0){
			// heartbeat_state = 0;
			return true;
		} else {
			return false;
		}
	}

	/** set_warning code(warning_code, index)
	 * 
	 * Assigns the error code to the array, assigns heartbeat_state to 
	 * 0 if an error is found.
	 * 
	 * @param warning_code The code of the errors that may be present.
	 * @param i Index of the error. THERE ARE ONLY TWO POSITIONS, DO NOT
	 * OVERFLOW!
	 * 
     */
	bool set_warning_code(uint8_t _warning_code, int i){
		warning_code[i] = _warning_code;
		if (warning_code[i] > 0) {
			return true;
		} else {
			return false;
		}
	}

	void set_heartbeat_state(int _heartbeat_state){heartbeat_state = _heartbeat_state;}
	int get_heartbeat_state() {return heartbeat_state;}
	int get_heartbeat_counter() {return heartbeat_counter;}
	uint8_t get_error_code(int i){return error_code[i];}
	uint8_t get_warning_code(int i){return warning_code[i];}

	PCB_Temp_Sensor pcb_temperature;

private:
	int heartbeat_state;
	int heartbeat_counter;
	int heartbeat_ID;

	uint8_t error_code[2];
	uint8_t warning_code[2];

	DigitalOut heart_LED;
};
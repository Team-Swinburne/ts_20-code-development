#include <mbed.h>

#define ORION_TIMEOUT_INTERVAL 200

// ABSOLUTE MAXIMUMS - Not preferences, set preferences within the Orion. 
#define MINIMUM_CELL_VOLTAGE 2400
#define MAXIMUM_CELL_VOLTAGE 4300
#define MAXIMUM_CELL_TEMPERATURE 80

	/*
Discharge Module
	Discharge Module is a slave of the precharge controller used to make the
	vehicle safe to work on. When the first contactor closes, the discharge relay
	must open the connection of the 4kR resistor normally connected across HV_MC- and
	HV_MC+. A microcontroller has been included for data acquisition purposes and to 
	generate warning messages in case that both precharge and discharge resistors are 
	present, creating a voltage divider and failing the precharge cycle.
	*/
class Discharge_Module {
public:
	/** Discharge_Module Constructor
     *
	 * Initialise discharge state to open to avoid generating error messages.
	 * 
     */
	Discharge_Module(){
		discharge_state = 2;
	}

	/** check_precharge_discharge_mismatch()
	 * 
	 * Check if the internal state of the discharge is valid. 
	 * 
	 *	@param precharge_state Current state of the precharge.
     *
     *  @returns boolean to show that discharge is functioning correctly.
     */
	bool check_precharge_discharge_mismatch(int precharge_state){
		if (precharge_state > 1 && discharge_state == 0){
			return true;
		} else {
			return false;
		}
	}

	void set_discharge_state(int _discharge_state){discharge_state = _discharge_state;}
	int get_discharge_state(){return discharge_state;}

private:
	int discharge_state;
};

	/*
Orion
	Orion Battery Management System. The Orion is used to check the health of the 
	battery pack. The Orion manages fault handling of the accumulator, with
	the precharge controller simply being a convenient place to keep the relay.
	A Watchdog must be used and kicked on each connection of the Orion on the CANBUS.
	*/
class Orion {
public:
	/** Orion Constructor
     *
	 * Initialise Orion state with 0 for all values. Orion must prove that it is in a valid
	 * state in order for the errors to clear.
	 * 
	 * @param AMS_ok_pin, location of the AMS_ok relay.
	 * 
     */
	Orion(PinName AMS_ok_pin) : AMS_ok(AMS_ok_pin) {
		orion_connection_state = false;
		relay_status = 0;
		low_voltage = 0;
		high_voltage = 0;
		high_temperature = 0;
	}

	/** connect_orion()
     *
	 * Kick for the Orions Watchdog timer. Detaches previous function call and resets
	 * count down timer.
	 * 
     */
	void connect_orion(){
		orion_timeout.detach();
		orion_connection_state = true;
		orion_timeout.attach(callback(this, &disconnect_orion_cb), ORION_TIMEOUT_INTERVAL);
		if (check_orion_safe()){
			AMS_ok = 1;
		} else {
			AMS_ok = 0;
		}
	}

	/** connect_orion(_relay_status)
     *
	 * Kick for the Orion's Watchdog timeout. Detaches previous function call and resets
	 * count down timer.
	 * 
	 * @param _relay_status for the current status of the relay, if the relay is in a bad state,
	 * will immediately trigger the AMS_ok relay to enter a fail state, halting the vehicle.
	 * 
     */
	void connect_orion(uint16_t _relay_status){
		relay_status = _relay_status;
		orion_timeout.detach();
		orion_connection_state = true;
		orion_timeout.attach(callback(this, &disconnect_orion_cb), ORION_TIMEOUT_INTERVAL);
		if (check_orion_safe() || relay_status > 0){
			AMS_ok = 1;
		} else {
			AMS_ok = 0;
		}
	}

	/** disconnect_orion()
     *
	 * Callback to disable the relay and change connection status flag. Disables vehicle.
	 * 
     */
	void disconnect_orion_cb(){
		orion_connection_state = false;
		AMS_ok = 0;
	}

	/** check_orion_safe()
     *
	 * Checks if the low, high, or overtemperature values are within range.
	 * 
	 * @returns whether the Orion is managing it's relay correctly.
	 * 
     */
	bool check_orion_safe(){
		if (low_voltage < MINIMUM_CELL_VOLTAGE){
			return false;
		} else if (high_voltage > MAXIMUM_CELL_VOLTAGE){
			return false;
		} else if (high_temperature > MAXIMUM_CELL_TEMPERATURE){
			return false;
		} else {
			return true;
		}
	}

	void set_AMS_ok(bool _AMS_ok){AMS_ok = _AMS_ok;}
	void set_low_voltage(int _low_voltage){low_voltage = _low_voltage;}
	void set_high_voltage(int _high_voltage){low_voltage = _high_voltage;}
	void set_high_temperature(int _high_temperature){high_temperature = _high_temperature;}

	bool get_AMS_ok(){return AMS_ok;}
	int get_low_voltage(){return low_voltage;}
	int get_high_voltage(){return high_voltage;}
	int get_high_temperature(){return high_temperature;}

private:
	DigitalOut AMS_ok;

	bool orion_connection_state;

	uint8_t relay_status;

	int low_voltage;
	int high_voltage;
	int high_temperature;

	Timeout orion_timeout;
};

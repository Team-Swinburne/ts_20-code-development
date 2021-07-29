#include <mbed.h>

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
		orion_timeout.attach(callback(this, &Orion::disconnect_orion_cb), ORION_TIMEOUT_INTERVAL);
		AMS_ok_status = !check_orion_safe();
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
		orion_timeout.attach(callback(this, &Orion::disconnect_orion_cb), ORION_TIMEOUT_INTERVAL);
		AMS_ok_status = !check_orion_safe();
	}

	/** disconnect_orion()
     *
	 * Callback to disable the relay and change connection status flag. Disables vehicle.
	 * 
     */
	void disconnect_orion_cb(){
		orion_connection_state = false;
		AMS_ok_status = 0;
	}

	/** check_orion_state()
     *
	 * Checks if the low, high, or overtemperature values are within range.
	 * 
	 * @returns whether the Orion is managing it's relay correctly.
	 * 
     */
	bool check_orion_state(){
		if (relay_status > 0){
			return true;
		}
		return false;
	}

	/** check_low_voltage()
     *
	 * Checks if the low, high, or overtemperature values are within range.
	 * 
	 * @returns True on fail.
	 * 
     */
	bool check_low_voltage(){
		if (low_voltage > MINIMUM_CELL_VOLTAGE){
			return false;
		}
		return true;
	}

	/** check_high_voltage()
     *
	 * Checks if the low, high, or overtemperature values are within range.
	 * 
	 * @returns True on fail.
	 * 
     */
	bool check_high_voltage(){
		if (high_voltage > MAXIMUM_CELL_VOLTAGE){
			return true;
		}
		return false;
	}

	/** check_high_temperature()
     *
	 * Checks if the low, high, or overtemperature values are within range.
	 * 
	 * @returns True on fail.
	 * 
     */
	bool check_overtemperature(){
		if (high_temperature > MAXIMUM_CELL_TEMPERATURE){
			return true;
		}
		return false;
	}

	void set_AMS_ok(bool _AMS_ok){AMS_ok = _AMS_ok;}
	void set_low_voltage(int _low_voltage){low_voltage = _low_voltage;}
	void set_high_voltage(int _high_voltage){high_voltage = _high_voltage;}
	void set_high_temperature(int _high_temperature){high_temperature = _high_temperature;}

	bool get_AMS_ok(){return AMS_ok_status;}
	int get_low_voltage(){return low_voltage;}
	int get_high_voltage(){return high_voltage;}
	int get_high_temperature(){return high_temperature;}

private:
	DigitalOut AMS_ok;

	bool orion_connection_state;
	bool AMS_ok_status;

	uint8_t relay_status;

	int low_voltage;
	int high_voltage;
	int high_temperature;

	Timeout orion_timeout;

	/** check_orion_safe()
     *
	 * Checks if the relay states, low, high, or overtemperature values are within range.
	 * 
	 * @returns True on fail.
	 * 
     */
	bool check_orion_safe(){
		if (check_orion_state() || check_low_voltage() || check_high_voltage() || check_overtemperature()){
			return true;
		}
		return false;
	}
};

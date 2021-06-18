#include <mbed.h>
// #include ".\ts_std_device.h"

#define ORION_TIMEOUT_INTERVAL 200

class Discharge_Module {
public:
	Discharge_Module(){
		discharge_state = 2;
	}

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

class Orion {
public:
	Orion(){
		last_connection = 0;

		discharge_ok = false;
		charger_ok = false;
		charger_safety = false;

		low_voltage = 0;
		high_voltage = 0;
		high_temperature = 0;
	}

	void set_low_voltage(int _low_voltage){low_voltage = _low_voltage;}
	void set_high_voltage(int _high_voltage){low_voltage = _high_voltage;}
	void set_high_temperature(int _high_temperature){high_temperature = _high_temperature;}

	int get_low_voltage(){return low_voltage;}
	int get_high_voltage(){return high_voltage;}
	int get_high_temperature(){return high_temperature;}

	bool connect_orion(){
		last_connection = current_time;
	}

	bool orion_connected(){
		bool orion_connection_state = false;
		if (current_time < last_connection + ORION_TIMEOUT_INTERVAL){
			orion_connection_state = true;
		} else {
			orion_connection_state = false;
		}
		return orion_connection_state;
	}

	bool get_orion_safe_status(){
		if (discharge_ok || charger_ok || charger_safety){
			return true;
		}
		else {
			return false;
		}
	}

private:

	// THIS NEEDS TO BE MOVED OUT!

	int current_time;

	int last_connection;

	bool orion_safety;

	bool discharge_ok;
	bool charger_ok;
	bool charger_safety;

	int low_voltage;
	int high_voltage;
	int high_temperature;
};

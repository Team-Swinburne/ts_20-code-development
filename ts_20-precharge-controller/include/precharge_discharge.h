#include <mbed.h>

#define ORION_TIMEOUT_INTERVAL 3		// 500 ms or 3s?

// ABSOLUTE MAXIMUMS - Not preferences, set preferences within the Orion. 
#define MINIMUM_CELL_VOLTAGE 2400		// 2400mV
#define MAXIMUM_CELL_VOLTAGE 4300		// 4300mV
#define MAXIMUM_CELL_TEMPERATURE 80		// 80 C

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

	/** precharge_discharge_mismatch()
	 * 
	 * Check if the internal state of the discharge is invalid. 
	 * 
	 *	@param precharge_state Current state of the precharge.
     *
     *  @returns true if precharge matches discharge.
     */
	bool precharge_discharge_mismatch(){
		if (precharge_state > 1 && discharge_state == 1){
			return true;
		} else {
			return false;
		}
	}

	void set_precharge_discharge_state(int _precharge_state, int _discharge_state)
		{precharge_state = _precharge_state; discharge_state = _discharge_state;}
	int get_discharge_state(){return discharge_state;}

private:
	int discharge_state;
	int precharge_state;
};

	/*
Precharge Controller
	The precharge controller is the master of the discharge controller.
	When the first contactor closes, the discharge relay must open the connection of the 4kR 
	resistor normally connected across HV_MC- and HV_MC+. A microcontroller has been included 
	for data acquisition purposes and to generate warning messages in case that both precharge 
	and discharge resistors are present, creating a voltage divider and failing the precharge cycle.
	*/
typedef Discharge_Module Precharge_Controller;
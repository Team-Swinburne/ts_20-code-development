// Analog devices on the Throttle

#include <mbed.h>

#define					INVERT_APPS_INPUT			true
#define 				TRAILBRAKING_ACTIVE_PERCENT	25 		// % To disable power if both pedals pressed
#define 				TRAILBRAKING_RESET_PERCENT	5 		// % To reenable power if 25% rule broken
#define 				DEADZONE        			15 			// 15% Pedal deadzone
#define 				APPS_ADC_ADDR				0x68 		// Address of the APPS MCP3428

#define					APPS_1_MIN					1086
#define					APPS_2_MIN					1086
#define					APPS_1_MAX					1624
#define					APPS_2_MAX					1624

/*
Throttle collects and process data from APPS (or Acceleration Plausibility Position Sensor) and proccess them into Torque command for the motor controller. 
It also checks for plausibility and other fail-safe function in accordance to comp rules
*/
class APPS {
public:
    /** Throttle constructor
     * 
     * @brief Initialize Throttle with 2 I2C Pins where the MCP3428 connects. Set all value to 0
     * 
     * @param _pin_sda SDA Pin
     * @param _pin_scl SCL Pin
     */
    APPS (PinName _pin_sda, PinName _pin_scl) : apps(_pin_sda,_pin_scl) {
     	apps_min[0] = APPS_1_MIN;
     	apps_min[1] = APPS_2_MIN;
     	apps_max[0] = APPS_1_MAX;
     	apps_max[1] = APPS_2_MAX;
     	   
        apps_output[0] = 0;
        apps_output_percent[0]= 0;
        apps_output[1] = 0;
        apps_output_percent[1]= 0;
        error_trail_braking = false;
    }

    /** Update apps values and percentage
     * 
     * @param idx APPS Index
     * @param channel data to send to the ADC to choose which channel to read form
     */
    void 
    update_apps(int16_t idx, char channel) {
        char cmd[1] = {channel};
        char data[2];

        apps.write(APPS_ADC_ADDR << 1, cmd, 1);
        if(!apps.read(APPS_ADC_ADDR << 1, data, 2)) {

		    apps_output[idx] = (int16_t)((data[0] << 8) | data[1]);
		    apps_output_percent[idx] = calculate_percent(apps_output[idx],apps_min[idx],apps_max[idx]);

            // Invert output if neccesary
		    if(INVERT_APPS_INPUT) {
			    apps_output_percent[idx] = 100 - apps_output_percent[idx];
		    }
            // Limit the apps percent from 0 -> 100% after accounting for Deadzone
            apps_output_percent[idx] = limit_apps(apps_output_percent[idx]); 
	    }
    }

    /** Check if APPPS disagree
    */
    bool check_apps_disagree() {
    // Implement 10% Rule, Compares Throttle Pedals for 10% difference
	//T.6.2.2 At least two entirely separate sensors have to be used as APPSs. The sensors must have different transfer functions which meet either:
	// Each sensor has a positive slope sense with either different gradients and/or offsets to the other(s).
	// An OEM pedal sensor with opposite slopes. Non OEM opposite slope sensor configurations require prior approval.
	//The intent is that in a short circuit the APPSs will only agree at 0% pedal position.
	//T.6.2.3 Implausibility is defined as a deviation of more than 10% pedal travel between the sensors or
	//other failure as defined in this Section T.6.2.
        if(     abs(floor(apps_output_percent[0] - apps_output_percent[1])) > 10 
            || (apps_output[0] < (apps_min[0] - 300)) 
            || (apps_output[1] < (apps_min[1]  - 300)) 
            || (apps_output[0] > (apps_max[0] + 300)) 
            || (apps_output[1] > (apps_max[1] + 300))) //|| (apps_output_1 < 5) || (apps_output_2 < 5)) 
        {
            return true;
        }
        else {
            return false;
        }
    }

    bool get_error_trailbraking() {return error_trail_braking;}
    float get_avg_percent() {return (apps_output_percent[0] + apps_output_percent[1]) / 2;}
    float get_apps_percent(int16_t _apps_idx) {return apps_output_percent[_apps_idx];}
    int get_apps_raw(int16_t _apps_idx) {return apps_output[_apps_idx];}

    void set_error_trailbraking(bool _error_trail_braking) {error_trail_braking = _error_trail_braking;}

private:
    I2C apps;

    bool error_trail_braking;
    int16_t apps_output[2];
    float apps_output_percent[2];

    int16_t   apps_min[2]; //APPS 1 and 2 min
    int16_t   apps_max[2]; //APPS 1 and 2 max


    // calculate percent from value, min and max
    float calculate_percent(int16_t value, int16_t min_value, int16_t max_value) {
        return ((float)(value - min_value) / (float)(max_value - min_value)) * 100;
    }

    // make APPS percentage is between DEADZONE and 100%
    float limit_apps(float apps_percent) {
        if(apps_percent < DEADZONE) {
			apps_percent = 0;
		}
        else {
			apps_percent = ((apps_percent - DEADZONE) / (100- DEADZONE)) * 100;
		}

		if(apps_percent > 100) {
			apps_percent = 100;
		}
        return apps_percent;
    }
};

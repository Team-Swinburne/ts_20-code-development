#include <mbed.h>

class PDOC{
public:
	PDOC(I2C &i2c1, int _adc_addr) : pdoc_adc(&i2c1, _adc_addr){};

	bool update_adc(){
		pdoc_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(0), 32768, 6.144), 3380) + 50;
		// pc.printf("PDOC_TEMPERAUTRE: %d \r\n", pdoc_temperature);
		pdoc_ref_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(1), 32768, 6.144), 3380);
		// pc.printf("PDOC_REF_TEMPERAUTRE: %d \r\n", pdoc_ref_temperature);
		return sensor_ok();
	}

	bool pdoc_ok(){
		bool _pdoc_ok = false;
		if (pdoc_temperature < pdoc_ref_temperature){
			_pdoc_ok = true;
		}
		return _pdoc_ok;
	}

	bool sensor_ok(){
		bool sensor_ok = false;
		if (pdoc_temperature < OVER_TEMPERATURE){
			sensor_ok = true;
		} else {
            sensor_ok = false;
        }
        
		if (pdoc_temperature > UNDER_TEMPERATURE){
			sensor_ok = true;
		} else {
            sensor_ok = false;
        }
		return sensor_ok;
	}

	int get_pdoc_temperature(){return pdoc_temperature;}
	int get_pdoc_ref_temperature(){return pdoc_ref_temperature;}
	

private:
	const int OVER_TEMPERATURE = 250; 
	const int UNDER_TEMPERATURE = -40;

	int pdoc_temperature;
	int pdoc_ref_temperature;

	Adafruit_ADS1115 pdoc_adc;

    float adc_to_voltage(int adc_value, int adc_resolution, float voltage_range){
	    float voltage = adc_value * voltage_range / adc_resolution;
	    return voltage;
    }

    int NTC_voltageToTemperature(float voltage, float BETA=3380){
        float r1 = 2000;
        float vin = 5;
        
        float R2 = 10000;
        float T2 = 25 + 270;

        float resistance;
        int temperature;
        
        resistance = r1/((vin/voltage)-1);
        temperature = ((BETA * T2)/(T2 * log(resistance/R2) + BETA))-270;
        
        return temperature;
    }
};

class HV_ADC{
public:
	HV_ADC(I2C &_i2c1, int _adc_addr, float _R_CAL) : high_voltage_adc(&_i2c1, _adc_addr){
        R_CAL = _R_CAL;
    };

	bool update_adc(){
		voltage = HV_voltageScaling(adc_to_voltage(high_voltage_adc.readADC_SingleEnded(0), 32768, 6.144));
	    // pc.printf("MC_VOLTAGE: %d \r\n", mc_voltage);
	    return sensor_ok(0);
	}

	bool sensor_ok(bool enable_uv){
		bool sensor_ok = false;
		if (voltage < OVER_VOLTAGE){
			sensor_ok = true;
		} else {
            sensor_ok = false;
        }

        if (enable_uv){
            if (voltage > UNDER_VOLTAGE){
                sensor_ok = true;
            } else {
                sensor_ok = false;
            }
        }
		return sensor_ok;
	}

	int get_voltage(){return voltage;}

private:
	const int OVER_VOLTAGE = 600; 
	const int UNDER_VOLTAGE = 250;

    float R_CAL;

	int voltage;

	Adafruit_ADS1115 high_voltage_adc;

    float adc_to_voltage(int adc_value, int adc_resolution, float voltage_range){
	    float voltage = adc_value * voltage_range / adc_resolution;
	    return voltage;
    }

    int HV_voltageScaling(float input){
        float R1 = 330000 * 4;
        float R2 = 10000 + R_CAL;

        float R1_R2 = R1 + R2;
        float scaling_factor = (R2 / R1_R2);
        float output = input / scaling_factor;
        
        int _output = output;

        return _output;
    }
};

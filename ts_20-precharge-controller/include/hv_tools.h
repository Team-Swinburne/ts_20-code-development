#include <mbed.h>

// HV Voltage Bridge Offset Resistors
#define MC_R_CAL 						5000
#define BATT_R_CAL 						5000
#define MINIMUM_PRECHARGE_VOLTAGE		400
#define MAXIMUM_PRECHARGE_VOLTAGE		600

// ADS1115 ADDR PINS: GND 48 - VDD 49 - SDA 4A - SCL 4B
#define PDOC_ADC_ADDR				0x4B
#define MC_HV_SENSE_ADC_ADDR		0x49
#define BATT_HV_SENSE_ADC_ADDR		0x48

#define ADC_RESOLUTION 32768
#define ADC_REF_VOLTAGE 6.144

#define PDOC_THERMISTOR_BETA 3380

#define PDOC_ADC_SENSOR1_CHANNEL 0
#define PDOC_ADC_REF_CHANNEL 1
#define PDOC_ADC_SENSOR2_CHANNEL 2


#define HV_ADC_SENSOR_CHANNEL 0
#define HV_ADC_TSAL_CHANNEL 3

#define PDOC_OVER_TEMPERATURE 250 
#define PDOC_UNDER_TEMPERATURE -40

#define HV_OVER_VOLTAGE 600 
#define HV_UNDER_VOLTAGE -5

// Serial pc(PIN_SERIAL_TX, PIN_SERIAL_RX); 

	/*
PDOC
	Precharge Discharge Over-Current device. Checks if precharge/discharge relays
	have overheated, hinting towards an overcurrent device. Inherrits ADS1115
	HV object. 
	*/
class PDOC{
public:
	/** PDOC Constructor
     *
	 * Connect to bus and set address.
	 * 
	 * Update ADC with:
	 * 	- update_adc();
	 * 
	 * Read ADC values with:
	 * 	- get_pdoc_temperature();
	 *  - get_pdoc_ref_temperature();
	 * 
     *  @param &i2c1 I2C bus to interface.
     *  @param _adc_addr Address of ADS1115 unit. 
     */
	PDOC(I2C &i2c1, int _adc_addr, PinName PDOC_ok_pin) : pdoc_adc(&i2c1, _adc_addr), PDOC_ok(PDOC_ok_pin){};
	
	/** update_adc()
	 * 
	 * Read from the pdoc temperature and the reference temperature and 
	 * update the member values.
     *
     *  @returns sensor validity as boolean. 
     */
	bool update_adc(){
		pdoc_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(PDOC_ADC_SENSOR1_CHANNEL), ADC_RESOLUTION, ADC_REF_VOLTAGE), PDOC_THERMISTOR_BETA);
		// pc.printf("PDOC_TEMPERATURE: %d \r\n", pdoc_temperature);
		pdoc_ref_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(PDOC_ADC_REF_CHANNEL), ADC_RESOLUTION, ADC_REF_VOLTAGE), PDOC_THERMISTOR_BETA);
		// pc.printf("PDOC_REF_TEMPERATURE: %d \r\n", pdoc_ref_temperature);
		return sensor_ok();
	}

	/** pdoc_ok()
     *
	 * Checks the PDOC is less than the reference value. 
	 * 
     *  @returns true if either the reference temperature is within margins or the relay is still active.
	 * Vehicle will be disabled if relay opens, however if the sensor also disagrees, then report failure.
     *  
     */
	bool pdoc_ok(){
		bool _pdoc_ok = false;
		if (pdoc_temperature < pdoc_ref_temperature || PDOC_ok.read()){
			_pdoc_ok = true;
		}
		return _pdoc_ok;
	}

	/** check_pdoc_relay_fail()
     *
	 * Checks the PDOC's reference value and the relay state match. Checks comparitor is outputting.
	 * 
     *  @returns True on fail.
     *  
     */
	bool check_pdoc_relay_fail(){
		bool soft_pdoc_fail = false;
		if (pdoc_temperature > pdoc_ref_temperature){
			soft_pdoc_fail = true;
		}

		if (soft_pdoc_fail != PDOC_ok){
			return false;
		}
		return true;
	}

	int16_t get_pdoc_channel_raw(uint8_t channel){
		return pdoc_adc.readADC_SingleEnded(channel);
	}
	float get_pdoc_channel_voltage(uint8_t channel){
		return adc_to_voltage(pdoc_adc.readADC_SingleEnded(channel), ADC_RESOLUTION, ADC_REF_VOLTAGE);
	}
	int get_pdoc_ok(){return PDOC_ok.read();}
	int get_pdoc_temperature(){return pdoc_temperature;}
	int get_pdoc_ref_temperature(){return pdoc_ref_temperature;}
	bool get_sensor_ok(){return sensor_ok_flag;}
	//float test_setpoint(){return NTC_voltageToTemperature(0.5, PDOC_THERMISTOR_BETA);}
	

private:
	int16_t pdoc_temperature;
	int16_t pdoc_ref_temperature;

	bool sensor_ok_flag;

	DigitalIn PDOC_ok;

	Adafruit_ADS1115 pdoc_adc;

    float adc_to_voltage(int16_t adc_value, int adc_resolution, float voltage_range){
	    float voltage = adc_value * voltage_range / adc_resolution;
	    return voltage;
    }

    int NTC_voltageToTemperature(float voltage, float BETA=PDOC_THERMISTOR_BETA){
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

	bool sensor_ok(){
		bool sensor_ok = false;
		if (pdoc_temperature < PDOC_OVER_TEMPERATURE){
			sensor_ok = true;
		} else {
            sensor_ok = false;
        }
        
		if (pdoc_temperature > PDOC_UNDER_TEMPERATURE){
			sensor_ok = true;
		} else {
            sensor_ok = false;
        }
		sensor_ok_flag = sensor_ok;
		return sensor_ok;
	}
};

	/*
HV_ADC
	High Voltage measurement interface. Isoalted interface that is used to 
	check if measurements are valid and returns them for later data acquisition purposes.
	*/
class HV_ADC{
public:
	/** HV_ADC Constructor
     *
	 * Connect to bus and set address.
	 * 
     *  @param &i2c1 I2C bus to interface.
     *  @param _adc_addr Address of ADS1115 unit. 
	 *
     */
	HV_ADC(I2C &_i2c1, int _adc_addr) : high_voltage_adc(&_i2c1, _adc_addr){
        R_CAL = 0;
    };

	/** HV_ADC Constructor
     *
	 * Initialise bus, set address, and the calibration offset on the resistor bridge.
	 * 
     *  @param &i2c1 I2C bus to interface.
     *  @param _adc_addr Address of ADS1115 unit. 
	 * 	@param _R_CAL value of lowest pot in resistor bridge.
     */
	HV_ADC(I2C &_i2c1, int _adc_addr, float _R_CAL) : high_voltage_adc(&_i2c1, _adc_addr){
        R_CAL = _R_CAL;
    };

	/** update_adc()
	 * 
	 * Read from the resistor bridge and update the member values. Note that the TSAL
	 * is only attached on the discharge, this value can be ignored otherwise.
	 * 
	 * Read ADC values with:
	 * 	- get_voltage();
	 *  - get_tsal_reference();
     *
     *  @returns sensor validity as boolean. 
     */
	bool update_adc(){
		voltage = HV_voltageScaling(adc_to_voltage(high_voltage_adc.readADC_SingleEnded(HV_ADC_SENSOR_CHANNEL), ADC_RESOLUTION, ADC_REF_VOLTAGE));
	    tsal_reference = adc_to_voltage(high_voltage_adc.readADC_SingleEnded(HV_ADC_TSAL_CHANNEL), ADC_RESOLUTION, ADC_REF_VOLTAGE);
		// pc.printf("MC_VOLTAGE: %d \r\n", mc_voltage);
	    return sensor_ok();
	}

	int get_voltage(){return voltage;}
	int get_tsal_reference(){return tsal_reference;}
	bool get_sensor_ok(){return sensor_ok_flag;}

private:
    float R_CAL;

	int16_t voltage;
	int16_t tsal_reference;

	bool sensor_ok_flag;

	Adafruit_ADS1115 high_voltage_adc;

    float adc_to_voltage(int16_t adc_value, int adc_resolution, float voltage_range){
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

	bool sensor_ok(){
		bool sensor_ok = false;
		if (voltage < HV_OVER_VOLTAGE){
			sensor_ok = true;
		} else {
            sensor_ok = false;
        }

        if (voltage > HV_UNDER_VOLTAGE){
        	sensor_ok = true;
        } else {
        	sensor_ok = false;
        }
		sensor_ok_flag = sensor_ok;
		return sensor_ok;
    }
		
};

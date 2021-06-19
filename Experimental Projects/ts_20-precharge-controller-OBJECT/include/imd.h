#include <mbed.h>

	/*
IMD
	Isolation Monitoring Device data interface. The IMD is required to 
	check that the HV and GLV systems are isolated. A PWM signal is generated 
	by the IMD for diagnosis. This object makes use of an interrupt pin and timer
	to return the period, frequency, and duty cycle of the waveform.
	*/
class IMD {
public: 
	/** IMD Constructor
     *
	 * Set the pin for the IMD data interface.
	 * 	
	 * 	@param IMD_ok_pin location of the IMD_ok pin.
     *  @param IMD_data_interrupt_pin location of the IMD_data pin.
	 * 
     */
	IMD(PinName IMD_ok_pin, PinName IMD_data_interrupt_pin) : IMD_ok(IMD_ok_pin), IMD_data_interrupt(IMD_data_interrupt_pin) {}

	/** start()
	 * 
	 * Creates the interrupt routines for the interrupt pins. 
	 * 
	 * On case of rise, callsback set_period(), which resets the timer from the last interrupt.
	 * On case of the subsequent fall, where the active part of the period will sink, calls
	 * set_duty_cycle(), which will measure the time, and return that as a percentage of the 
	 * period.
	 * 
	 * Read values with:
	 * 	- get_period();
	 *  - get_frequency();
	 * 	- get_duty_cycle();
	 * 
     */
	void start(){
		IMD_data_interrupt.rise(callback(this, &IMD::set_period));
		IMD_data_interrupt.fall(callback(this, &IMD::set_duty_cycle));
		imd_timer.start();
	}

	int get_IMD_ok(){return IMD_ok.read();}
	int get_period(){return period;}
	int get_frequency(){return frequency;}
	int get_duty_cycle(){return duty_cycle;}

private:
	DigitalIn IMD_ok;
	InterruptIn IMD_data_interrupt;

	float active_end;
	float duty_cycle;
	float period;

	int	frequency;

	Timer imd_timer;

	void set_period(){
		period = imd_timer.read_us();
		frequency = 1000000.0f/period;
        imd_timer.reset();
	}

	void set_duty_cycle(){
		active_end = imd_timer.read_us();
		duty_cycle = (active_end/period)*100.0;
	}
};

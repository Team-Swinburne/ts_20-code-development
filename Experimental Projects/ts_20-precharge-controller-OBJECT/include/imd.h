#include <mbed.h>

class IMD_Data {
public: 
	IMD_Data(PinName imd_data_interrupt_pin) : imd_data_interrupt(imd_data_interrupt_pin) {}

	void start(){
		imd_data_interrupt.rise(callback(this, &IMD_Data::set_period));
		imd_data_interrupt.fall(callback(this, &IMD_Data::set_duty_cycle));
		imd_timer.start();
	}

	void set_period(){
		period = imd_timer.read_us();
		frequency = 1000000.0f/period;
        imd_timer.reset();
	}

	void set_duty_cycle(){
		active_end = imd_timer.read_us();
		duty_cycle = (active_end/period)*100.0;
	}

	int get_period(){return period;}
	int get_frequency(){return frequency;}
	int get_duty_cycle(){return duty_cycle;}

private:
	InterruptIn imd_data_interrupt;

	float active_end;
	float duty_cycle;
	float period;

	int	frequency;

	Timer imd_timer;
};

#include <mbed.h>

class Relay{
public:
    Relay(PinName relay_pin) : _Relay(relay_pin){
        _Relay = 0;
    }
    
    void activate_relay(){
		_Relay = 1;
	}

	void disable_relay(){
		_Relay = 0;
	}

	void set_relay(bool state){
		_Relay = state;
	}

private:
    DigitalOut _Relay;
};

class AIR{
public:
	AIR(PinName relay_pin, PinName feedback_pin) : Relay(relay_pin), Feedback(feedback_pin){
		Relay = 0;
	}

	void activate_relay(){
		Relay = 1;
	}

	void disable_relay(){
		Relay = 0;
	}

	void set_relay(bool state){
		Relay = state;
	}

	bool relay_ok(){
		bool relay_state = false;
		if (Relay == Feedback){
			relay_state = true;
		}
		return relay_state;
	}

private:
	DigitalOut Relay;
	DigitalIn Feedback;
};

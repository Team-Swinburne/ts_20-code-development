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

	void set_relay(bool state){_Relay = state;}
	bool get_relay(){return _Relay.read();}

private:
    DigitalOut _Relay;
};

class AIR{
public:
	AIR(PinName relay_pin, PinName feedback_pin) : Relay(relay_pin), Feedback(feedback_pin, PullDown){
		Relay = 0;
	}

	void activate_relay(){
		Relay = 1;
	}

	void disable_relay(){
		Relay = 0;
	}

	bool relay_ok(){
		bool relay_state = false;
		if (Relay.read() == Feedback.read()){
			relay_state = true;
		}
		return relay_state;
	}

	void set_relay(bool state){Relay = state;}

	bool get_relay(){return Relay.read();}
	bool get_feedback(){return Feedback.read();}

private:
	DigitalOut Relay;
	DigitalIn Feedback;
};

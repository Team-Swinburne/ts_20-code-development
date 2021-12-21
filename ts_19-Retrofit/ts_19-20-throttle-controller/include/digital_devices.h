// Check button and shit like that

#include <mbed.h>

#define 		BUTTON_CHECK_INTERVAL_CHRONO		200ms		// 5 Hz
#define 		BUTTON_CHECK_INTERVAL           	0.2		// 5 Hz
#define 		BUTTON_PUSH_TIME			        1           // Time button must be held in seconds		1	

class Button {
public:
    /** 
     * @brief Button Contructor
     * 
     * @param button_pin
     */
    Button(PinName button_pin) : button_IN(button_pin){
        button_pressed = false;
        button_counter = 0;
    }

    /** 
     * @brief Update the status of the button
     */
    void update_value(){
        if (button_IN) {
            button_counter++;
        }
        else
        {
            button_counter = 0;
            button_pressed = false;
        }
        if (button_counter > BUTTON_PUSH_TIME/BUTTON_CHECK_INTERVAL) {

            button_pressed = true;
        }
    }

    /**
     * @brief Check wether the button is pressed
     * 
     * @return true if the button is pressed more than 1 second
     * @return false otherwise
     */
    bool isPressed() {return button_pressed;}
private:
    bool button_pressed;
    int button_counter;

    DigitalIn button_IN;
};

// RTDS class to handle the Tweety Bird
class RTDS {
public:

    /** 
    * @brief RTDS Contructor
    * 
    * @param rtds_pin
    */
    RTDS(PinName rtds_pin): rtds_OUT(rtds_pin){
        rtds_sounded = false;
    }

    /**
     * Activate the RTDS for a set period of time
     * 
     * @param rtds_on_interval(ms) chrono time duration in
     */
    void activate(std::chrono::milliseconds rtds_on_interval) {
        rtds_OUT = 0;
        rtds_timeout.attach(callback(this, &RTDS::deactivate_cb),rtds_on_interval);
    }

    void deactivate_cb() {
        rtds_OUT = 0;
    }

    void set_sounded(bool _rtds_sounded) {rtds_sounded=_rtds_sounded;}
    bool get_sounded() {return rtds_sounded;}
    
private:
    bool rtds_sounded;

    DigitalOut rtds_OUT;
    Timeout rtds_timeout;


};
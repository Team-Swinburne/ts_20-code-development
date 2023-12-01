// Check button and shit like that

#include <mbed.h>

#define 		BUTTON_CHECK_INTERVAL_CHRONO		200ms		// 5 Hz
#define 		BUTTON_CHECK_INTERVAL           	0.2		// 5 Hz
#define 		BUTTON_PUSH_TIME			        1           // Time button must be held in seconds		1	

class Button {
public:
    bool button_pressed;
    bool state_AV;
    bool state_AV_drive;
    /** 
     * @brief Button Contructor
     * 
     * @param button_pin
     */
    Button(PinName button_pin) : button_IN(button_pin){
        button_pressed = false;
        button_counter = 0;
        // state_AV = false;
    }

    void set_AV_state(bool state) 
        { 
            state_AV = state; 
           
        }
    void set_AV_drive_state(bool state) 
        { 
            state_AV_drive = state; 
           
        }
    /** 
     * @brief Update the status of the button
     */
    void update_value(){
        if (!state_AV){ 
            if (button_IN) {
                button_counter++;
            }
            else
            {
                button_counter = 0;
                button_pressed = false;
            }
            if (button_counter > (int)(BUTTON_PUSH_TIME/BUTTON_CHECK_INTERVAL)) {

                button_pressed = true;
            }
        }
    }

    /**
     * @brief Check wether the button is pressed
     * 
     * @return true if the button is pressed more than 1 second
     * @return false otherwise
     */
    //  char isPressed() {button_pressed;}
    
        // char isPressed() {return ((state_AV)? 1 : button_pressed);} // THIS IS JUST FOR TESTING
    char isPressed(){

        if(state_AV_drive){
            return true;
        }
        else{
            return button_pressed;
        }
    }

        // char isPressed() {return ((state_AV)? 1 : tue);} // THIS IS JUST FOR TESTING

private:
    
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
    void activate(float rtds_on_interval) {
        rtds_OUT = 1;
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

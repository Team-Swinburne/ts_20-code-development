#include <mbed.h>

/***************************************************************************
    watchdogs.h

    Since Pre-charge is the only board on the car with the ability to trip Tractive Loop with
    a digital pin, it is used to check for vital errors on other boards of the car through CAN 
    and shut the car down as a pre-caution.
    
    UCM Accumulator, UCM Inverter, and the Motor Controller are boards that will use this watchdog.
    DO NOT mistake this for the built-in mBed Watchdog

****************************************************************************/
#define CONNECTED       true
#define DISCONNECTED    false

class Watchdogs {
public:
    Watchdogs(void) :   TIMEOUT_INTERVAL(0), 
                        device_connection_status(DISCONNECTED) {}
    Watchdogs(float interval) : TIMEOUT_INTERVAL(interval) {}

    void connect() {
        if (TIMEOUT_INTERVAL > 0) {
            device_timeout.detach();
            device_connection_status = CONNECTED;
            device_timeout.attach(callback(this, &Watchdogs::disconnect), TIMEOUT_INTERVAL);
        }
    }

    void disconnect() {
        device_connection_status = DISCONNECTED;

    }
    bool check_device() {
        if (device_error && device_connection_status) {
            return true;
        }
        return false;
    }

    void set_device_error(bool _device_error){device_error = _device_error;}
    bool get_device_ok(){return check_device();}
private:
    const float TIMEOUT_INTERVAL;

    Timeout device_timeout;
    bool device_connection_status = DISCONNECTED, device_error;
};
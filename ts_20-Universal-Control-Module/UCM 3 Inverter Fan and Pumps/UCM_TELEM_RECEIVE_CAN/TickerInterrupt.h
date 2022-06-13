// TEAM SWINBURNE - STM32 TICKER ARDUINO LIBRARY USING PROPER ISR
// NAM TRAN
// REVISION 0 (24/06/2021)

/***************************************************************************
    TickerInterrupt.h

    INTRO
    This library support attaching callback function to "proper"ticker
    with stm32duino framework.
    A HardwareTimer is used as the base interrupt will call the timerHandler 
    function every 1ms. timerHandler will then check if it's time to call the
    actual callback function using a counter

    Based on STM32_TimerInterrupt
    Author: Khoi Hoang

    Revision     Date          Comments
    --------   ----------     ------------
    0.0        24/06/2021     Initial coding

****************************************************************************/
#include "Arduino.h"

typedef void (*fpointer)(); // function pointer for passing ticker callback
struct tickerInfo {
    uint32_t interval;
    uint32_t counter;
    fpointer callback;
};

#define MAX_TICKER_NUMBER 10

/**TickerInterrupt Contructor
 *
 * Attach Timer handler to a HarwareTimer, only create 1 instance
 *
 * @param _hardwareTimer  Choose Hardware timer (TIM1, TIM2, etc.)
 * @param _interval(ms)   Interval at which the Hardware timer runs
 * 
 * @warning Only create 1 instance of this
*/
class TickerInterrupt {
public:
    TickerInterrupt(TIM_TypeDef *_hardwareTimer, double _interval);
    
    /// Start the Hardware timer
    void start();

    /**start()
     * Attach callback function
     
     * @param[in] _callback  Pass callback function (global or static only)
     * @param _interval(ms)  Callback execution interval
    */
    void attach(fpointer _callback, int _interval);

private:
    uint32_t interval;
    TIM_TypeDef *timerInstance;

    static volatile int tickerNumber;
    static tickerInfo ticker[MAX_TICKER_NUMBER];

    /**timerHandler()
     * main "logic" of the ticker
    */
    static void timerHandler();
};

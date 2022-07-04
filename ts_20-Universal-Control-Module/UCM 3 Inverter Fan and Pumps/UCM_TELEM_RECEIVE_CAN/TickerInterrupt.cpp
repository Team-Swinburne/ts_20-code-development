// TEAM SWINBURNE - STM32 TICKER ARDUINO LIBRARY USING PROPER ISR
// NAM TRAN
// REVISION 0 (24/06/2021)

/***************************************************************************
    TickerInterrupt.cpp

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
#include <Arduino.h>
#include "TickerInterrupt.h"

volatile int TickerInterrupt::tickerNumber = -1;
tickerInfo TickerInterrupt::ticker[MAX_TICKER_NUMBER];

TickerInterrupt::TickerInterrupt(TIM_TypeDef *_timerInstance, double _interval) {
    timerInstance = _timerInstance;
    interval = (uint32_t)(_interval*1000);
}

void TickerInterrupt::timerHandler() {
    for (int i = 0; i <= tickerNumber; i++) {
        ticker[i].counter++;
        if (ticker[i].counter/ticker[i].interval >= 1) {
            ticker[i].counter = 0;
            ticker[i].callback();
        }    
    }
}

void TickerInterrupt::start() {
    HardwareTimer *timer = new HardwareTimer(timerInstance);
    timer->setOverflow(interval,MICROSEC_FORMAT); // 10 Hz
    timer->attachInterrupt(timerHandler);
    timer->resume();
}

void TickerInterrupt::attach(fpointer _callback, int _interval) {
    tickerNumber++;
    if (tickerNumber < MAX_TICKER_NUMBER) {
        ticker[tickerNumber].counter = 0;
        ticker[tickerNumber].interval = (uint32_t)(_interval);
        ticker[tickerNumber].callback = _callback;
    }
    delay(10);
}

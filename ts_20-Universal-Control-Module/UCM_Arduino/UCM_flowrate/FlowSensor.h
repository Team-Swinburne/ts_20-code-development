#ifndef __FLOW_SENSOR__
#define __FLOW_SENSOR__
#include <Arduino.h>

#define FLOW_SENSOR_1_CALIBRATION 1.0f

int Flowsensor_1_Pulses = 0;
int Flowsensor_1_PulsesTotal = 0;
float Flowsensor_1_LiquidTotal = 0.0;
float Flowsensor_1_FlowRate = 0.0;

void FlowSensor1_Increment()
{
  Flowsensor_1_Pulses++;
  Flowsensor_1_PulsesTotal++;
}

class FlowSensor
{
    public:
    static void measure()
    {
      Flowsensor_1_LiquidTotal = FLOW_SENSOR_1_CALIBRATION * Flowsensor_1_PulsesTotal;
      Flowsensor_1_FlowRate = FLOW_SENSOR_1_CALIBRATION * Flowsensor_1_Pulses;
      Flowsensor_1_Pulses = 0;
    }

    static int getPulses()
    {
      
    }

    FlowSensor(int FlowSensorPin, void (&function)(void))
    {
        pinMode(FlowSensorPin, INPUT);
        attachInterrupt(digitalPinToInterrupt(FlowSensorPin), function, RISING);
    };

    private:
};






#endif //__FLOW_SENSOR__

#ifndef __FLOW_SENSOR__
#define __FLOW_SENSOR__
#include <Arduino.h>

void FlowSensor_init(int _FlowSensor, int _FlowSensorPin);
void FlowSensor_Measure();

#endif //__FLOW_SENSOR__

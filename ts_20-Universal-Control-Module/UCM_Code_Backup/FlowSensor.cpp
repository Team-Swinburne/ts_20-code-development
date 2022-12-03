#include "FlowSensor.h"

float FS1_Calibration = 0.222;
float FS2_Calibration = 1.0;
float FS3_Calibration = 1.0;
float FS4_Calibration = 1.0;
   
extern volatile int FS1_Pulses;
extern volatile int FS2_Pulses;
extern volatile int FS3_Pulses;
extern volatile int FS4_Pulses;

extern float FS1_FlowRate;
extern float FS2_FlowRate;
extern float FS3_FlowRate;
extern float FS4_FlowRate;

static void FlowSensor1_INT()
{
  FS1_Pulses++;
}

static void FlowSensor2_INT()
{
  FS2_Pulses++;
}

static void FlowSensor3_INT()
{
  FS3_Pulses++;
}

static void FlowSensor4_INT()
{
  FS4_Pulses++;
}

void FlowSensor_init(int _FlowSensor, int _FlowSensorPin)
{
  switch(_FlowSensor)
  {
    case 1:
      pinMode(_FlowSensorPin, INPUT);
      attachInterrupt(digitalPinToInterrupt(_FlowSensorPin), FlowSensor1_INT, RISING);
      break;

    case 2:
      pinMode(_FlowSensorPin, INPUT);
      attachInterrupt(digitalPinToInterrupt(_FlowSensorPin), FlowSensor2_INT, RISING);
      break;

    case 3:
      pinMode(_FlowSensorPin, INPUT);
      attachInterrupt(digitalPinToInterrupt(_FlowSensorPin), FlowSensor3_INT, RISING);
      break;

    case 4:
      pinMode(_FlowSensorPin, INPUT);
      attachInterrupt(digitalPinToInterrupt(_FlowSensorPin), FlowSensor4_INT, RISING);
      break;
  }
}

void FlowSensor_Measure()
{
  FS1_FlowRate = FS1_Calibration * FS1_Pulses;
  FS2_FlowRate = FS2_Calibration * FS2_Pulses;
  FS3_FlowRate = FS3_Calibration * FS3_Pulses;
  FS4_FlowRate = FS4_Calibration * FS4_Pulses;
        
  FS1_Pulses = 0;
  FS2_Pulses = 0;
  FS3_Pulses = 0;
  FS4_Pulses = 0;  
}

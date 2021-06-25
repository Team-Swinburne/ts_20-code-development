#include <Arduino.h>
#include <ADC_ADS1115.h>

ADC_ADS1115 ads(&Wire);
ADC_ADS1115 ads2(&Wire);

void setup(void)
{
  Serial.begin(115200);

  ads.setAddr_ADS1115(ADS1115_IIC_ADDRESS0); // 0x48
  ads.setGain(eGAIN_TWOTHIRDS);              // 2/3x gain
  ads.setMode(eMODE_SINGLE);                 // single-shot mode
  ads.setRate(eRATE_128);                    // 128SPS (default)
  ads.setOSMode(eOSMODE_SINGLE);             // Set to start a single-conversion
  ads.init();

  ads2.setAddr_ADS1115(ADS1115_IIC_ADDRESS1); // 0x49
  ads2.setGain(eGAIN_TWOTHIRDS);              // 2/3x gain
  ads2.setMode(eMODE_SINGLE);                 // single-shot mode
  ads2.setRate(eRATE_128);                    // 128SPS (default)
  ads2.setOSMode(eOSMODE_SINGLE);             // Set to start a single-conversion
  ads2.init();
}

void loop(void)
{
  if (ads.checkADS1115())
  {
    int16_t adc0, adc1, adc2, adc3;
    adc0 = ads.readVoltage(0);
    Serial.print("A0:");
    Serial.print(adc0);
    Serial.println("mV");
    adc1 = ads.readVoltage(1);
    Serial.print("A1:");
    Serial.print(adc1);
    Serial.println("mV");
    adc2 = ads.readVoltage(2);
    Serial.print("A2:");
    Serial.print(adc2);
    Serial.println("mV");
    adc3 = ads.readVoltage(3);
    Serial.print("A3:");
    Serial.print(adc3);
    Serial.println("mV");
  }
  else
  {
    Serial.println("ADS1115 Disconnected!!!");
  }
  Serial.println("-----------------------------------");
  if (ads2.checkADS1115())
  {
    int16_t adc02, adc12, adc22, adc32;
    adc02 = ads2.readVoltage(0);
    Serial.print("A02:");
    Serial.print(adc02);
    Serial.println("mV");
    adc12 = ads2.readVoltage(1);
    Serial.print("A12:");
    Serial.print(adc12);
    Serial.println("mV");
    adc22 = ads2.readVoltage(2);
    Serial.print("A22:");
    Serial.print(adc22);
    Serial.println("mV");
    adc32 = ads2.readVoltage(3);
    Serial.print("A32:");
    Serial.print(adc32);
    Serial.println("mV");
  }
  else
  {
    Serial.println("ADS1115 2 Disconnected!!!");
  }
  Serial.println("-----------------------------------");
  delay(1000);
}
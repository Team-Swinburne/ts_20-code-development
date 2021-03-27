#include <mbed.h>
#include <vector>

#include "Adafruit_ADS1015.h"
// Adafruit_ADS1115 ads(&i2c, 0x4B);

using namespace std;

const int HIGHEST_ADDR = 127;
const char NULL_FRAME[] = {0x00};

I2C i2c(PB_7, PB_6);
Serial pc(PA_2, PA_3);

DigitalOut led1(PC_13);

Adafruit_ADS1115 ads(&i2c, 0x4B);

int main() {
  pc.printf("Starting MBED I2C Scanner (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);  
  
  i2c.frequency(100000);

  uint16_t reading;
  while (1) {
    reading = ads.readADC_SingleEnded(0); // read channel 0
    pc.printf("reading: %d\r\n", reading); // print reading    
    wait(0.5); // loop 2 sek
  }

  // vector<int> addresses;

  // while(1) {
  //   pc.printf("\r\n --- SCANNING BUS --- \r\n");

  //   for (int i = 0; i <= HIGHEST_ADDR; i++) {
  //     if (!i2c.write(i, NULL_FRAME, 1)){
  //       pc.printf("Found, added %X \n\r", i);
  //       addresses.push_back(i);
  //     } else {
  //       pc.printf("Nothing at %X \n\r", i);
  //     }
  //     wait(0.05);
  //     led1 = !led1;
  //   }

  //   pc.printf(" \r\n --- ANALYSIS --- \n\r");
  //   pc.printf("Found %d addresses at the following addresses: \r\n", addresses.size());

  //   for (int i = 0; i < addresses.size(); i++){
  //     pc.printf(" - %X, \r\n", addresses[i]);
  //   }

  //   addresses.clear();

  //   pc.printf("\r\n --- FINISHED --- \r\n");

  //   wait(5);
    
    // break; 

    // // HOLD EXECUTION UNTIL REQUESTED
    // pc.printf("Any key to restart \r\n");
    // while (!pc.readable()) {
    //   pc.printf(".");
    // }
}
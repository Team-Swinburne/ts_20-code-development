#include <mbed.h>
#include <vector>

using namespace std;

const int LOWEST_ADDR = 0
const int HIGHEST_ADDR = 127;
const int NULL_FRAME = [0x00, 0x00];

int main() {
  pc.printf("Starting MBED I2C Scanner (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);

  I2C i2c(PB_7, PB_6);
  Serial pc(PA_2, PA_3);
  
  i2c.frequency(100000);

  vector<int> addresses;

  while(1) {
    pc.printf("\r\n --- SCANNING BUS --- \r\n")

    for (int address = LOWEST_ADDR; address <= HIGHEST_ADDR; address++) {
      if (!i2c.write(address, NULL_FRAME, 1)){
        pc.printf("Found, added %X \n\r", address);
        addresses.push_back(address);
      } else {
        pc.printf("Nothing at %f \n\r", address)
      }
    }

    pc.printf(" \r\n --- ANALYSIS --- \n\r");
    pc.printf("Found %d addresses at the following addresses: \r\n", addresses.size());

    for (int i = 0; i < addresses.size(); i++){
      pc.printf(" - %X, \r\n", addresses[i]);
    }

    addresses.clear();

    pc.printf("\r\n --- FINISHED --- \r\n");

    pc.printf("Any key to restart \r\n");
    while (!pc.readable()) {
      pc.printf(".");
    }
  }
}
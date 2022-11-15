// TEAM SWINBURNE - TS_22
// PRECHARGE CONTROLLER
// 
// Test File: test_power_supply.cpp (22/06/2022)

#include <mbed.h>

DigitalOut led1(PC_13);
DigitalOut air_pos(PA_10);
DigitalOut air_neg(PA_8);
DigitalOut precharge(PA_9);
DigitalOut AMS_ok(PB_13);
DigitalOut Led_TX(PB_0);
DigitalOut Led_RX(PB_1);
DigitalOut Led_2(PA_6);
DigitalOut Led_3(PA_7);


void load(){
	led1 = 0;
	air_pos = 1;
	air_neg = 1;
	precharge = 1;
	AMS_ok = 1;
	Led_TX = 1;
	Led_RX = 1;
	Led_2 = 1;
	Led_3 = 1;
}

void unload(){
	led1 = 1;
	air_pos = 0;
	air_neg = 0;
	precharge = 0;
	AMS_ok = 0;
	Led_TX = 0;
	Led_RX = 0;
	Led_2 = 0;
	Led_3 = 0;
}

int main() {

	unload();

	return 0;
}

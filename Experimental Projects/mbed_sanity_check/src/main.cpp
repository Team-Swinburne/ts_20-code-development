#include "mbed.h"

Serial pc(PA_2, PA_3, 9600);

DigitalOut led1(PC_13);

int main(){

    pc.printf("STARTING!!! \r\n");


    while(1){
        led1 = !led1;

        pc.printf("SCREAMING!!! \r\n");

        wait(1);
    }

    return 0;
}
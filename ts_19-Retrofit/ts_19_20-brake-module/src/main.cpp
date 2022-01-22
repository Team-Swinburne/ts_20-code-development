// TEAM SWINBURNE - TS19-20 Retrofit
// BRAKE MODULE
// MICHAEL COCHRANE & PATRICK CURTAIN & NAM TRAN
//-----------------------------------------------
//Pins
//5: MOSI
//6: MISO
//7: SCK
//8: GPIO
//9: SDA/TX         - CAN1 RX
//10: SCL/RX        - CAN1 TX
//11: MOSI2
//12: MISO2
//13: SCK/TX
//14: RX
//15: ADC1          - BOTS OK
//16: ADC2          - Cockpit OK
//17: ADC3          - Inertia OK
//18: ADC4/AOut     - SDN OK
//19: ADC5          - BSPD OK
//20: ADC6
//21: PWM1          
//22: PWM2
//23: PWM3      
//24: PWM4  
//25: PWM5
//26: PWM6
//27: SCL/RX        - I2C SCI (Brake)
//28: SDA/TX        - I2C SDA (Brake)
//29: CANTX         - CAN2 TX
//30: CANRX         - CAN2 RX
//...
//35: RD+ (ethernet) - High Pressure (NOT AVALIABLE!)
//-----------------------------------------------

//Init
#include <mbed.h>
#include <CAN.h>
#include <stdio.h>

CAN                     can1(p9, p10);
DigitalOut              can1_rx_led(LED1);
DigitalOut              can1_tx_led(LED2); 
CAN                     can2(p30, p29);
DigitalOut              can2_rx_led(LED3);
DigitalOut              can2_tx_led(LED4);
CANMessage              can1_msg;
CANMessage              can2_msg;

I2C                     brake(p28, p27);
DigitalIn               BOTS_ok(p16);
DigitalIn               cockpit_ok(p17);
DigitalIn               intertia_ok(p18);
DigitalIn               SDN_ok(p19);
DigitalIn               BSPD_ok(p20);
DigitalIn               BSPD_Test(p21);

Ticker                  ticker_heartbeat;
Ticker                  ticker_buttons;
Ticker                  ticker_1hz;
Ticker                  ticker_can_transmit;

int8_t                  heartbeat_state             = 0;
int                     heartbeat_counter           = 0;    

// BRAKE LIMITS
int                 brake1_min;                 // = 400;
int                 brake1_max;                 // = 1100;
int                 brake2_min;                 // = 400;
int                 brake2_max;                 // = 1100;

#define                 INVERT_BRAKE_INPUT          false
#define                 BRAKE_ADC_ADDR              0x68        // Address of the MCP3428
#define                 Deadzone                    35          // 35% Pedal deadzone

int16_t                 brake_output_1              = 0;
float                   brake_output_1_percent      = 0;
int16_t                 brake_output_2              = 0;
float                   brake_output_2_percent      = 0;
int                     brake_avg_percent           = 0;

// Intervals
#define                 BRAKE_HEARTRATE             1           // 1 Hz
#define                 BUTTON_CHECK_INTERVAL       0.2         // 5 Hz 
#define                 CAN_BROADCAST_INTERVAL      0.02        // 50 Hz

//-----------------------------------------------

// CAN IDs

//Brake
#define BRAKE_HEARTBEAT_ID                          0x308
#define BRAKE_SENSORS_ID                            0x309
#define BRAKE_SAFETY_ID                             0x30A

//-----------------------------------------------

// Local storage for calibrating sensor
LocalFileSystem local("local");
FILE *stream = fopen("/local/brake_ca.txt", "r");

//-----------------------------------------------

//Heartbeat - Broadcasts that the throttle is still alive and functional with a counter so we know when it has failed if it does
void Heartbeat(){
    heartbeat_counter++;
    char TX_data[2] = {(char)heartbeat_state, (char)heartbeat_counter};
    if(can1.write(CANMessage(BRAKE_HEARTBEAT_ID, &TX_data[0], 2))) 
    {
        //printf("Heartbeat Success! State: %d Counter: %d\n", heartbeat_state, heartbeat_counter);
    }else
    {
        printf("Hearts dead :(\n");
    }
}

// Receive CAN1 Data
void CAN1_Receive(){
    can1_rx_led = !can1_rx_led;
    if (can1.read(can1_msg)) {
        if ((can1_msg.id == 0x700)) {
            int brake1_min_t = can1_msg.data[0] << 8 | can1_msg.data[1];
            int brake1_max_t = can1_msg.data[2] << 8 | can1_msg.data[3];
            int brake2_min_t = can1_msg.data[4] << 8 | can1_msg.data[5];
            int brake2_max_t = can1_msg.data[6] << 8 | can1_msg.data[7];
            
            //save calibration
            freopen("/local/brake_ca.txt", "w",stream);
            fprintf(stream, "%d %d\n", brake1_min_t, brake1_max_t);
            fprintf(stream, "%d %d\n", brake2_min_t, brake2_max_t);
            fclose(stream);
        }
    }
}

// Receive CAN2 Data
void CAN2_Receive(){
    can2_rx_led = !can2_rx_led;
}

// Transmit CAN1 Data
void CAN1_Transmit(){
    can1_tx_led = !can1_tx_led;
    char TX_data[8] = {0};

    //Broadcast SDN/Cockpit/Interia/BOTS states
    TX_data[0] = BOTS_ok.read();
    TX_data[1] = cockpit_ok.read();
    TX_data[2] = intertia_ok.read();
    TX_data[3] = SDN_ok.read();
    TX_data[4] = BSPD_Test.read();
    
    if(can1.write(CANMessage(BRAKE_SAFETY_ID, &TX_data[0], 5))) 
    {
        //printf("Safety Loop State Transmit Success! BOTS_ok: %d cockpit_ok: %d\r\ninertia_ok: %d, SDN_ok: %d, BSPD_ok: %d\r\n", TX_data[0], TX_data[1], TX_data[2], TX_data[3], TX_data[4]);
    }

    //Broadcast brake sensor percentages
    TX_data[0] = brake_output_1_percent;
    TX_data[1] = brake_output_2_percent;
    TX_data[2] = brake_avg_percent;
    TX_data[3] = brake_output_1 >> 8;
    TX_data[4] = brake_output_1 & 0xFF;
    TX_data[5] = brake_output_2 >> 8;
    TX_data[6] = brake_output_2 & 0xFF;
    
    if(can1.write(CANMessage(BRAKE_SENSORS_ID, &TX_data[0], 3))) 
    {
        //printf("Brake State Transmit Success! B1 Percent: %d B2 Percent: %d AVG Percent: %d\r\n", TX_data[0], TX_data[1], TX_data[2]);
    }
}

// Transmit CAN2 Data
void CAN2_Transmit(){
    can2_tx_led = !can2_tx_led;
}

// Read in throttle pedal positions from I2C pins
void Read_Brake_Pedal(){
    char cmd[1];
    char data[2];

    cmd[0] = 0;
    data[0] = 0; data[1] = 0;

    //Read from Channel 1 of the MCP3428
    cmd[0] = 0x10;  // Channel 1, continuous mode, 12bit, MCP3428
    if(!brake.write(BRAKE_ADC_ADDR << 1, cmd, 1)){
        //printf("BRAKE1 Write Success!");
    }else
    {
        //printf("BRAKE1 Write Fail\r\n");
    }
    if(!brake.read(BRAKE_ADC_ADDR << 1, data, 2)){
        // printf("Data 0 = %d      Data 1 = %d     ", data[0], data[1]);
        brake_output_1 = (int16_t)((data[0] << 8) | data[1]);
        printf("BRAKE1! Analog In : %d  ", brake_output_1);
    
        brake_output_1_percent = (( ((float)(brake_output_1 - brake1_min)) / (float)(brake1_max-brake1_min) ) * 100);
        //printf("Pedal : %f    ", brake_output_1_percent);
        if(INVERT_BRAKE_INPUT)
        {
            brake_output_1_percent = 100 - brake_output_1_percent;
        }
        if(brake_output_1_percent < Deadzone)
        {
            brake_output_1_percent = 0;
        }else
        {
            brake_output_1_percent = ((brake_output_1_percent - Deadzone) / (100- Deadzone)) * 100;
        }
        if(brake_output_1_percent > 100)
        {
            brake_output_1_percent = 100;
        }
        printf("Percentage : %f     ", brake_output_1_percent);
    }
    else
    {
        printf("No brake read\r\n");
    }
    
    wait(0.04);
    cmd[0] = 0;
    data[0] = 0; data[1] = 0;

    //Read from Channel 2 of the MCP428
    cmd[0] = 0x30;  // Channel 2, continuous mode, 12bit, MCP3428
    if(!brake.write(BRAKE_ADC_ADDR << 1, cmd, 1)){
        // printf("BRAKE1 Write Success!");
    }else
    {
        //printf("BRAKE2 Write Fail\r\n");
    }
    if(!brake.read(BRAKE_ADC_ADDR << 1, data, 2)){
        // printf("Data 0 = %d      Data 1 = %d     ", data[0], data[1]);
        brake_output_2 = (int16_t)((data[0] << 8) | data[1]);
        printf("BRAKE2! Analog In : %d  ", brake_output_2);

        brake_output_2_percent = (( ((float)(brake_output_2 - brake2_min)) / (float)(brake2_max-brake2_min) ) * 100);
        //printf("Pedal : %f    ", brake_output_2_percent);
        if(INVERT_BRAKE_INPUT)
        {
            brake_output_2_percent = 100 - brake_output_2_percent;
        }
        if(brake_output_2_percent < Deadzone)
        {
            brake_output_2_percent = 0;
        }else
        {
            brake_output_2_percent = ((brake_output_2_percent - Deadzone) / (100- Deadzone)) * 100;
        }
        if(brake_output_2_percent > 100)
        {
            brake_output_2_percent = 100;
        }
        printf("Percentage : %f\r\n", brake_output_2_percent);
    }

    brake_avg_percent = (brake_output_1_percent + brake_output_2_percent)/2;
}

// Initial setup on power on
void Setup(){
    can1.frequency(250000);
    //can1.attach(&CAN1_Receive);                                           //Handle CAN1 Receive
    can2.frequency(250000);
    //can2.attach(&CAN2_Receive);                                           //Handle CAN2 Receive
    
    ticker_heartbeat.attach(&Heartbeat, BRAKE_HEARTRATE);           //Handle heartbeat every second
    ticker_can_transmit.attach(&CAN1_Transmit, CAN_BROADCAST_INTERVAL); //Transmit on CAN1 at the set interval

    //BSPD_Test.mode(PullUp);
    //printf("Setup\r\n");
    
    // pull brake calibration from usb flash
    char line[20];
    fgets(line, sizeof(line), stream);
    sscanf(line, "%d %d", &brake1_min, &brake1_max);
    fgets(line, sizeof(line), stream);
    sscanf(line, "%d %d", &brake2_min, &brake2_max);
    fclose(stream);
//        FILE *fp = fopen("/local/out.txt", "w");  // Open "out.txt" on the local file system for writing
//    fprintf(fp, "Hello World!");
//    fclose(fp);

}

int main(){
    Setup();
    while(1)
    {   
        Read_Brake_Pedal();
        //printf("BSPD: %d\r\n", BSPD_Test.read());
    }
}

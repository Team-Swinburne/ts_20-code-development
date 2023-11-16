
// TEAM SWINBURNE - TS19
// DASH BOARD
// MICHAEL COCHRANE
//-----------------------------------------------
//Pins
//5: MOSI
//6: MISO
//7: SCK
//8: GPIO
//9: SDA/TX             - CAN1 RX
//10: SCL/RX            - CAN1 TX
//11: MOSI2
//12: MISO2
//13: SCK/TX
//14: RX
//15: ADC1
//16: ADC2 
//17: ADC3
//18: ADC4/AOut
//19: ADC5 
//20: ADC6
//21: PWM1              - BSPD LED          
//22: PWM2              - AMS LED
//23: PWM3              - IMD LED
//24: PWM4              - PDOC LED
//25: PWM5
//26: PWM6
//27: SCL/RX
//28: SDA/TX
//29: CANTX             - CAN2 TX
//30: CANRX             - CAN2 RX
//-----------------------------------------------

#include <mbed.h>
#include "LCD_DISCO_F469NI.h"
#include <math.h>
#include "can_addresses.h"

CAN                     can2(PB_8, PB_9);
DigitalOut              can1_rx_led(LED1);
DigitalOut              can1_tx_led(LED2); 
CAN                     can1(PB_5, PB_13);
DigitalOut              can2_rx_led(LED3);
DigitalOut              can2_tx_led(LED4);
CANMessage              can1_msg;
CANMessage              can2_msg;

DigitalOut              BSPD_led(D0);
DigitalOut              AMS_led(D2);
DigitalOut              IMD_led(D1);
DigitalOut              PDOC_led(D3);
DigitalInOut            multi_led1(D12);
DigitalInOut            multi_led2(D11);
DigitalInOut            multi_led3(D10);
DigitalInOut            multi_led4(D9);

LCD_DISCO_F469NI lcd;

Ticker                  ticker_heartbeat;
Ticker                  ticker_can_transmit;
Ticker                  ticker_display_update;

int16_t                 heartbeat_state = 0;
int                     heartbeat_counter = 0;
int                     ams_state = 0;
int                     ams_prev_state = 0;
bool                    apps_disagree = 0;
bool                    trailbraking_active = 0;
bool                    precharge_pressed = 0;
bool                    drive_pressed = 0;
bool                    BSPD_error = 0;
bool                    AMS_error = 0;
bool                    IMD_error = 0;
bool                    PDOC_error = 0;
int16_t                 motor_speed = 0;
float                   max_accum_temp = 0;
uint16_t                rineheart_voltage = 0;
uint16_t                motor_highest_temp = 0;
uint16_t                rineheart_highest_temp = 0;
float                   percent_accum_temp = 95;
float                   percent_rineheart_temp = 76;
float                   percent_motor_temp = 20;
float                   percent_rineheart_voltage = 80;
int                     multi_index = 0;

// Constants
const float             BAR_LENGTH = (lcd.GetXSize() * 0.7 - 4);
const float             BAR_START =  (lcd.GetXSize() * 0.1 + 2);
#define Volt_Min                    380
#define Volt_Max                    592
#define Acc_Temp_Min                0
#define Acc_Temp_Max                80
#define Rineheart_Temp_Min          0
#define Rineheart_Temp_Max          80
#define Motor_Temp_Min              0
#define Motor_Temp_Max              80
#define Motor_Speed_Min             0
#define Motor_Speed_Max             6000
const int LED_ARRAY_DIRECTIONS[12][4] = {
    {1,1,0,0},
    {1,1,0,0},
    {1,0,1,0},
    {1,0,1,0},
    {0,1,1,0},
    {1,0,0,1},
    {0,1,1,0},
    {1,0,0,1},
    {0,1,0,1},
    {0,1,0,1},
    {0,0,1,1},
    {0,0,1,1}
};
const int LED_ARRAY_OUTPUTS[12][4] = {
    {0,1,0,0},
    {1,0,0,0},
    {0,0,1,0},
    {1,0,0,0},
    {0,0,1,0},
    {0,0,0,1},
    {0,1,0,0},
    {1,0,0,0},
    {0,0,0,1},
    {0,1,0,0},
    {0,0,0,1},
    {0,0,1,0}
};

// Intervals
#define DASH_HEARTRATE              1           // 1 Hz
#define DASH_UPDATERATE             0.5         // 10 Hz
#define CAN_BROADCAST_INTERVAL      0.005       // 200 Hz

//-----------------------------------------------
//Heartbeat - Broadcasts that the AMS is still alive and functional with a counter so we know when it has failed if it does
void Heartbeat(){
    heartbeat_counter++;
    char TX_data[2] = {(char)heartbeat_state, (char)heartbeat_counter};
    if(can1.write(CANMessage(CAN_DASH_BASE_ADDRESS, &TX_data[0], 2))) 
    {
        printf("Heartbeat Success! State: %d Counter: %d\n", heartbeat_state, heartbeat_counter);
    }else
    {
        printf("Hearts dead :(\r\n");
    }
}

// Receive CAN1 Data
void CAN1_Receive(){
    can1_rx_led = !can1_rx_led;
    if (can1.read(can1_msg)) 
    {
        switch(can1_msg.id) 
        {
            case CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS+TS_HEARTBEAT_ID: 
                ams_state = can1_msg.data[0];
                break;
            case CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS+TS_ERROR_WARNING_ID:
                AMS_led = !can1_msg.data[4];
                PDOC_led = !can1_msg.data[5];
                IMD_led = !can1_msg.data[6];
            break;
            case CAN_MOTEC_THROTTLE_CONTROLLER_BASE_ADDRESS+TS_DIGITAL_1_ID:
                precharge_pressed = can1_msg.data[0];
                drive_pressed = can1_msg.data[1];
            break;
            case CAN_MOTEC_THROTTLE_CONTROLLER_BASE_ADDRESS+TS_ERROR_WARNING_ID:
                apps_disagree = can1_msg.data[0];
                trailbraking_active = can1_msg.data[1];
            break;
            case CAN_TEMP_MODULE_BASE_ADDRESS+TS_ANALOGUE_1_ID:   
                max_accum_temp = (float)can1_msg.data[1];
            break;
            case RMS_VOLTAGE_INFO: 
                rineheart_voltage = (can1_msg.data[0] | (can1_msg.data[1] << 8))/10;
            break;
            case RMS_TEMPERATURE_SET_2:
                rineheart_highest_temp = can1_msg.data[0] | (can1_msg.data[1] << 8);
            break;
            case RMS_TEMPERATURE_SET_3:
                motor_highest_temp = can1_msg.data[4] | (can1_msg.data[5] << 8);
            break;
            case RMS_MOTOR_POSITION_INFO:
                motor_speed = can1_msg.data[2] | (can1_msg.data[3] << 8);
            break;
            case CAN_BRAKE_MODULE_BASE_ADDRESS+TS_DIGITAL_1_ID:
                BSPD_led = !can1_msg.data[4];
            break;
        }
    }
}

uint32_t Get_Bar_Colour(float percentage){
    if(percentage > 90)
    {
        return LCD_COLOR_RED;
    }else if(percentage > 80)
    {
        return LCD_COLOR_ORANGE;
    }else if(percentage > 60)
    {
        return LCD_COLOR_YELLOW;
    }else if(percentage > 20)
    {
        return LCD_COLOR_GREEN;
    }else
    {
        return LCD_COLOR_BLUE;
    }
}

uint32_t Get_Volt_Bar_Colour(float percentage){
    if(percentage > 40)
    {
        return LCD_COLOR_GREEN;
    }else if(percentage > 20)
    {
        return LCD_COLOR_YELLOW;
    }else if(percentage > 10)
    {
        return LCD_COLOR_ORANGE;
    }else
    {
        return LCD_COLOR_RED;
    }
}

void Update_LED_Array(int LED_Select) {

    if(LED_ARRAY_DIRECTIONS[LED_Select][0] == 1)
    {
        multi_led1.output();
        multi_led1 = LED_ARRAY_OUTPUTS[LED_Select][0];
    }else
    {
        multi_led1.input();
    }
    if(LED_ARRAY_DIRECTIONS[LED_Select][1] == 1)
    {
        multi_led2.output();
        multi_led2 = LED_ARRAY_OUTPUTS[LED_Select][1];
    }else
    {
        multi_led2.input();
    }
    if(LED_ARRAY_DIRECTIONS[LED_Select][2] == 1)
    {
        multi_led3.output();
        multi_led3 = LED_ARRAY_OUTPUTS[LED_Select][2];
    }else
    {
        multi_led3.input();
    }
    if(LED_ARRAY_DIRECTIONS[LED_Select][3] == 1)
    {
        multi_led4.output();
        multi_led4 = LED_ARRAY_OUTPUTS[LED_Select][3];
    }else
    {
        multi_led4.input();
    }
}

void Update_Display(){
    // TOP TEXT
    char text[40];
    sprintf(text, "Run Time: %d", heartbeat_counter);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.019, LINE(1), (uint8_t *)text, LEFT_MODE);
    lcd.SetTextColor(LCD_COLOR_BLACK);
    if(ams_state != ams_prev_state)
    {
        lcd.FillRect(
                    lcd.GetXSize() * 0.25,
                    LINE(2) - 1, 
                    lcd.GetXSize() * 0.6, 
                    LINE(1) + 2
                    );
    }
    ams_prev_state = ams_state;
    lcd.SetTextColor(LCD_COLOR_WHITE);
    switch(ams_state) 
    {
        case 0:
            strcpy(text, "AMS State: 0 Idle");
            break;
        case 1:
            strcpy(text, "AMS State: 1 Waiting for Precharge");
            break;
        case 2:
            strcpy(text, "AMS State: 2 Precharging");
            break;
        case 3:
            strcpy(text, "AMS State: 3 Waiting for Drive");
            break;
        case 4:
            strcpy(text, "AMS State: 4 Drive");
            break;
        case 7:
            strcpy(text, "AMS State: 7 Error");
            break;
    }
    lcd.DisplayStringAt(lcd.GetXSize() * 0.02, LINE(2), (uint8_t *)text, LEFT_MODE);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.02, LINE(1), (uint8_t *)"TS_19", RIGHT_MODE);

    // MOTOR TEMP BAR
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.DrawRect(lcd.GetXSize() * 0.1, LINE(5), lcd.GetXSize() * 0.7, LINE(3));
    lcd.SetTextColor(Get_Bar_Colour(percent_motor_temp));
    lcd.FillRect(
                    BAR_START,
                    LINE(5) + 2, 
                    BAR_LENGTH * (percent_motor_temp/100.0), 
                    LINE(3) - 4
                    );
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.FillRect(
                    BAR_START + BAR_LENGTH - (BAR_LENGTH * ((100 - percent_motor_temp)/100.0)), 
                    LINE(5) + 2, 
                    BAR_LENGTH - (BAR_LENGTH * (percent_motor_temp/100.0)), 
                    LINE(3) - 4
                    );
    lcd.SetTextColor(LCD_COLOR_WHITE);
    sprintf(text, "%d", (int)floor(motor_highest_temp/10.0));
    lcd.DisplayStringAt(lcd.GetXSize() * 0.15, LINE(4), (uint8_t *)"Motor Temp", LEFT_MODE);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.02, LINE(6), (uint8_t *)text, LEFT_MODE);

    // RINEHEART TEMP BAR
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.DrawRect(lcd.GetXSize() * 0.1, LINE(10), lcd.GetXSize() * 0.7, LINE(3));
    lcd.SetTextColor(Get_Bar_Colour(percent_rineheart_temp));
    lcd.FillRect(
                    BAR_START,
                    LINE(10) + 2, 
                    BAR_LENGTH * (percent_rineheart_temp/100.0), 
                    LINE(3) - 4
                    );
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.FillRect(
                    BAR_START + BAR_LENGTH - (BAR_LENGTH * ((100 - percent_rineheart_temp)/100.0)), 
                    LINE(10) + 2, 
                    BAR_LENGTH - (BAR_LENGTH * (percent_rineheart_temp/100.0)), 
                    LINE(3) - 4
                    );
    lcd.SetTextColor(LCD_COLOR_WHITE);
    sprintf(text, "%d", (int)floor(rineheart_highest_temp/10.0));
    lcd.DisplayStringAt(lcd.GetXSize() * 0.15, LINE(9), (uint8_t *)"Rineheart Temp", LEFT_MODE);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.02, LINE(11), (uint8_t *)text, LEFT_MODE);

    // MOTOR TEMP BAR
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.DrawRect(lcd.GetXSize() * 0.1, LINE(15), lcd.GetXSize() * 0.7, LINE(3));
    lcd.SetTextColor(Get_Bar_Colour(percent_accum_temp));
    lcd.FillRect(
                    BAR_START,
                    LINE(15) + 2, 
                    BAR_LENGTH * (percent_accum_temp/100.0), 
                    LINE(3) - 4
                    );
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.FillRect(
                    BAR_START + BAR_LENGTH - (BAR_LENGTH * ((100 - percent_accum_temp)/100.0)), 
                    LINE(15) + 2, 
                    BAR_LENGTH - (BAR_LENGTH * (percent_accum_temp/100.0)), 
                    LINE(3) - 4
                    );
    lcd.SetTextColor(LCD_COLOR_WHITE);
    sprintf(text, "%d", (int)floor(max_accum_temp));
    lcd.DisplayStringAt(lcd.GetXSize() * 0.15, LINE(14), (uint8_t *)"Accumulator Temp", LEFT_MODE);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.02, LINE(16), (uint8_t *)text, LEFT_MODE);

    // BATTERY VOLTAGE BAR
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.DrawRect(lcd.GetXSize() * 0.85, LINE(5), lcd.GetXSize() * 0.08, LINE(11));
    lcd.SetTextColor(Get_Volt_Bar_Colour(percent_rineheart_voltage));
    lcd.FillRect(
                    lcd.GetXSize() * 0.85 + 2, 
                    (LINE(5) + 2) + ((LINE(11) - 4) * ((100 - percent_rineheart_voltage)/100.0)), 
                    lcd.GetXSize() * 0.08 - 4, 
                    (LINE(11) - 4) - ((LINE(11) - 4) * ((100 - percent_rineheart_voltage)/100.0))
                    );
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.FillRect(
                    lcd.GetXSize() * 0.85 + 2, 
                    (LINE(5) + 2), 
                    lcd.GetXSize() * 0.08 - 4, 
                    (LINE(11) - 4) - ((LINE(11) - 4) * (percent_rineheart_voltage/100.0))
                    );
    lcd.SetTextColor(LCD_COLOR_WHITE);
    sprintf(text, "%d", rineheart_voltage);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.88, LINE(4), (uint8_t *)"V", LEFT_MODE);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.87, LINE(17), (uint8_t *)text, LEFT_MODE);

    // APPS DISAGREE BAR
    if(apps_disagree) {
        lcd.SetTextColor(LCD_COLOR_BLUE);
    }else
    {
        lcd.SetTextColor(LCD_COLOR_BLACK);
    }
    lcd.FillRect(0, lcd.GetYSize() * 0.95, lcd.GetXSize() * 0.5, lcd.GetYSize() * 0.05);
    
    // TRAILBRAKING/APPS BAR
    if(trailbraking_active) {
        lcd.SetTextColor(LCD_COLOR_BLUE);
    }else
    {
        lcd.SetTextColor(LCD_COLOR_BLACK);
    }
    lcd.FillRect(lcd.GetXSize() * 0.5, lcd.GetYSize() * 0.95, lcd.GetXSize() * 0.5, lcd.GetYSize() * 0.05);

    // DRIVE/PRECHARGE BUTTON PRESSED BARS
    if(precharge_pressed) {
        lcd.SetTextColor(LCD_COLOR_RED);
    }else if(drive_pressed) {
        lcd.SetTextColor(LCD_COLOR_GREEN);
    } else
    {
        lcd.SetTextColor(LCD_COLOR_BLACK);
    }
    lcd.FillRect(0, 0, lcd.GetXSize() * 0.01, lcd.GetYSize());
    lcd.FillRect(0, 0, lcd.GetXSize(), lcd.GetYSize() * 0.02);
    lcd.FillRect(lcd.GetXSize() - (lcd.GetXSize() * 0.01), 0, lcd.GetXSize() * 0.01, lcd.GetYSize());
    lcd.FillRect(0, lcd.GetYSize() - (lcd.GetYSize() * 0.02), lcd.GetXSize(), lcd.GetYSize() * 0.02);

    // TOP LED ARRAY
    int LED_Select = (int)(((motor_speed - Motor_Speed_Min)/Motor_Speed_Max) * 11);
    Update_LED_Array(LED_Select);
}

void Check_Percentages() {
    percent_accum_temp = (((max_accum_temp - Acc_Temp_Min)/Acc_Temp_Max) * 100.0);
    percent_rineheart_voltage = (((rineheart_voltage - Volt_Min)/Volt_Max) * 100.0);
    percent_rineheart_temp = ((floor(rineheart_highest_temp/10.0) - Rineheart_Temp_Min) / Rineheart_Temp_Max) * 100;
    percent_motor_temp = ((floor(motor_highest_temp/10.0) - Motor_Temp_Min) / Motor_Temp_Max) * 100;
    if(percent_accum_temp > 100) {
        percent_accum_temp = 100;
    }else if(percent_accum_temp < 0.5) {
        percent_accum_temp = 0.5;
    }

    if(percent_rineheart_temp > 100) {
        percent_rineheart_temp = 100;
    }else if(percent_rineheart_temp < 0.5) {
        percent_rineheart_temp = 0.5;
    }

    if(percent_motor_temp > 100) {
        percent_motor_temp = 100;
    }else if(percent_motor_temp < 0.5) {
        percent_motor_temp = 0.5;
    }

    if(percent_rineheart_voltage > 100) {
        percent_rineheart_voltage = 100;
    }else if(percent_rineheart_voltage < 0.5) {
        percent_rineheart_voltage = 0.5;
    }
}
void BSOD() {

}

void Setup() {
    ticker_heartbeat.attach(&Heartbeat, DASH_HEARTRATE);                //Handle heartbeat every second
    //BSOD();
    //wait(8);
    lcd.Clear(LCD_COLOR_BLACK);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font20);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.37, lcd.GetYSize() * 0.407, (uint8_t *)"Team", LEFT_MODE);
    BSP_LCD_SetFont(&Font24);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.45, lcd.GetYSize() * 0.4, (uint8_t *)"Swinburne", LEFT_MODE);
    BSP_LCD_SetFont(&Font16);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.49, lcd.GetYSize() * 0.4 + 22, (uint8_t *)"FORMULA SAE", LEFT_MODE);
    BSP_LCD_SetFont(&Font24);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.548, lcd.GetYSize() * 0.4 + 36, (uint8_t *)"ts_", LEFT_MODE);
    lcd.SetTextColor(LCD_COLOR_RED);
    lcd.DisplayStringAt(lcd.GetXSize() * 0.60, lcd.GetYSize() * 0.4 + 36, (uint8_t *)"19", LEFT_MODE);
    lcd.FillRect(
                    lcd.GetXSize() * 0.650, 
                    lcd.GetYSize() * 0.38, 
                    lcd.GetXSize() * 0.03, 
                    66
                    );
    wait_ms(100);
    PDOC_led = 1;
    wait_ms(10);
    for(int i = 0; i < 4; i++)
    {
        Update_LED_Array(i);
        wait_ms(10);
    }
    AMS_led = 1;
    for(int i = 4; i < 8; i++)
    {
        Update_LED_Array(i);
        wait_ms(10);
    }
    IMD_led = 1;
    for(int i = 8; i < 12; i++)
    {
        Update_LED_Array(i);
        wait_ms(10);
    }
    BSPD_led = 1; 

    can1.frequency(500000);
    can1.attach(&CAN1_Receive);                                         //Handle CAN1 Receive

    BSP_LCD_SetFont(&Font24);
    PDOC_led = 0;
    AMS_led = 0;
    IMD_led = 0;
    BSPD_led = 0;
    Update_LED_Array(0);

    wait_ms(2000);
    lcd.Clear(LCD_COLOR_BLACK);

}

int main() {
    Setup();
    while(1) {
        Check_Percentages();
        Update_Display();
    }
}
// TEAM SWINBURNE - TS STANDARD CANBUS DESIGN RULES
// BEN MCINNES, NAM TRAN, LUKE DELTON, PATRICK CURTAIN
// REVISION 0.1 (21/06/21)

/***************************************************************************
    can_addresses.h

    Derived from https://docs.google.com/spreadsheets/d/1s9SunyjSyh4vkgJb64f5PWy6DnKNB9WHros9Rx-3RJg/edit?usp=sharing

    INTRO
    Team Swinburne devices shall make use of a standard format for increasing 
    the readability of the CAN. Note that this is not a networking stack, nor
    the framework for a networking stack, but rather a simple oranisation of
    the vehicle's data link layer for ease of interpretation. 

    The TS_22 vehicle shall included one global CANBUS, and two CANBUS dedicated to a link between 
    the throttle controller and inverters.

    Revision     Date          Comments
    --------   ----------     ------------
    0.0        21/06/2021     Initial address space
    0.1        22/03/2021     Change address space to 50 instead of 100
****************************************************************************/

#define CANBUS_FREQUENCY							BR500K

#define TEST_ADDRESS                                0x600
/*



DEVICE ADDRESS SPACE

Each device that is designed by the team will have a three byte long base address.
The first of these bytes is used to signify the high level priority of the 
device in relation to the vehicle. Devices, such as the BMS, Brake, and throttle,
will be assigned the lowest address, as these will take precedence during the 
CANBUS arbiration phase.
The second byte signifies the version of the device. This is intended to future
proof boards, and provide space for 15 additional devices where multiple may be 
required. Should more address space be required for additional devices, this can 
be subdivided.
The thrid byte is the message code, an address space that is directly related 
to the device.
*/

//CAN ADDRESSES
#define CAN_ORION_BMS_BASE_ADDRESS                  0x100
#define ORION_TEMPERATURE_ID                        0x105
#define CAN_BRAKE_MODULE_BASE_ADDRESS               0x150
#define CAN_AMK_MOTOR_CONTROLLER_BASE_ADDRESS       0x200
#define UCM3_SP_CONTROL                             0x262
#define UCM4_RL_CONTROL                             0x263
#define UCM5_RR_CONTROL                             0x264
#define Brake_light                                 0x253  
#define CAN_MOTEC_THROTTLE_CONTROLLER_BASE_ADDRESS  0x250
#define CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS       0x300
#define CAN_PRECHARGE_CONTROLLER_DIGITAL_1_ID       0x302
#define CAN_UCM1_BASE_ADDRESS                       0x360 
#define CAN_UCM2_BASE_ADDRESS                       0x370 
#define CAN_UCM3_BASE_ADDRESS                       0x380 
#define CAN_UCM4_BASE_ADDRESS                       0x390 
#define CAN_UCM5_BASE_ADDRESS                       0x400 
#define CAN_ORION_TEMP_MODULE_BASE_ADDRESS          0x450
#define CAN_DISCHARGE_MODULE_BASE_ADDRESS           0x500
#define CAN_DASH_BASE_ADDRESS                       0x550


// Miscillatious
#define CAN_ORION_BMS_RINEHART_LIMITS				0x202
#define CAN_TC_CHARGER_STATUS_ID					0x18FF50E5

//Max Inverter Temp, max motor temp (left and right side)
#define CAN_INVERTER_PASSTHROUGH   0x256



 
/*

MESSAGE CODE STRUCTURE

The first six message codes are assigned to specific device outputs and should be taken
into consideration when designing the devices output. 
    Heartbeat: This is used to reflect the devices status and should iterate once per second,
        counting the time since the last reset event. This serves as a simple debugging tool
        and allows for watchdogs to be attached between devices with ease.
    
    Errors/Warnings: The first 4 bytes are for semiphores relating to the health of the device, 
        errors being flags that should stop the vehicle, and warnings being those that should 
        be relayed to the driver. These codes should use bitwise logic to maximise the information.
        To assist in human readability, the highest prority messages should be assigned to the final 
        four bytes of the message, since these will be easier to read in hexidecimal format.
    
    Digital 1 & 2: A space for 16 bytes of potential semiphores. These can be broken up into 128 bits,
        if required.
    
    Analogue 1 & 2: A space for 16 bytes worth of analogue values. These can be combined into pairs for
        16 bit resolution. Note that unsigned, and big endian is the preferred 16 bit method due to 
        readability concerns, however both are acceptable, provided the database is updated. 

For the sake of simplicity, these can be left as 8 byte messages, however once an order is sorted,
care should be taken to reduce the byte count and increase network throughput.

The message code structures are broken down as follows:

M.ID    Message Name        Byte 0          Byte 1          Byte 2              Byte 3          Byte 4          Byte 5          Byte 6          Byte 7
0.      Heartbeat           State           Counter         PCB Temperature     Compiled Date
1.      Errors/Warnings     Error[0]        Error[1]        Warning[0]          Warning[1]      SPARE_1         SPARE_2         SPARE_3         SPARE_4
2.      Digital 1           Digital[0]      Digital[1]      Digital[2]          Digital[3]      Digital[4]      Digital[5]      Digital[6]      Digital[7]
3.      Digital 2           Digital[0]      Digital[1]      Digital[2]          Digital[3]      Digital[4]      Digital[5]      Digital[6]      Digital[7]
4.      Analogue 1          Analogue[0]     Analogue[1]     Analogue[2]         Analogue[3]     Analogue[4]     Analogue[5]     Analogue[6]     Analogue[7]
5.      Analogue 2          Analogue[0]     Analogue[1]     Analogue[2]         Analogue[3]     Analogue[4]     Analogue[5]     Analogue[6]     Analogue[7]

*/

// The following enumeration can be appended to each base address.
// eg. CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS + TS_DIGITAL_1_ID

typedef enum TS_STD_CAN_MESSAGES{
    TS_HEARTBEAT_ID,
    TS_ERROR_WARNING_ID,
    TS_DIGITAL_1_ID,
    TS_DIGITAL_2_ID,
    TS_ANALOGUE_1_ID,
    TS_ANALOGUE_2_ID,
} ts_std_can_messages_t;

// Enumerations can also be used to parse each of these in a meaningful way. A basic template is included for that sake 
// in this document, and should be included within the application to relfect the signals present.

enum CAN_HEART_SIGNALS {
    HEART_STATE,
    HEART_COUNTER,
    HEART_PCB_TEMP,
    HEART_HARDWARE_REV,
    HEART_COMPILE_DATE,
    HEART_COMPILE_TIME,
};


// The error and warning messages can be handled within a similar mannor, turning a byte array into a bit array
// with a simple function.

    // uint8_t array_to_uint8(bool arr[], int count){
    //     int ret = 0;
    //     int tmp;
    //     for (int i = 0; i < count; i++) {
    //         tmp = arr[i];
    //         ret |= tmp << (count - i - 1);
    //     }
    //     return ret;
    // }

    // ... then within the error check function.

    // uint8_t error_check(){
    //     bool error_code[8];

    //     error_code[ERROR_AMS_OK] 	= !orion.get_AMS_ok();
    //     error_code[ERROR_PDOC_OK] 	= !pdoc.get_pdoc_ok();
    //     error_code[ERROR_IMD_OK] 	= !imd.get_IMD_ok();
    //     error_code[ERROR_ORION_OK] 	= !orion.check_orion_safe();
        
    //     error_code[ERROR_SPARE_4] = false;
    //     error_code[ERROR_SPARE_5] = false;
    //     error_code[ERROR_SPARE_6] = false;
    //     error_code[ERROR_SPARE_7] = false;

    //     return array_to_uint8(error_code, 8);
    // }

/*

MESSAGE TIMINGS

Care must be taken to avoid overloaded the network, and decreasing the amount of jitter
that may be experienced by nodes. The following should serve as a guide for recommended 
message periods that will provided an acceptable amount of network traffic, whilst 
still relaying infomation about the vehicle. Care should always be taken to ensure 
that each signal is being sampled at a the nyquest frequency. Some additional tricks may need
to be used to increase the potential sampling frequencies, such as packing in a time-multiplexed
signal into a single frame.

Message Description                                 Minimum Period (ms)
Heartbeats                                          1000
Error/Warning (Information Only)                    500
Error/Warning (Safety Critical/Watchdog Required)   100
Digital/Analogue (Driver Reaction Times)            10
Digital/Analogue (High Nyquest speeds required)     50
Digital/Analogue (Medium frequency acceptable)      200
Digital/Analogue (Low Priority)                     500

*/

// CANBUS Intervals (in milliseconds)

#define INTERVAL_HEARTBEAT                      1000
#define INTERVAL_DRIVER_REACTION                10
#define INTERVAL_HIGH_PRIORITY                  50
#define INTERVAL_ERROR_WARNING_CRITICAL         100
#define INTERVAL_MEDIUM_PRIORITY                200
#define INTERVAL_ERROR_WARNING_LOW_PRIORITY     500        

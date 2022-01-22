// TEAM SWINBURNE - TS19
// CAN IDS
// TEAM SWINBURNE
//-----------------------------------------------

//AMS
#define AMS_HEARTBEAT_ID                            0x300
#define AMS_DATA_ID       	                     	0x301
#define AMS_DATA2_ID                                0x302

//Throttle
#define THROTTLE_HEARTBEAT_ID						0x304
#define THROTTLE_SENSORS_ID							0x305
#define THROTTLE_OUTPUT_ID							0x306
#define THROTTLE_ERRORS_ID							0x307

//Brake
#define BRAKE_HEARTBEAT_ID							0x308
#define BRAKE_SENSORS_ID							0x309
#define BRAKE_SAFETY_ID								0x30A

//Temp Sensor
#define TEMP_HEARTBEAT_ID                   		0x310
#define TEMP_SUMMARY_ID                     		0x311
#define TEMP_BANK_1_ID                      		0x312
#define TEMP_BANK_2_ID                      		0x313
#define TEMP_BANK_3_ID                      		0x314
#define TEMP_BANK_4_ID                      		0x315
#define TEMP_BANK_5_ID                      		0x316
#define TEMP_BANK_6_ID                      		0x317
#define TEMP_BANK_7_ID                      		0x318
#define TEMP_BANK_8_ID                      		0x319
#define TEMP_ERRORS_ID                      		0x31A

//Dash
#define DASH_HEARTBEAT_ID                   		0x320

//Pressure
#define PRESSURE_OUTPUT_ONE_ID                      0X330
#define PRESSURE_OUTPUT_TWO_ID                      0X331

//Discharge
#define DISCHARGE_DATA_ID							0x340

//Orion
#define ORION_DATA_ID        						0x20B	 		//orion battery data message
#define ORION_CURRENT_ID     						0x70B			//orion current

//RMS
#define RMS_ID                              		0x200          	//Base address
#define RMS_TEMPERATURE_SET_1               		RMS_ID + 0xA0
#define RMS_TEMPERATURE_SET_2        				RMS_ID + 0xA1
#define RMS_TEMPERATURE_SET_3        				RMS_ID + 0xA2
#define RMS_ANALOG_INPUT_VOLTAGES    				RMS_ID + 0xA3
#define RMS_DIGITAL_INPUT_STATUS     				RMS_ID + 0xA4
#define RMS_MOTOR_POSITION_INFO      				RMS_ID + 0xA5  	//used for reading speed of motor
#define RMS_CURRENT_INFO             				RMS_ID + 0xA6
#define RMS_VOLTAGE_INFO             				RMS_ID + 0xA7
#define RMS_FLUX_ID_IQ_INFO          				RMS_ID + 0xA8
#define RMS_INTERNAL_VOLTAGES        				RMS_ID + 0xA9
#define RMS_INTERNAL_STATES          				RMS_ID + 0xAA
#define RMS_FAULT_CODES              				RMS_ID + 0xAB
#define RMS_TORQUE_AND_TIMER_INFO    				RMS_ID + 0xAC
#define RMS_MODULATION_AND_FLUX_INFO 				RMS_ID + 0xAD
#define RMS_FIRMWARE_INFO            				RMS_ID + 0xAE
#define RMS_DIAG_DATA                				RMS_ID + 0xAF
#define RMS_COMMAND_MESSAGE          				RMS_ID + 0xC0  //Messages to Motor Controller - speed setting
#ifndef __UCM_3_INFO_H__
#define __CUM_3_INFO_H__

typedef struct HeartBeat_s{
  uint8_t Counter = 0;
  uint8_t State = 0;
}HeartBeat_struct;

//Enums for all the bytes in the heartbeat CAN message
typedef enum CAN_HEARTBEAT_SIGNALS{
  CAN_HEARTBEAT_STATE,
  CAN_HEARTBEAT_COUNTER,
  CAN_HEARTBEAT_PCB_TEMP,
  CAN_HEARTBEAT_HARDWARE_REVISION,
  CAN_HEARTBEAT_UCM_ID
} can_HEARTBEAT_signals_t;

#endif // __UCM_3_INFO_H__

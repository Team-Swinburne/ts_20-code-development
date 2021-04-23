import can
import time


# Constants
CANBUS_SPEED = 500000

THROTTLE_CONTROLLER_PERIPERAL_ID = 0x343

ORION_BMS_STATUS_ID = 0x200

def send_handler(bus, msg):
    try:
        bus.send(msg)
        print("Message sent on {}".format(bus.channel_info))
    except:
        print("Message not sent")

def send_precharge_request(bus):
    payload = [1,0,0,0,0,0,0,0]
    msg.can.Message(arbitration_id=THROTTLE_CONTROLLER_PERIPERAL_ID, data=payload)
    time.sleep(1)
    payload = [0,0,0,0,0,0,0,0]

def send_relay_status(bus):
    payload = [7,0,0,0,0,0,0,0]
    msg.can.Message(arbitration_id=ORION_BMS_STATUS_ID, data=payload)
    send_handler(bus, msg)
    time.sleep(0.2)

def setup():
    # may need to add serial=12093 <- or whatever number that is. 
    bus = can.interface.Bus(bustype='kvaser', channel=0, bitrate=CANBUS_SPEED)\
    
    return bus

def main():
    bus = setup()

    while(1):
        send_relay_status(bus)
        # Program loop

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        send_precharge_request()
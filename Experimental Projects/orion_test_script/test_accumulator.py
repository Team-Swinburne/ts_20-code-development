import tkinter as tk
import can
import time

import threading

# https://realpython.com/python-gui-tkinter/

# CANBUS SPEED
CANBUS_SPEED = 500000

# CANBUS ADDRESSES
THROTTLE_CONTROLLER_PERIPERAL_ID = 0x343

ORION_BMS_STATUS_ID = 0x180


# INERFACE OBJECTS

window = tk.Tk()

greeting = tk.Label(text="TS_21 ELECTRICAL TEST INTERFACE")

precharge_button = tk.Button(
    text="PRECHARGE",
    bg = "red",
    fg = "white"
)

drive_button = tk.Button(
    text="DRIVE",
    bg = "green",
    fg = "white"
)

def send_handler(bus, msg):
    try:
        bus.send(msg,timeout=None)
        print(msg.data)
        print("Message sent on {}\r".format(bus.channel_info))
    except:
        print("Message not sent")

def throttle_interface(bus, msg):
    # DEFAULT PERIPERAL PAYLOAD
    payload = [0,0,0,0,0,0,0,0]
    if (precharge_button.get()):
        payload[0] = 1;
    if (drive_button.get()):
        payload[1] = 1;
    msg = can.Message(arbitration_id=THROTTLE_CONTROLLER_PERIPERAL_ID, data=payload)
    time.sleep(1)

def orion_interface(bus):
    payload = [7,0,0,0,0,0,0,0]
    msg = can.Message(arbitration_id=ORION_BMS_STATUS_ID, data=payload)
    send_handler(bus, msg)
    time.sleep(0.2)

def setup_canbus():
    # may need to add serial=12093 <- or whatever number that is. 
    bus = can.interface.Bus(bustype='kvaser', channel=0, bitrate=CANBUS_SPEED)\
    
    print(bus.get_stats())
    bus.flash(flash=True)

    return bus

def manage_window():

    greeting.pack()

    precharge_button.pack()
    drive_button.pack()

    window.mainloop()





try:
    threading.Thread(target=setup_canbus)
    threading.Thread(target=manage_window)
except:
    print("Error! Cannot start new thread")
    
while 1:
    pass
import sys
import os

import time
import serial
import serial.tools.list_ports as port_list

# CONSTANTS
timestamp = time.strftime("%Y%m%d-%H%M%S")
filename = 'SAVE - ' + timestamp + '.csv'
savepath = './saves'

def printNewSection():
    print("*---------------------------------------------*")

def sniffPorts():
    ports = list(port_list.comports())
    print("The following ports are available!")
    for p in ports:
        print (p)

def readSerial(ser):
    print("Serial port opened, type CTL+C to exit!")
 
    # WILL REQUIRE ADMIN RIGHTS! Best just to make your own one
    # but that's just my opinion :D
    # os.mkdir(savepath)

    

    completename = os.path.join(savepath, filename) 
    f = open(completename, "w")

    ser.flush()
    
    

    while (ser.isOpen()):
        buf = ser.readline().decode('ascii')
        buf = buf.rstrip('\r')
        buf = buf.rstrip('\n')
        f.write(buf)
        print(buf)

def main():
    # This boi doesn't really need to be there, but it's usefulish?

    printNewSection()
    print()
    sniffPorts()
    print()
    printNewSection()
    print()

    try:
        sys.argv[1]
        port = sys.argv[1]
        
    except Exception as e:
	    print('Please add COMx as arguement!')
	    return 

    ser = serial.Serial(port, 9600)
    ser.write(str.encode("R"))
    readSerial(ser)
    ser.close()   

if __name__ == '__main__':
    main()
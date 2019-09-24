###############################################################################
#                                                                             #
# proc_data_f_gen_train_test_code_arduino.py  for Arduino                     #
#                                                                             #
###############################################################################
# Author:  Joao Nuno Carvalho                                                 #
# Data:    2019.09.22                                                         #
# License: MIT Open Source License                                            #
#                                                                             #
# Description: Lists the serial ports and connects to the serial port via     #
#              pySerial in order to save the WiFi training data from the      #
# ESP32 that should be running the program WiFiScan that is on this           #
# repository and also comes with the ESP32 WIFI demo programs.                #
# You have to configure the com port once and the name of the training        #
# datafile for each room. Monitor the data_file that is generated and stop it #
# when it reaches 20KB. To stop it press on the terminal a CTRL+C.            #
###############################################################################

# List the serial ports on the computer.
# You have to configure bellow to one of them.
import serial.tools.list_ports
print([comport.device for comport in serial.tools.list_ports.comports()])


# Collect data from serial port and write it to disk.
path = './/data_files//'

# STEP's to RUN:
# Uncomment or modify one of this lines for each room you would like.
# Um can change the name of the room but you have to maintain the ending part "_data.dat".
# To stop the program when the file reach's 20KB use Ctrl+C .

filename = 'room_A_data.dat'
#filename = 'room_B_data.dat'
#filename = 'room_C_data.dat'
#filename = 'room_D_data.dat'

# Configure the serial port to connect to you ESP32, note you can't be connected at the
# some time with the Arduino Serial Monitor or Serial Plotter.
my_ESP32_serial_port = '/dev/ttyUSB0'   # 'COM4' 

import serial
ser = serial.Serial(my_ESP32_serial_port, baudrate=115200)
f = open(path + filename, 'wb')
while (True):
    data = ser.read()
    f.write(data)
    f.flush()
ser.close()
f.close()

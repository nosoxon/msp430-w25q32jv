#!/usr/bin/env python3

import serial
import sys

port = '/dev/ttyACM0'

try:
    s = serial.Serial(port, 2400, timeout=0.05)
    s.baudrate = 9600
except:
    print(f'Opening serial port {port} failed')
    sys.exit(1)

s.flushInput()

with open('dump.bin', 'wb') as f:
    s.write('4194304 03000000\r'.encode())
    while True:
        data = s.read(1024)
        if len(data) > 0:
            f.write(data)

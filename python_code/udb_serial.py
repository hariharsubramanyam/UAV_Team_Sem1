#!/usr/bin/python
import serial
import time

def to16bit(x):
    return (x%(1<<16))

def to8bit(x):
    return (x%(1<<8))

def sendCommand(serialPort, throttle = 0, roll = 0, pitch = 0, yaw = 0, gear = 0, aux1 = 0,
        aux2 = 0):
    throttle = to16bit(throttle)
    msg = bytearray()
    msg.append(0)
    msg.append(0)
    msg.append((((throttle >> 8) & 0b00000111)))
    msg.append(to8bit(throttle))
    msg.append((((pitch >> 8) & 0b00000111 | 0b00001000)))
    msg.append(to8bit(pitch))
    msg.append((((roll >> 8) & 0b00000111 | 0b00010000)))
    msg.append(to8bit(roll))
    msg.append((((yaw >> 8) & 0b00000111 | 0b00011000)))
    msg.append(to8bit(yaw))
    msg.append((((gear >> 8) & 0b00000111 | 0b00100000)))
    msg.append(to8bit(gear))
    msg.append((((aux1 >> 8) & 0b00000111 | 0b00101000)))
    msg.append(to8bit(aux1))
    msg.append((((aux2 >> 8) & 0b00000111 | 0b00110000)))
    msg.append(to8bit(aux2))
    serialPort.write(msg)

ser = serial.Serial('/dev/ttyACM0',57600,timeout = 1)
while True:
    ftime = 0
    while ftime < 40:
        tVal = ftime * 20
        sendCommand(serialPort = ser, throttle = tVal, gear = 0x07ff)
        ftime += 1
        print tVal
        time.sleep(0.5)

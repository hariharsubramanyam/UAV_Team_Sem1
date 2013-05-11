#!/usr/bin/python
import pygame
import serial
import time

# Just a bit of boilerplate to start from next time

pygame.init()
pygame.joystick.init()

while not pygame.joystick.get_init():
    print "Joystick not yet initialized. Trying again..."
    pygame.joystick.init()

joy = pygame.joystick.Joystick(0)
joy.init()

def quadprint():
    while True:
        axislist = []
        pygame.event.get()
        for axis in range(joy.get_numaxes()):
            position = joy.get_axis(axis)
            print axis_convert(axis, position)
        pygame.time.wait(200)

def axis_convert(axis, value):
    if axis == 3:
        if value < 0:
            return 2000
        else:
            return 1000
    offset = [0.4433, 0.4846, -0.6701, 0.9794, 0.8866, 0.4949]
    mults = [1021, 982, -789, 1, 560, 1042]
    value += offset[axis]
    if value < 0 and axis != 2:
        value = 0
    value *= mults[axis]
    return int(value + 1000)

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
def prog_loop()
    while True:
        t = []
        pygame.event.get()
        for axis in range(6):
            t.append(axis, axis_convert(joy.get_axis(axis)))
        sendCommand(serialPort = ser, throttle = t[2], roll = t[0], pitch =
                t[1], yaw = t[5], gear = t[4], aux1 = t[3], aux2 = 0)

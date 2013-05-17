#!/usr/bin/python
import pygame
import serial
import time

def quadprint():
    while True:
        axislist = []
        pygame.event.get()
        for axis in range(joy.get_numaxes()):
            position = joy.get_axis(axis)
            print axis_convert2(axis, position)
        pygame.time.wait(200)

def axis_convert(axis, value):
    if axis == 3:
        if value < 0:
            return 2000
        else:
            return 0
    offset = [0.4433, 0.4846, -0.6701, 0.9794, 0.8866, 0.4949]
    #offset = [0.4433, 0.4846, -0.6, 0.9794, 0.8866, 0.4949]
    mults = [1021, 982, -789, 1, 560, 1042]
    value += offset[axis]
    if value < 0 and axis != 2:
        value = 0
    value *= mults[axis]
    if axis == 2:
        return int(value * 2 + 65) 
    value += 1000
    if axis == 5:
        print value
    if value > 2000:
        return 2000
    elif value < 1000:
        return 1000
    else:
        return int(value)
    #return int(value + 1000)

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
    msg.append((((roll >> 8) & 0b00000111 | 0b00001000)))
    msg.append(to8bit(roll))
    msg.append((((pitch >> 8) & 0b00000111 | 0b00010000)))
    msg.append(to8bit(pitch))
    msg.append((((yaw >> 8) & 0b00000111 | 0b00011000)))
    msg.append(to8bit(yaw))
    msg.append((((gear >> 8) & 0b00000111 | 0b00100000)))
    msg.append(to8bit(gear))
    msg.append((((aux1 >> 8) & 0b00000111 | 0b00101000)))
    msg.append(to8bit(aux1))
    msg.append((((aux2 >> 8) & 0b00000111 | 0b00110000)))
    msg.append(to8bit(aux2))
    serialPort.write(msg)

def prog_loop():
    while True:
        t = []
        pygame.event.get()
        for axis in range(6):
            val = axis_convert(axis, joy.get_axis(axis))
            t.append(val)
        print t[3]
        sendCommand(serialPort = ser, throttle = t[2], roll = t[0], pitch =
                t[1], yaw = t[5], gear = t[3], aux1 = 0, aux2 = 0)
        #pygame.time.wait(50)

def step():
    fTime = 0
    while True:
        tVal = fTime * 40
        print "Throttle value: ", tVal
        sendCommand(serialPort = ser, throttle = tVal, gear = 2000)
        if fTime > 40:
            fTime = 0
        else:
            fTime += 1
        pygame.time.wait(50)

def axis_convert2(axis, value):
    if axis == 3:
        if value < 0:
            return 2000
        else:
            return 0
    #offset = [0.4433, 0.4846, -0.6701, 0.9794, 0.8866, 0.4949]
    #offset = [0.4433, 0.4846, -0.6, 0.9794, 0.8866, 0.4949]
    #order roll - pitch - throttle - gear - yaw
    # used to be [-200, 0, 50, 0, -200]
    tune = [0, 0, 0, 0, 0, -124]
    mults = [-1021 / 1, 982 / 1, -789, 1, 560 / 2, -1042]
    #value += offset[axis]
    value *= mults[axis]
#    if axis == 2 or 1:
    value = int(value * 2) 
    if value > 1000:
        value = 1000
    elif value < -1000:
        value = -1000
    
    value = int(value + 1024 + tune[axis])

    if value > 1950:
        value = 1950
    elif value < 50:
        value = 50

    return value

def prog_loop2():
    while True:
        t = []
        pygame.event.get()
        for axis in range(6):
            val = axis_convert2(axis, joy.get_axis(axis))
            t.append(val)
        print t
        sendCommand(serialPort = ser, throttle = t[2], roll = t[0], pitch =
                t[1], yaw = t[5], gear = t[3], aux1 = 0, aux2 = 0)
        pygame.time.wait(50)


def quadprint2():
    while True:
        axislist = []
        pygame.event.get()
        for axis in range(joy.get_numaxes()):
            position = joy.get_axis(axis)
            print axis_convert2(axis, position)
        pygame.time.wait(200)

if __name__=='__main__':
    pygame.init()
    pygame.joystick.init()
    print "running"

    while not pygame.joystick.get_init():
        print "Joystick not yet initialized. Trying again..."
        pygame.joystick.init()

    joy = pygame.joystick.Joystick(0)
    joy.init()
    ser = serial.Serial('/dev/ttyUSB0',57600,timeout = 1)
    prog_loop2()
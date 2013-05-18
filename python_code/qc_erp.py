#!/usr/bin/python

# Uses Norbert Feurle's extended realtime plotter program.

# Imports
import pylab
import pygame
import serial
import Tkinter
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
import struct

# Variables

ser = serial.Serial('/dev/ttyACM0', 57600)

S_IDLE = 0
S_HEADER1 = 1
S_HEADER2 = 2
HEADER_BYTE_1 = 0xFF
HEADER_BYTE_2 = 0XFE
S_HEADER_SIZE = 3
S_HEADER_CMD = 4
inBuf = []
c_state = S_IDLE
INBUF_SIZE = 64
(dataSize, offset, checksum, cmd) = (0,0,0,0)
CMD_SONARS = 0xA1
CMD_AHRS = 0x01

Y_MIN = -32000
Y_MAX = 32000

NUM_SONARS = 4

# Plotter setup. Might need some rearrangement.

root = Tkinter.Tk()
root.wm_title("Extended Realtime Plotter")

xAchse=pylab.arange(0,100,1)
yAchse=pylab.array([0]*100)

fig = pylab.figure(1)
ax = fig.add_subplot(111)
ax.grid(True)
ax.set_title("Realtime Waveform Plot")
ax.set_xlabel("Time")
ax.set_ylabel("Amplitude")
ax.axis([0,100,Y_MIN,Y_MAX])

line1 = ax.plot(xAchse,yAchse,'-')
line2 = ax.plot(xAchse,yAchse,'.')
line3 = ax.plot(xAchse,yAchse,'+')
line4 = ax.plot(xAchse,yAchse,'*')
line5 = ax.plot(xAchse,yAchse,'^')
line6 = ax.plot(xAchse,yAchse,'*')

canvas = FigureCanvasTkAgg(fig, master=root)
canvas.show()
canvas.get_tk_widget().pack(side=Tkinter.TOP, fill=Tkinter.BOTH, expand=1)

toolbar = NavigationToolbar2TkAgg(canvas, root)
toolbar.update()
canvas._tkcanvas.pack(side=Tkinter.TOP, fill=Tkinter.BOTH, expand=1)

"""
values1 = [] # Redundant?
values2 = [] 
values3 = []
values4 = []
values5 = []
values6 = []
"""
values1 = [0 for x in range(100)]
values2 = [0 for x in range(100)]
values3 = [0 for x in range(100)]
values4 = [0 for x in range(100)]
values5 = [0 for x in range(100)]
values6 = [0 for x in range(100)]

button = Tkinter.Button(master=root, text='Quit', command=_quit)
button.pack(side=Tkinter.BOTTOM)

wScale = Tkinter.Scale(master=root,label="View Width:", from_=3,
        to=1000, sliderlength=30, length=ax.get_frame().get_window_extent().width,
        orient=Tkinter.HORIZONTAL)

wScale2 = Tkinter.Scale(master=root,label="Generation Speed:", from_=1, to=200,
        sliderlength=30,length=ax.get_frame().get_window_extent().width,
        orient=Tkinter.HORIZONTAL)

wScale2.pack(side=Tkinter.BOTTOM)
wScale.pack(side=Tkinter.BOTTOM)

wScale.set(100)
wScale2.set(wScale2['to']-10)

# Function definitions

def to16bit(x):
    return (x%(1<<16))


def to8bit(x):
    return (x%(1<<8))


def read_and_decode_udb3():
    global c_state, c, S_HEADER1, S_IDLE, S_HEADER2, S_HEADER_CMD, S_HEADER_SIZE,\
    INBUF_SIZE, HEADER_BYTE_1, HEADER_BYTE_2, INBUF_SIZE, offset,\
    dataSize, checksum, cmd

    c = ser.read(1)
    while c:
        c = bytearray(c)[0]
        if (c_state == S_IDLE):
            c_state =  S_HEADER1  if (c==HEADER_BYTE_1) else S_IDLE
        elif (c_state == S_HEADER1):
            c_state = S_HEADER2 if(c==HEADER_BYTE_2) else S_IDLE
        elif (c_state == S_HEADER2):
            cmd = c
            checksum = 0
            checksum = checksum + c
            c_state = S_HEADER_CMD
        elif (c_state == S_HEADER_CMD):
            if (c > INBUF_SIZE):
                c_state = S_IDLE
                return
            dataSize = c
            inBuf = []
            offset = 0
            checksum = checksum + c
            c_state = S_HEADER_SIZE  # the command is to follow
        elif (c_state == S_HEADER_SIZE and offset < dataSize):
            checksum = checksum + c
            inBuf.append(c)
            offset = len(inBuf)
        elif (c_state == S_HEADER_SIZE and offset >= dataSize):
            if (to8bit(checksum) == 0xFF - c + 1) :  # compare calculated and transferred checksum
                evaluateCommand(cmd, dataSize, inBuf)  # we got a valid packet, evaluate it
        else :
            a = 0
            print "wrong checksum" , checksum , 0xFF - c + 1
            c_state = S_IDLE
            return 
        c = ser.read()


def evaluateCommand(command, datasize, buf):
    global inBuf
    if command == CMD_SONARS:
        print "got sonar"
    elif command == CMD_AHRS:
        buf = [chr(x) for x in buf]
        buf = ''.join(buf)
        buf = struct.unpack("7h",buf)
        print buf
        values1.append(buf[1])
        values2.append(buf[2])
        values3.append(buf[3])
        values4.append(buf[4])
        values5.append(buf[5])
        values6.append(buf[6])


def SerialReader():
    read_and_decode_udb3()
    root.after(int(wScale2['to'])-wScale2.get(),SerialReader)


def Realtimeplotter():
    global values,wScale,wScale2
    NumberSamples=min(len(values1),wScale.get())
    CurrentXAxis=pylab.arange(len(values1)-NumberSamples,len(values1),1)
    line1[0].set_data(CurrentXAxis,pylab.array(values1[-NumberSamples:]))
    line2[0].set_data(CurrentXAxis,pylab.array(values2[-NumberSamples:]))
    line3[0].set_data(CurrentXAxis,pylab.array(values3[-NumberSamples:]))
    line4[0].set_data(CurrentXAxis,pylab.array(values4[-NumberSamples:]))
    line5[0].set_data(CurrentXAxis,pylab.array(values5[-NumberSamples:]))
    line6[0].set_data(CurrentXAxis,pylab.array(values6[-NumberSamples:]))
    ax.axis([CurrentXAxis.min(),CurrentXAxis.max(),Y_MIN,Y_MAX])
    canvas.draw()
    root.after(10,Realtimeplotter)
    #canvas.draw()

    #manager.show()

def _quit():
    root.quit()     # stops mainloop
    root.destroy()  # this is necessary on Windows to prevent
                    # Fatal Python Error: PyEval_RestoreThread: NULL tstate


def sendCommand(serialPort, throttle = 0, roll = 0, pitch = 0, yaw = 0,
                gear = 0, aux1 = 0,aux2 = 0):
    """ Combine channel and values into 16 bit integer and pack data into 
    packet format.
    """
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


def axis_convert2(axis, value):
    """ Scale raw values coming from the joystick to be UDB3 readable """
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


def quadprint2():
    """ Print values that the joystick is reading and sending to the quad """
    while True:
        axislist = []
        pygame.event.get()
        for axis in range(joy.get_numaxes()):
            position = joy.get_axis(axis)
            print axis_convert2(axis, position)
        pygame.time.wait(200)


root.protocol("WM_DELETE_WINDOW", _quit)  #thanks aurelienvlg
root.after(10,SerialReader)
root.after(10,RealtimePlotter)
Tkinter.mainloop()
pylab.show()


import pylab
from pylab import *
import Tkinter
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
import serial
import struct
from time import *
import threading
import numpy

serialdata = []
data = True

def to16bit(x):
    return (x%(1<<16))
def to8bit(x):
    return (x%(1<<8))

def quaternionToEuler((q0,q1,q2,q3)):
  roll = math.atan2(2*(q0*q1+q2*q3),1-2*(q1**2+q2**2))
  pitch = math.asin(2*(q0*q2-q3*q1))
  yaw = math.atan2(2*(q0*q3+q1*q2),1-2*(q2**2+q3**2))
  return (roll,pitch,yaw)

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
CMD_AHRS_EXT = 0x02
def read_and_decode_udb3():
  global c_state, c, S_HEADER1, S_IDLE, S_HEADER2, S_HEADER_CMD, S_HEADER_SIZE, INBUF_SIZE, HEADER_BYTE_1, HEADER_BYTE_2, INBUF_SIZE, offset, dataSize, checksum, cmd
  c = ser.read(1)
  while not(type(c) == type(2)):
    c = bytearray(c)[0]
    if (c_state == S_IDLE):
      c_state =  S_HEADER1  if (c==HEADER_BYTE_1) else S_IDLE
    elif (c_state == S_HEADER1):
      c_state = S_HEADER2 if(c==HEADER_BYTE_2) else S_IDLE
    elif (c_state == S_HEADER2):
      cmd = c
      checksum = 0
      checksum = to8bit(checksum + c)
      c_state = S_HEADER_CMD
    elif (c_state == S_HEADER_CMD):
      if (c > INBUF_SIZE):
        c_state = S_IDLE
        return
      dataSize = c
      inBuf = []
      offset = 0
      checksum = to8bit(checksum + c)
      c_state = S_HEADER_SIZE  # the command is to follow
    elif (c_state == S_HEADER_SIZE and offset < dataSize):
      checksum = to8bit(checksum + c)
      inBuf.append(c)
      offset = len(inBuf)
    elif (c_state == S_HEADER_SIZE and offset >= dataSize):
      if (to8bit(checksum) == to8bit(0xFF - c + 1)):  # compare calculated and transferred checksum
        evaluateCommand(cmd, dataSize, inBuf)  # we got a valid packet, evaluate it
      else :
        a = 0
        print "wrong checksum" , to8bit(checksum) , to8bit(0xFF - c + 1)
      c_state = S_IDLE
      return 
    c = ser.read()


prevVal = -1
alpha = 0.5

previousValues = []
MEDIAN_WINDOW_SIZE = 10

def evaluateCommand(command,datasize, buf):
  global inBuf,prevVal,alpha, previousValues, MEDIAN_WINDOW_SIZE
  if command == CMD_SONARS:
    buf = [chr(x) for x in buf]
    buf = ''.join(buf)
    #enables arbitrary number of sonar readings
    n = "%d" % datasize
    buf = struct.unpack(n+"B",buf)

    if len(previousValues) < MEDIAN_WINDOW_SIZE:
      previousValues = [0]*MEDIAN_WINDOW_SIZE
    previousValues.append(buf[0])
    del previousValues[0]
    xt = numpy.median(previousValues)
    #print xt

    '''
    xt = buf[0]
    if prevVal == -1:
      prevVal = xt
    xt = alpha*xt + (1-alpha)*prevVal
    prevVal = xt
    '''
    values12.append(xt*100)
  elif command == CMD_AHRS:
    buf = [chr(x) for x in buf]
    buf = ''.join(buf)
    buf = struct.unpack("7h",buf)
    #print buf
    values1.append(buf[1])
    values2.append(buf[2])
    values3.append(buf[3])
    values4.append(buf[4])
    values5.append(buf[5])
    values6.append(buf[6])
  elif command == CMD_AHRS_EXT:
    buf = [chr(x) for x in buf]
    buf = ''.join(buf)
    buf = struct.unpack("17h",buf)
    #print buf

    (roll,pitch,yaw) = quaternionToEuler(tuple([x/10000.0 for x in buf[:4]]))

    #Roll com vs. est
    #values1.append(buf[1])
    #values2.append(buf[2])
    values1.append(roll*180/3.14*100)
    values2.append(pitch*180/3.14*100)
    #Gyro
    #values6.append(buf[8] + 7500)
    #values7.append(buf[9] + 7500)
    #values8.append(buf[10] + 7500)
    #Acc
    values3.append(buf[11] - 7500)
    values4.append(buf[12] - 7500)
    values5.append(buf[13] - 7500)
    #Integrals
    values9.append(buf[5] - 7500)
    values10.append(buf[6] - 7500)
    values11.append(buf[7] - 7500)

Y_MIN = -32000
Y_MAX = 32000

Y_MIN = -10000
Y_MAX = 10000

Y_MIN = -5000
Y_MAX = 5000

Y_MIN = -12000
Y_MAX = 12000
NUM_SONARS = 4

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
line2 = ax.plot(xAchse,yAchse,'--')

line3 = ax.plot(xAchse,yAchse,'-')
line4 = ax.plot(xAchse,yAchse,'-')
line5 = ax.plot(xAchse,yAchse,'-')

line6 = ax.plot(xAchse,yAchse,'-')
line7 = ax.plot(xAchse,yAchse,'-')
line8 = ax.plot(xAchse,yAchse,'-')

line9 = ax.plot(xAchse,yAchse,'-')
line10 = ax.plot(xAchse,yAchse,'--')
line11 = ax.plot(xAchse,yAchse,'--')

line12 = ax.plot(xAchse,yAchse,'-')

canvas = FigureCanvasTkAgg(fig, master=root)
canvas.show()
canvas.get_tk_widget().pack(side=Tkinter.TOP, fill=Tkinter.BOTH, expand=1)

toolbar = NavigationToolbar2TkAgg( canvas, root )
toolbar.update()
canvas._tkcanvas.pack(side=Tkinter.TOP, fill=Tkinter.BOTH, expand=1)

values1 = [0 for x in range(100)]
values2 = [0 for x in range(100)]

values3 = [0 for x in range(100)]
values4 = [0 for x in range(100)]
values5 = [0 for x in range(100)]

values6 = [0 for x in range(100)]
values7 = [0 for x in range(100)]
values8 = [0 for x in range(100)]

values9 = [0 for x in range(100)]
values10 = [0 for x in range(100)]
values11 = [0 for x in range(100)]

values12 = [0 for x in range(100)]

ser = serial.Serial('/dev/ttyUSB0', 57600)


def RealtimePloter():
  global values,wScale,wScale2

  if wScale2.get()>= 200:
    #subplot(211)
    '''
    NumberSamples=min(len(values1),wScale.get())
    CurrentXAxis=pylab.arange(len(values1)-NumberSamples,len(values1),1)
    line1[0].set_data(CurrentXAxis,pylab.array(values1[-NumberSamples:]))
    line2[0].set_data(CurrentXAxis,pylab.array(values2[-NumberSamples:]))
    line3[0].set_data(CurrentXAxis,pylab.array(values3[-NumberSamples:]))
    line4[0].set_data(CurrentXAxis,pylab.array(values4[-NumberSamples:]))
    line5[0].set_data(CurrentXAxis,pylab.array(values5[-NumberSamples:]))
    line6[0].set_data(CurrentXAxis,pylab.array(values6[-NumberSamples:]))
    line7[0].set_data(CurrentXAxis,pylab.array(values7[-NumberSamples:]))
    line8[0].set_data(CurrentXAxis,pylab.array(values8[-NumberSamples:]))
    #line9[0].set_data(CurrentXAxis,pylab.array(values9[-NumberSamples:]))
    #line10[0].set_data(CurrentXAxis,pylab.array(values10[-NumberSamples:]))
    #line11[0].set_data(CurrentXAxis,pylab.array(values11[-NumberSamples:]))
    '''
    #subplot(212)
    NumberSamples=min(len(values12),wScale.get())
    CurrentXAxis=pylab.arange(len(values12)-NumberSamples,len(values12),1)
    line12[0].set_data(CurrentXAxis,pylab.array(values12[-NumberSamples:]))

    ax.axis([CurrentXAxis.min(),CurrentXAxis.max(),Y_MIN,Y_MAX])
    canvas.draw()

  root.after(10,RealtimePloter)

class SensorThread(threading.Thread):   
    _stopped = False

    def stop(self):
        self._stopped = True

    def run(self):
        try:
            while not self._stopped:
                read_and_decode_udb3()
                sleep(0.001)
        except KeyboardInterrupt:
            exit()

  
sensorThread = SensorThread()

def _quit():
    root.quit()     # stops mainloop
    root.destroy()  # this is necessary on Windows to prevent
                    # Fatal Python Error: PyEval_RestoreThread: NULL tstate
    sensorThread.stop()
    

wScale = Tkinter.Scale(master=root,label="View Width:", from_=3, to=3000,sliderlength=30,length=ax.get_frame().get_window_extent().width, orient=Tkinter.HORIZONTAL)
wScale2 = Tkinter.Scale(master=root,label="Generation Speed:", from_=1, to=200,sliderlength=30,length=ax.get_frame().get_window_extent().width, orient=Tkinter.HORIZONTAL)

if __name__ == "__main__":
    sensorThread.start()

    button = Tkinter.Button(master=root, text='Quit', command=_quit)
    button.pack(side=Tkinter.BOTTOM)

    wScale2.pack(side=Tkinter.BOTTOM)
    wScale.pack(side=Tkinter.BOTTOM)

    wScale.set(100)
    wScale2.set(wScale2['to']-0)

    root.protocol("WM_DELETE_WINDOW", _quit)  #thanks aurelienvlg
    root.after(10,RealtimePloter)
    Tkinter.mainloop()
    #pylab.show()
    #Gui().run()
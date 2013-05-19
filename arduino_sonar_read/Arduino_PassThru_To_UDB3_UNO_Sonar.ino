#include <SoftwareSerial.h>
//#define DEBUG

SoftwareSerial mySerial(2, 3); // RX, TX

//*******Packet Commands*******
#define CMD_SONARS 0xA1
//***end of Packet Commands****

#define HEADER_BYTE_1 0xFF
#define HEADER_BYTE_2 0xFE
#define OUTBUF_SIZE 64
static uint8_t outBuf[OUTBUF_SIZE];
static uint8_t outDataSize = 0;

//timer to execute various events
long previousMillis = 0;
long interval = 20; //20hz

void setup()  
{
 // Open serial communications and wait for port to open:
  Serial.begin(57600);

  // set the data rate for the SoftwareSerial port
  mySerial.begin(57600);
  //mySerial.println("Hello, world?");
}

void loop() // run over and over
{
  //execute this timer every N=interval msecs
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;   
    processSonar();
    //read and send sonar data
    //here
    //...
    //Use Serial1 to send data
    //Serial1.write();
  }
  /*
  while (Serial.available())
  {
    byte t = Serial.read();
    mySerial.write(t);
    //Serial1.write();
  }
  */
  while (mySerial.available())
  {
    byte t = mySerial.read();
    Serial.write(t);
  }
}



void processSonar()
{
  float sensorvalue = analogRead(A0); //get analog sensor value from pin 0
  float inchvalue = (254.0/1024.0) * 2.0 * sensorvalue; //convert to inches
  float mmvalue = 10 * 2.54*inchvalue; //convert to mm
  word mmout = word(mmvalue);
  byte outVal = (byte)(mmout/20);
  outBuf[0] = outVal;
  outDataSize = 1;
  
  /*
  sensorvalue = analogRead(A1); //get analog sensor value from pin 0
  inchvalue = (254.0/1024.0) * 2.0 * sensorvalue; //convert to inches
  mmvalue = 10 * 2.54*inchvalue; //convert to mm
  mmout = word(mmvalue);
  outVal = (byte)(mmout/20);
  outBuf[1] = outVal;
  
  sensorvalue = analogRead(A2); //get analog sensor value from pin 0
  inchvalue = (254.0/1024.0) * 2.0 * sensorvalue; //convert to inches
  mmvalue = 10 * 2.54*inchvalue; //convert to mm
  mmout = word(mmvalue);
  outVal = (byte)(mmout/20);
  outBuf[2] = outVal;
  outDataSize = 3;
  */
  
  
  #ifdef DEBUG
    Serial.print("Sonar: ");
    Serial.print(outVal);
    Serial.println(" mm; ");
  #endif
  sendPacketPC(CMD_SONARS);
}


//************************************
// Sending packets of data
//************************************

//put your data in the outBuf before sending, set the outDataSize to the size of your data
void sendPacketPC(uint8_t cmd) {
  uint8_t checksum = 0;
  Serial.write(HEADER_BYTE_1); 
  Serial.write(HEADER_BYTE_2);
  Serial.write(cmd);
  checksum += cmd;
  Serial.write(outDataSize);
  checksum += outDataSize;
  for (int i=0; i < outDataSize; i++) {
    uint8_t t = outBuf[i];
    Serial.write(t);
    checksum += t;
  }
  checksum = 0xFF - checksum + 1;
  Serial.write(checksum);
  outDataSize = 0;
}

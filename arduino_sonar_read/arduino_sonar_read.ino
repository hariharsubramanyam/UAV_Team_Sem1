#include <SPI.h>
#include <SoftwareSerial.h>
#define DEBUG


float distance = 0.0;
SoftwareSerial udbSerial(2, 3); // RX, TX


void setup() {                    //begin of program
  Serial.begin(57600);
  Serial.println("Running Program...");
  udbSerial.begin(57600);
  delay(500);  
}
void loop(){                     //looping of program
  processSonar();
  delay(1000);
}

float getDistance(int analogPin){
  return 2.54*(254.0/1024.0) *2.0*analogRead(analogPin); //get analog sensor value from pin 0 and convert to inches
}


//***end of Packet Commands****

#define HEADER_BYTE_1 0xFF
#define HEADER_BYTE_2 0xFE
#define INBUF_SIZE 64
static uint8_t inBufMain[INBUF_SIZE];
static uint8_t inBufUDB[INBUF_SIZE];
#define OUTBUF_SIZE 64
static uint8_t outBuf[OUTBUF_SIZE];
static uint8_t dsmxBuf[OUTBUF_SIZE];
static uint8_t outDataSize = 0;

//timer to execute various events
long previousMillis = 0;
long interval = 50; //100hz
boolean GO = false;



void processSonar()
{
  float sensorvalue = analogRead(A0); //get analog sensor value from pin 0
  float inchvalue = (254.0/1024.0) *2.0* sensorvalue; //convert to inches
  float mmvalue = 10 * 2.54*inchvalue; //convert to mm
  word mmout = word(mmvalue);
  //if doesn't work, try re-ordering low and high bytes
  outBuf[1] = highByte(mmout);
  outBuf[0] = lowByte(mmout);
  outDataSize = 2;
  #ifdef DEBUG
    Serial.print("Sonar: ");
    Serial.print(mmout);
    Serial.print(" mm; ");
  #endif
  sendPacketPC(CMD_SONARS);
}

//************************************
// Receive the data from USB/Xbee
//************************************

void processMainSerial() {
  static enum _serial_state {
    IDLE,
    HEADER1,
    HEADER2,
    HEADER_SIZE,
    HEADER_CMD
  } 
  c_state;

  static uint8_t dataSize, offset, checksum, cmd;

  uint8_t c;
  while (Serial.available()){
    c = Serial.read();
    // regular data handling to detect and handle MSP and other data
    if (c_state == IDLE) {
      c_state = (c==HEADER_BYTE_1) ? HEADER1 : IDLE;
    } 
    else if (c_state == HEADER1) {
      c_state = (c==HEADER_BYTE_2) ? HEADER2 : IDLE;
    } 
    else if (c_state == HEADER2) {
      cmd = c;
      c_state = HEADER_CMD;
    }
    else if (c_state == HEADER_CMD) {
      if (c > INBUF_SIZE) {  // now we are expecting the payload size
        c_state = IDLE;
        return;
      }
      dataSize = c;
      offset = 0;
      checksum = 0;
      checksum = checksum + c;
      c_state = HEADER_SIZE;  // the command is to follow
    } 
    else if (c_state == HEADER_SIZE && offset < dataSize) {
      checksum = checksum + c;
      inBufUDB[offset++] = c;
    } 
    else if (c_state == HEADER_SIZE && offset >= dataSize) {
      if (checksum == 0xFF - c) {  // compare calculated and transferred checksum
        evaluateMainCommand(cmd, dataSize);  // we got a valid packet, evaluate it
      }
#ifdef DEBUG
      else {
        Serial.print("Wrong checksum on Main serial: "); 
        Serial.print(0xFF - c);
        Serial.print(" ");
        Serial.println(checksum);
      }
#endif
      c_state = IDLE;
    }
  }
}

void evaluateMainCommand(uint8_t cmd, uint8_t dataSize) {
  #ifdef DEBUG
   Serial.print("Main: Received packet: ");
   Serial.print(cmd);
   Serial.print(' ');
     
   Serial.print(dataSize);
   Serial.print(' ');
   
   for (int i=0; i<dataSize; i++)
   {
     Serial.print(inBufMain[i]);
     Serial.print(' ');
   }
   Serial.println();
  #endif
}


//************************************
// Receive the data from UDB3
//************************************

void processUDBSerial() {
  static enum _serial_state {
    IDLE,
    HEADER1,
    HEADER2,
    HEADER_SIZE,
    HEADER_CMD
  } 
  c_state;

  static uint8_t dataSize, offset, checksum, cmd;

  uint8_t c;
  while (udbSerial.available()){
    c = udbSerial.read();
    // regular data handling to detect and handle MSP and other data
    if (c_state == IDLE) {
      c_state = (c==HEADER_BYTE_1) ? HEADER1 : IDLE;
    } 
    else if (c_state == HEADER1) {
      c_state = (c==HEADER_BYTE_2) ? HEADER2 : IDLE;
    } 
    else if (c_state == HEADER2) {
      cmd = c;
      c_state = HEADER_CMD;
    }
    else if (c_state == HEADER_CMD) {
      if (c > INBUF_SIZE) {  // now we are expecting the payload size
        c_state = IDLE;
        return;
      }
      dataSize = c;
      offset = 0;
      checksum = 0;
      checksum = checksum + c;
      c_state = HEADER_SIZE;  // the command is to follow
    } 
    else if (c_state == HEADER_SIZE && offset < dataSize) {
      checksum = checksum + c;
      inBufUDB[offset++] = c;
    } 
    else if (c_state == HEADER_SIZE && offset >= dataSize) {
      if (checksum == 0xFF - c) {  // compare calculated and transferred checksum
        evaluateUDBCommand(cmd, dataSize);  // we got a valid packet, evaluate it
      }
#ifdef DEBUG
      else {
        Serial.print("Wrong checksum on UDB serial: "); 
        Serial.print(0xFF - c);
        Serial.print(" ");
        Serial.println(checksum);
      }
#endif
      c_state = IDLE;
    }
  }
}

void evaluateUDBCommand(uint8_t cmd, uint8_t dataSize) {
  //process inBufUDB with size datasize
  #ifdef DEBUG
   Serial.print("UDB3: Received packet: ");
   Serial.print(cmd);
   Serial.print(' ');
     
   Serial.print(dataSize);
   Serial.print(' ');
   
   for (int i=0; i<dataSize; i++)
   {
     Serial.print(inBufUDB[i]);
     Serial.print(' ');
   }
   Serial.println();
  #endif
}



//************************************
// Sending packets of data
//************************************

//put your data in the outBuf before sending, set the outDataSize to the size of your data
void sendPacketPC(uint8_t cmd) {
  uint8_t checksum = 0;

#ifndef DEBUG
  Serial.write(HEADER_BYTE_1); 
  checksum += HEADER_BYTE_1;
  Serial.write(HEADER_BYTE_2);
  checksum += HEADER_BYTE_2;
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
#endif
#ifdef DEBUG
  Serial.print("Sending packet: "); 
  Serial.print(HEADER_BYTE_1); 
  Serial.print(" ");
  checksum += HEADER_BYTE_1;
  Serial.print(HEADER_BYTE_2);
  Serial.print(" ");
  checksum += HEADER_BYTE_2;
  Serial.print(cmd);
  Serial.print(" ");
  checksum += cmd;
  Serial.print(outDataSize);
  Serial.print(" ");
  checksum += outDataSize;
  for (int i=0; i < outDataSize; i++) {
    uint8_t t = outBuf[i];
    Serial.print(t);
    Serial.print(" ");
    checksum += t;
  }
  checksum = 0xFF - checksum + 1;
  Serial.print(checksum);
  Serial.println();
  outDataSize = 0;
#endif
}


//put your data in the outBuf before sending, set the outDataSize to the size of your data
void sendPacketUDB() {
  uint8_t checksum = 0;

  udbSerial.write(HEADER_BYTE_1); 
  checksum += HEADER_BYTE_1;
  udbSerial.write(HEADER_BYTE_2);
  checksum += HEADER_BYTE_2;
  //udbSerial.write(cmd);
  //checksum += cmd;
  //udbSerial.write(outDataSize);
  //checksum += outDataSize;
  for (int i=0; i < 14; i++) {
    uint8_t t = outBuf[i];
    udbSerial.write(t);
    checksum += t;
  }
  //checksum = 0xFF - checksum + 1;
  //udbSerial.write(checksum);
  //outDataSize = 0;
}

void testPitchRollYaw(uint16_t throttle, uint16_t roll, uint16_t pitch, uint16_t yaw, uint16_t gear, uint16_t aux1, uint16_t aux2) {
  dsmxBuf[1] = (uint8_t) throttle;
  dsmxBuf[0] = (uint8_t) (throttle >> 8) & 0b00000111;
  dsmxBuf[3] = (uint8_t) pitch;
  dsmxBuf[2] = (uint8_t) (pitch >> 8) & 0b00000111 | 0b00001000;
  dsmxBuf[5] = (uint8_t) roll;
  dsmxBuf[4] = (uint8_t) (roll >> 8) & 0b00000111 | 0b00010000;
  dsmxBuf[7] = (uint8_t) yaw;
  dsmxBuf[6] = (uint8_t) (yaw >> 8) & 0b00000111 | 0b00011000;
  dsmxBuf[9] = (uint8_t) gear;
  dsmxBuf[8] = (uint8_t) (gear >> 8) & 0b00000111 | 0b00100000;
  dsmxBuf[11] = (uint8_t) aux1;
  dsmxBuf[10] = (uint8_t)(aux1 >> 8) & 0b00000111 | 0b00101000;
  dsmxBuf[13] = (uint8_t) aux2;
  dsmxBuf[12] = (uint8_t) (aux2 >> 8) & 0b00000111 | 0b00110000;
  
  
  udbSerial.write(HEADER_BYTE_1); 
  udbSerial.write(HEADER_BYTE_1);
  
  for (int i=0; i < 14; i++) {
    uint8_t t = dsmxBuf[i];
    udbSerial.write(t);
  }
  
}

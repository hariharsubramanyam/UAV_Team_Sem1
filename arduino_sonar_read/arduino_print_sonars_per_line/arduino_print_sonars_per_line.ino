#define NUM_SONARS 1

int sonarPins[4];

void setup(){
    sonarPins[0] = A0;
    sonarPins[1] = A1;
    sonarPins[2] = A2;
    sonarPins[3] = A3;
    Serial.begin(57600);
}

int i;
void loop(){ 
  for(i = 0; i < NUM_SONARS; i++){
    Serial.print(analogRead(sonarPins[i]));
    Serial.print(" ");
  }
  Serial.print("100 ");
  Serial.print("150 ");
  Serial.print("50 ");
  Serial.println();
}


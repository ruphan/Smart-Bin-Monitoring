#include "HX711.h"

HX711 scale(5, 6);

#define DUSTBIN_ID "SB_NITT_1"
const int _VERSION_ = 1;

float calibration_factor = 48100; // this calibration factor is adjusted according to my load cell

uint16_t currentWeight = 0x0000;
uint16_t currentIR = 0x0000;

uint16_t prevWeight = 0x0000;
uint16_t prevIR = 0x0000;

uint16_t thresholdWeight = 0x1388;                          // 50kgs 
uint16_t thresholdIR = 0x0000;

uint16_t getWeight(){
  float units;
  //float ounces;
  scale.set_scale(calibration_factor);
  units = scale.get_units(), 10;
  if (units < 0)
  {
    units = 0.00;
  }
  //ounces = units * 0.035274;
  // KG to milli-grams conversion
  uint16_t weight = units*1000;
  return weight;
}

uint8_t getIRReading(){
  uint8_t currentIR = 0x00;
  for(int i=0;i<6;i++){
    currentIR = currentIR|(uint8_t)digitalRead(i+7);
    currentIR = currentIR<<1;
  }
  currentIR = currentIR<<1;
  currentIR = ~currentIR;
  currentIR &= 0b11111100;
  return currentIR;
}

bool sendDataToESP(){
  // Flush Tx, Rx both
  Serial.flush();
  Serial.write((byte)0x01);                                               // Start of Frame
  Serial.write((byte)0x10);                                               // Start of Current Weight Block
  Serial.write((byte)((currentWeight >> 8) & 0xFF));
  Serial.write((byte)(currentWeight & 0xFF));
  Serial.write((byte)0x11);                                               // Start of Threshold Weight Block
  Serial.write((byte)((thresholdWeight >> 8) & 0xFF));
  Serial.write((byte)(thresholdWeight & 0xFF));
  Serial.write((byte)0x20);                                               // Start of IR LED Block
  Serial.write((byte)((currentIR >> 8) & 0xFF));
  Serial.write((byte)(currentIR & 0xFF));
  Serial.write((byte)0x21);                                               // Start of Threshold IR LED Block
  Serial.write((byte)((thresholdIR >> 8) & 0xFF));
  Serial.write((byte)(thresholdIR & 0xFF));
  Serial.write((byte)0x25);                                               // Version Info
  Serial.write((byte)(_VERSION_));
  Serial.write((byte)0x30);                                               // ID Info Block Start
  Serial.write(DUSTBIN_ID);
  Serial.write((byte)0x0A);                                               // ID Info Block End
  Serial.write((byte)0x04);                                              // End of Transmission
  // Read ACK back from ESP
  long long prevTry = millis();
  while(!Serial.available()){
    if( millis() - prevTry > 1000){
      break;
    }
  }
  byte ack = Serial.read();
  prevTry = millis();
  while(!Serial.available()){
    if( millis() - prevTry > 1000){
      break;
    }
  }
  byte end = Serial.read();
  if( ack == 0x06 && end == 0x04){
    return true;
  }
  return false;
}

void setup() {
  // initialization for IR
  Serial.begin(115200);
  pinMode(7,INPUT);
  pinMode(8,INPUT);
  pinMode(9,INPUT);
  pinMode(10,INPUT);
  pinMode(11,INPUT);
  pinMode(12,INPUT);

  // innitialization for weight sensor
  scale.set_scale();
  scale.tare();

  // Read and send the data after power up
  currentWeight = getWeight();
  currentIR = 0x0000 | getIRReading();
  prevWeight = currentWeight;
  prevIR = currentIR;
  bool res = sendDataToESP();
}

void loop() {
  currentWeight = getWeight();
  currentIR = 0x0000 | getIRReading();
  if(currentWeight != prevWeight || currentIR != prevIR){
    bool res = sendDataToESP(); 
    prevWeight = currentWeight;
    prevIR = currentIR;
  }
}

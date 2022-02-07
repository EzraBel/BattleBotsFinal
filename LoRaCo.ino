/*
   LoRaLib Transmit Example
   This example transmits LoRa packets with one second delays
   between them. Each packet contains up to 256 bytes
   of data, in the form of:
    - Arduino String
    - null-terminated char array (C-string)
    - arbitrary binary data (byte array)
   For more detailed information, see the LoRaLib Wiki
   https://github.com/jgromes/LoRaLib/wiki
   For full API reference, see the GitHub Pages
   https://jgromes.github.io/LoRaLib/
*/

// include the library
#include <LoRaLib.h>

// create instance of LoRa class using SX1278 module
// this pinout corresponds to RadioShield
// https://github.com/jgromes/RadioShield
// NSS pin:   10 (4 on ESP32/ESP8266 boards)
// DIO0 pin:  2
// DIO1 pin:  3

#define NUM_CHANNELS 6

#define X_AX 1
#define Y_AX 2
#define MOTOR_AX 3
#define TRIM 4
#define SENSITIVITY 5
#define AXES 4

#define TIMEOUT 1000

#define MOTOR_MAX 255
#define MOTOR_STOP 127
#define WEAPON_MOTOR_STOP 0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

SX1276 lora = SX1276(new Module(RFM95_CS, RFM95_INT, 5));

typedef struct Direct{
  float ax[4];
  float trim;
  float sensitivity;
} direct_t;

typedef struct Transmit{
  int leftMotor, rightMotor, weaponMotor;
}transmit_t;

direct_t* channelMap = new direct_t();
transmit_t* channelValues = new transmit_t();

int timeKeeper = 0;


void setup() {
  while (!Serial);
  Serial.begin(9600);
  Serial1.begin(115200);
  delay(100);
  Serial.println("Transmitter powered up!");

  
  int state = lora.beginFSK();
  state = lora.setOutputPower(20.0);
  state = lora.setBitRate(10.0);
  state = lora.setFrequency(915);
  if (state == ERR_NONE) {
    blink(5, 500);
  } else {
    
    while (true){
      blink(1,1000);
    }
  }

  while(Serial1.available()){
    Serial1.read();
  }

  timeKeeper = millis();
}

void loop() {

  byte direct[sizeof(direct_t)];

  int state = -1;

  
  if(Serial1.available()){

    //Serial.println(sizeof(direct_t));


    Serial1.readBytes(direct, sizeof(direct_t));
    memcpy(channelMap, direct, sizeof(direct_t));

  
    
    
    char buff[256];

    char float_buff1[6], float_buff2[6], float_buff3[6], float_buff4[6], float_buff5[6];

    dtostrf(channelMap->ax[X_AX], 4, 2, float_buff1);
    dtostrf(channelMap->ax[Y_AX], 4, 2, float_buff2);
    dtostrf(channelMap->ax[MOTOR_AX], 4, 2, float_buff3);
    dtostrf(channelMap->sensitivity, 4, 2, float_buff4);
    dtostrf(channelMap->trim, 4, 2, float_buff5);
    sprintf(buff, "X = %s | Y = %s | Weapon = %s | Sens = %s | Trim = %s\n", float_buff1, float_buff2, float_buff3, float_buff4, float_buff5);
    //Serial.print(buff);
     //packaging up for output 
    
    calculateTransmitValues();
    
    sprintf(buff, "Left = %i | Right = %i | Weapon: %i\n", channelValues->leftMotor, channelValues->rightMotor, channelValues->weaponMotor);
    Serial.print(buff);


    
  }

  byte writeInfo[sizeof(transmit_t)];

  memcpy(writeInfo, channelValues, sizeof(transmit_t)); //writing to mem

  state = lora.transmit(writeInfo, sizeof(transmit_t));

  if(state == ERR_TX_TIMEOUT){
    Serial.print("TX_TIMEOUT");
  }


  if(state == ERR_NONE){
    blink(1, 5);
  }

}

void calculateTransmitValues(){

  bool neg = channelMap->ax[Y_AX] < 0;
  channelMap->ax[Y_AX] = pow(abs(channelMap->ax[Y_AX]), channelMap->sensitivity) * (neg ? -1 : 1);

  int startingValue = (1 - (channelMap->ax[Y_AX] + 1)/2.0) * MOTOR_MAX;
  int turnRate = (channelMap->ax[Y_AX] < 0 ? ((channelMap->ax[Y_AX] + 1)/2.0) * MOTOR_MAX : ((1 - channelMap->ax[Y_AX])/2.0) * MOTOR_MAX) * -channelMap->ax[X_AX];
  
  channelValues->leftMotor = startingValue + turnRate;
  channelValues->rightMotor = startingValue - turnRate;
  channelValues->weaponMotor = (channelMap->ax[MOTOR_AX] + 1)/2.0 * MOTOR_MAX;
  
}

void blink(int times, int wait){
  for(int i = 0; i < times; i++){
    digitalWrite(13, HIGH);
    delay(wait);
    digitalWrite(13, LOW);
    delay(wait);
  }
}

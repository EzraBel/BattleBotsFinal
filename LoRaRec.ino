#define NUM_CHANNELS 6

// include the library
#include <LoRaLib.h>
#include<Servo.h>

// create instance of LoRa class using SX1278 module
// this pinout corresponds to RadioShield
// https://github.com/jgromes/RadioShield
// NSS pin:   10 (4 on ESP32/ESP8266 boards)
// DIO0 pin:  2
// DIO1 pin:  3
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

#define LEFT_MOTOR 12
#define RIGHT_MOTOR 11
#define WEAPON_MOTOR 10

#define MOTOR_STOP 127
#define WEAPON_MOTOR_STOP 0

#define TIMEOUT 1000

#define WEAPON_MOTOR_LOW 1100
#define WEAPON_MOTOR_HIGH 2100

#define DRIVE_MOTOR_LOW 1200
#define DRIVE_MOTOR_HIGH 1800


SX1276 lora = SX1276(new Module(RFM95_CS, RFM95_INT, 5)); //set up lora.
const int CHANNELS[6] = { 12, 11, 10, 9, 6, 5};

Servo left, right, weapon;

int timeKeeper = 0;

int stateTracker[] = {0,0};

typedef struct Transmit{
  int leftMotor, rightMotor, weaponMotor;
}transmit_t; // 6 bytes

transmit_t* channelMap = new transmit_t(); // just making one


void setup() {
  //Serial.begin(9600);

  channelMap->weaponMotor = WEAPON_MOTOR_HIGH;
  left.attach(LEFT_MOTOR);
  //channelMap->leftMotor = MOTOR_STOP;
  right.attach(RIGHT_MOTOR);
  //channelMap->rightMotor = MOTOR_STOP;
  weapon.attach(WEAPON_MOTOR);
  //channelMap->weaponMotor = WEAPON_MOTOR_LOW;
  delay(4000);
  Serial.println("Receiver powered up! ");
  delay(3000);
  // hooking up to physical parts
 

  int state = lora.beginFSK(); 
  state = lora.setOutputPower(20.0);
  state = lora.setBitRate(10);
  state = lora.setFrequency(915);
  
  if (state == ERR_NONE) { 
    Serial.println("Correct state! ");
    delay(3000);
    
  } else {
    while (true){
      Serial.println("Incorrect state! You need to reboot! ");
      delay(400);
      } 
  }
  
  
  timeKeeper = millis(); // num since boot
  
}

void loop() {
  
  // you can also receive data as byte array
  byte received[sizeof(transmit_t)]; // basically a char. 6 1B slots vs 3 2b
  int state = lora.receive(received, sizeof(transmit_t)); // sent to mem from loRa

  if(state == -6 && stateTracker[0] == -6){
    Serial.println("!!!!!!!!!!!!!!CONNECTION IS LOST!!");
    Serial.println(stateTracker[0], stateTracker[1]);
    channelMap->leftMotor = MOTOR_STOP;
    channelMap->rightMotor = MOTOR_STOP;
    channelMap->weaponMotor = WEAPON_MOTOR_LOW;

    //this is keeping track of the last two states. if it is -6 twice, connection is actually lost. 
    
  } else {
    stateTracker[1] = stateTracker[0];
    stateTracker[0] = state;
    // stateTracker[0] is current state and stateTracker[1] is last state
  }

 

  weapon.writeMicroseconds(map(channelMap->weaponMotor, 0, 255, WEAPON_MOTOR_LOW, WEAPON_MOTOR_HIGH)); // map takes val btwn lo and hi and scales
  left.writeMicroseconds(map(channelMap->leftMotor, 0, 255, DRIVE_MOTOR_LOW, DRIVE_MOTOR_HIGH));
  right.writeMicroseconds(map(channelMap->rightMotor, 0, 255, DRIVE_MOTOR_LOW, DRIVE_MOTOR_HIGH)); // whole block can go at 98
  //scale num (0,255) to be what was set earlier like 1100 2000 

  char buff[256];
  if (state == ERR_NONE) { // if receive was succesfull 
    memcpy(channelMap, received, sizeof(transmit_t));   //copy stuff filled in over  i.e R[0] is just 1. CM[0] - 256 +2
    
    sprintf(buff, "Left = %i | Right = %i | Weapon: %i", channelMap->leftMotor, channelMap->rightMotor, channelMap->weaponMotor);
    Serial.println(buff);
    //Serial.println(state);
    // use for debugging

    timeKeeper = millis();
  }
  else
  {
    Serial.println("Receive was unsuccesfull!!");
    Serial.println(state);
    //Serial.println(buff);
    delay(400);
  }
  
  
    
      
  }

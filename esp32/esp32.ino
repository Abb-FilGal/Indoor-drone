#include <Arduino.h>
#include <PS4Controller.h>

#define MOTOR1 25
#define MOTOR2 26
#define MOTOR3 27
#define MOTOR4 14

int pitchTarget;
int yawTarget;
int rollTarget;
int liftTarget;
int descendTarget;

void setup() {
  // set up bluetooth connection
  
  // set up motor pins

  // set up sensors

}

void loop() {
  // read bluetooth values, set target
   if (PS4.isConnected()) {
    rollTarget = PS4.LStickX();
    Serial.print("Roll: ");
    Serial.println(rollTarget);
    
    pitchTarget = PS4.LStickY();
    Serial.print("pitch: ");
    Serial.println(pitchTarget);

    yawTarget = PS4.RStickX();
    Serial.print("yaw: ");
    Serial.println(yawTarget);

    liftTarget = PS4.LStickY();
    Serial.print("pitch: ");
    Serial.print(pitchTarget);
    
   }else{
    Serial.println("ERROR: PS4 not connected");
   }

  // read sensor values (current state)

  // calculate response

  // write to motors

}

#include <Bluepad32.h>
#include <ESP32Servo.h>

#define MOTORRF 25
#define MOTORRB 26
#define MOTORLF 27
#define MOTORLB 14

ControllerPtr myController;


const int minPof = 1150;
const int maxPof = 2000;

const int incrementStep = 5;



struct {
  Servo RF;
  Servo RB; 
  Servo LF;
  Servo LB; 
  } motor;

struct {
  int pitch;
  int roll;
  int yaw;
  int lift;
  } target;

struct {
  int pitch;
  int roll;
  int yaw;
  int lift;
  } current;

 struct {
  int RF;
  int LF;
  int RB;
  int LB;
 } motorPof;


 
// This callback gets called any time a new gamepad is connected.
void onConnectedController(ControllerPtr ctl) {
        if (myController == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n");
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myController = ctl;
    }
    else {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {

        if (myController == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n");
            myController = nullptr;
    }

    else {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, ",
        ctl->index(),        // Controller Index
        ctl->dpad(),         // D-pad
        ctl->buttons(),      // bitmask of pressed buttons
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY(),       // (-511 - 512) right Y axis
        ctl->brake(),        // (0 - 1023): brake button
        ctl->throttle()      // (0 - 1023): throttle (AKA gas) button
    );
}

void processGamepad(ControllerPtr ctl) {
  
    // prints controller status
    dumpGamepad(ctl);

    // updates target values
    if (target.pitch > ctl->axisY() + incrementStep) {
      target.pitch -= incrementStep; 
    } else if (target.pitch < ctl-> axisY() - incrementStep) {
      target.pitch += incrementStep;
      }

    if (target.roll > ctl->axisX() + incrementStep) {
      target.roll -= incrementStep;
    } else if (target.roll < ctl->axisX() - incrementStep) {
      target.roll += incrementStep;
      }
      if (target.yaw > ctl->axisRX() + incrementStep) {
      target.yaw -= incrementStep;
    } else if (target.yaw < ctl->axisRX() - incrementStep) {
      target.yaw += incrementStep;
      }
      if (target.lift > ctl->throttle()-ctl->brake() + incrementStep) {
      target.lift -= incrementStep;
    } else if (target.lift < ctl->throttle() - ctl->brake() - incrementStep) {
      target.lift += incrementStep;
      }
}

void processControllers() {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            } else {
                Serial.println("Unsupported controller");
            }
        }
}

void writeToMotors(){
  motor.RF.writeMicroseconds(motorPof.RF);
  motor.LF.writeMicroseconds(motorPof.LF);
  motor.RB.writeMicroseconds(motorPof.RB);
  motor.LB.writeMicroseconds(motorPof.LB);
}

void motorSetup() {
  motor.RF.attach(MOTORRF);
  motor.LF.attach(MOTORLF);
  motor.RB.attach(MOTORRB);
  motor.LB.attach(MOTORLB);

  motorPof.RF = 1000;
  motorPof.LF = 1000;
  motorPof.RB = 1000;
  motorPof.LB = 1000;

  writeToMotors();

  delay(2000);  // Wait for arming sequence

  writeToMotors();

  delay(1000);  // Wait for arming sequence

  motorPof.RF = minPof;
  motorPof.LF = minPof;
  motorPof.RB = minPof;
  motorPof.LB = minPof;

  writeToMotors(); 
}

void calculateAction() {
  int motorPofValue = map(target.lift, 0, 1024, minPof, maxPof);

  Serial.println(motorPofValue);
  
  motorPof.RF = motorPofValue;
  motorPof.LF = motorPofValue;  
  motorPof.RB = motorPofValue;  
  motorPof.LB = motorPofValue;
  
 }

void setup() {
    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // fixes connection issues
    BP32.forgetBluetoothKeys();

    // disables touchpad
    BP32.enableVirtualDevice(false);

    delay(1000);

    motorSetup();
}

void loop() {

    // Update controller data
    bool dataUpdated = BP32.update();
    if (dataUpdated)
        processControllers();


    // Update current drone status

    

    // calculate actions
    calculateAction();


    // perform actions
    writeToMotors();

    delay(50);
}

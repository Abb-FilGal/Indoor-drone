#include <Bluepad32.h>
#include <ESP32Servo.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define MOTORRF 25
#define MOTORRB 26
#define MOTORLF 27
#define MOTORLB 33

#define BUTTON_L1 0x10
#define BUTTON_R1 0x20

ControllerPtr myController;

Adafruit_MPU6050 mpu;

float prevPitchError = 0;
float prevRollError = 0;
float prevYawError = 0;

const float rollP = 0.6;
const float pitchP = 0.6;
const float yawP = 2.0;

const float rollI = 3.5;
const float pitchI = 3.5;
const float yawI = 12.0;

const float rollD = 0.03;
const float pitchD = 0.03;
const float yawD = 0.;

float gx,gy,gz;

float prevIPitchError = 0;
float prevIRollError = 0;
float prevIYawError = 0;

struct {
  uint8_t r=0;
  uint8_t g=255;
  uint8_t b=0;
} color;


const float ts = 0.004;

const float minPof = 1000;
const float maxPof = 2000;

const int incrementStep = 1;


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
  float RF = 0.25;
  float LF = 0.25;
  float RB = 0.25;
  float LB = 0.25;
 } motorPof;

 sensors_event_t a, g, temp;

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
    Serial.println(ctl->battery());
    

    // Handle lock
    static bool pressed = false;
    static bool locked = false;
    uint32_t buttons = ctl->buttons();

    if(!((buttons & (BUTTON_L1 | BUTTON_R1)) == (BUTTON_L1 | BUTTON_R1))) {
      pressed = false; 
      Serial.println("BUTTON NOT PRESSED");
    }

    if((buttons & (BUTTON_L1 | BUTTON_R1)) == (BUTTON_L1 | BUTTON_R1)) {
      if(!pressed) {
        pressed=true;
        locked = !locked;
        switch (locked) {
            case false:
                // Red
                color.r=0;
                color.g=255;
                color.b=0;
                break;
            case true:
                // Green
                color.r=255;
                color.g=0;
                color.b=0;
                break;
        }
      }

    }

    if(ctl->battery()<20) {
     Serial.println("Battery low");
    }


    ctl->setColorLED(color.r, color.g, color.b);
    

    

    

    if(!locked) {
    // updates target values

    if (target.pitch >= ctl->axisY() + incrementStep) {

      target.pitch -= incrementStep; 
    } else if (target.pitch <= ctl-> axisY() - incrementStep) {
      target.pitch += incrementStep;
      }

    if (target.roll >= ctl->axisX() + incrementStep) {
      target.roll -= incrementStep;
    } else if (target.roll <= ctl->axisX() - incrementStep) {
      target.roll += incrementStep;
      }
      if (target.yaw >= ctl->axisRX() + incrementStep) {
      target.yaw -= incrementStep;
    } else if (target.yaw <= ctl->axisRX() - incrementStep) {
      target.yaw += incrementStep;
      }
      if (target.lift >= ctl->throttle()-ctl->brake() + incrementStep) {
      target.lift -= incrementStep;
    } else if (target.lift <= ctl->throttle() - ctl->brake() - incrementStep) {
      target.lift += incrementStep;
      }

    if(target.lift>999) {
       ctl->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */,
                            0x40 /* strongMagnitude */);
    }


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
  int motorThrottle = map(target.lift, 0, 1024, 1000, 12);
  
  motor.RF.writeMicroseconds(motorPof.RF + motorThrottle);
  motor.LF.writeMicroseconds(motorPof.LF * motorThrottle);
  motor.RB.writeMicroseconds(motorPof.RB * motorThrottle);
  motor.LB.writeMicroseconds(motorPof.LB * motorThrottle);
}

void motorSetup() {
  motor.RF.attach(MOTORRF);
  motor.LF.attach(MOTORLF);
  motor.RB.attach(MOTORRB);
  motor.LB.attach(MOTORLB);

  motorPof.RF = 0;
  motorPof.LF = 0;
  motorPof.RB = 0;
  motorPof.LB = 0;

  writeToMotors();

  delay(2000);  // Wait for arming sequence

  writeToMotors();

  delay(1000);  // Wait for arming sequence

  motorPof.RF = 0;
  motorPof.LF = 0;
  motorPof.RB = 0;
  motorPof.LB = 0;

  writeToMotors(); 
}

void calculateAction() {
  int motor = map(target.lift, 0, 1024, 1000, 2000);

  float desiredRoll = map(target.roll, -512, 512, -1, 1);
  float desiredPitch = map(target.pitch, -512, 512, -1, 1);
  float desiredYaw = map(target.yaw, -512, 512, -1, 1);

  float errorRoll = (desiredRoll - gx);
  float errorPitch = (desiredPitch - gy);
  float errorYaw = (desiredYaw - gz);

  
  prevIRollError += rollI*(errorRoll+prevRollError)* ts /2;
  float Droll = rollD*(errorRoll-prevRollError)/ts;
  prevRollError = errorRoll;
  
  prevIPitchError += pitchI*(errorPitch+prevPitchError)* ts /2;
  float Dpitch = pitchD*(errorPitch-prevPitchError)/ts;
  prevPitchError = errorPitch;

  prevIYawError += yawI*(errorYaw+prevYawError)* ts /2;
  float Dyaw = yawD*(errorYaw-prevYawError)/ts;
  prevYawError = errorYaw;

  float InputRoll = rollP*errorRoll+prevIRollError+Droll;  
  float InputPitch = pitchP*errorPitch+prevIPitchError+Dpitch;
  float InputYaw = yawP*errorYaw+prevIYawError+Dyaw;

  motorPof.RB -= InputRoll;
  motorPof.LB += InputRoll;
  motorPof.RF -= InputRoll;
  motorPof.LF += InputRoll;

  motorPof.RB += InputPitch;
  motorPof.LB += InputPitch;
  motorPof.RF -= InputPitch;
  motorPof.LF -= InputPitch;

  motorPof.RB += InputYaw;
  motorPof.LB -= InputYaw;
  motorPof.RF -= InputYaw;
  motorPof.LF += InputYaw;
 }

 void getSensorValues() {
  mpu.getEvent(&a, &g, &temp);
  gx = g.gyro.x;
  gy = g.gyro.y;
  gz = g.gyro.z;

  Serial.print("Rotation X: ");
  Serial.print(gx);
  Serial.print(", Y: ");
  Serial.print(gy);
  Serial.print(", Z: ");
  Serial.print(gz);
  Serial.println(" rad/s");
  Serial.println("");

 }


void setup() {
    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    if (!mpu.begin()){
      Serial.println("Adafruit MPU6050 test!");
      while (1) {
        delay(10);
      }
    }

    Serial.println("MPU6050 Found!");

    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);

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
    getSensorValues();

    

    // calculate actions
    calculateAction();


    // perform actions
    writeToMotors();

    delay(4);
}

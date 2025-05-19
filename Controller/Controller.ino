#include <Bluepad32.h>

ControllerPtr myController;

#define MOTORRF 25

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
    // dumpGamepad(ctl);

    // updates target values
    target.pitch = ctl->axisY();
    target.roll = ctl->axisX();
    target.yaw = ctl->axisRX();
    target.lift = ctl->throttle() - ctl->brake();
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
}

void loop() {

    // Update controller data
    bool dataUpdated = BP32.update();
    if (dataUpdated)
        processControllers();


    // Update current drone status

    

    // calculate actions


    // perform actions

    delay(150);
}

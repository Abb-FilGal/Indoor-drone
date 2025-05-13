#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>


// UUIDs
#define DRONE_SERVICE_UUID        "6b1d428a-3abd-4e08-a55f-63f2a0dfa565"

// Battery Service (standard)
#define BATTERY_SERVICE_UUID      BLEUUID((uint16_t)0x180F)
#define BATTERY_CHAR_UUID         BLEUUID((uint16_t)0x2A19)

// Command characteristic
#define COMMAND_CHAR_UUID         "cf95ef36-ec9d-40a5-b747-29c7b93a63fe"

// Telemetry UUIDs
#define TELEMETRY_SERVICE_UUID    "a1209824-94ff-4ed8-a3b3-50f0a9c41325"
#define ALTITUDE_CHAR_UUID        "1a2b3c4d-5e6f-7a8b-9c0d-1e2f3a4b5c6d"
#define SPEED_CHAR_UUID           "2a3b4c5d-6e7f-8a9b-0c1d-2e3f4a5b6c7d"
#define GPS_CHAR_UUID             "3a4b5c6d-7e8f-9a0b-1c2d-3e4f5a6b7c8d"

// Status UUIDs
#define STATUS_SERVICE_UUID       "b3dca7d3-cd74-4726-a9f5-d9f2f3c9f3b3"
#define FLIGHT_MODE_CHAR_UUID     "4a5b6c7d-8e9f-0a1b-2c3d-4e5f6a7b8c9d"
#define ERROR_STATE_CHAR_UUID     "5a6b7c8d-9e0f-1a2b-3c4d-5e6f7a8b9c0d"

// Global variables
BLECharacteristic* pBatteryLevelCharacteristic;
BLECharacteristic* pCommandCharacteristic;
BLECharacteristic* pAltitudeCharacteristic;
BLECharacteristic* pSpeedCharacteristic;
BLECharacteristic* pGPSCharacteristic;
BLECharacteristic* pFlightModeCharacteristic;
BLECharacteristic* pErrorStateCharacteristic;

// Custom callback for command characteristic
class CommandCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.print("Received Command: ");
      for (int i = 0; i < value.length(); i++)
        Serial.print(value[i]);
      Serial.println();

      // TODO: Parse and act on the command here
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Drone BLE Server");

  BLEDevice::init("Fernando de la Salsa Picante III");
  BLEServer *pServer = BLEDevice::createServer();

  // === Command/Control Service ===
  BLEService *pDroneService = pServer->createService(DRONE_SERVICE_UUID);
  pCommandCharacteristic = pDroneService->createCharacteristic(
    COMMAND_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_READ
  );
  pCommandCharacteristic->setCallbacks(new CommandCallback());


  pCommandCharacteristic->addDescriptor(new BLE2902());

  pCommandCharacteristic->setValue("Awaiting Command");



  pDroneService->start();

  // === Battery Service ===
  BLEService *pBatteryService = pServer->createService(BATTERY_SERVICE_UUID);
  pBatteryLevelCharacteristic = pBatteryService->createCharacteristic(
    BATTERY_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );

  pBatteryLevelCharacteristic->addDescriptor(new BLE2902());
  pBatteryLevelCharacteristic->setValue((uint8_t*)"\x64", 1); // 100%
  pBatteryService->start();

  // === Telemetry Service ===
  BLEService *pTelemetryService = pServer->createService(TELEMETRY_SERVICE_UUID);
  pAltitudeCharacteristic = pTelemetryService->createCharacteristic(ALTITUDE_CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pSpeedCharacteristic = pTelemetryService->createCharacteristic(SPEED_CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pGPSCharacteristic = pTelemetryService->createCharacteristic(GPS_CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  pAltitudeCharacteristic->addDescriptor(new BLE2902());
  pSpeedCharacteristic->addDescriptor(new BLE2902());
  pGPSCharacteristic->addDescriptor(new BLE2902());

  pAltitudeCharacteristic->setValue("0.0 m");
  pSpeedCharacteristic->setValue("0.0 m/s");
  pGPSCharacteristic->setValue("0.000000, 0.000000");
  pTelemetryService->start();

  // === Status Service ===
  BLEService *pStatusService = pServer->createService(STATUS_SERVICE_UUID);
  pFlightModeCharacteristic = pStatusService->createCharacteristic(FLIGHT_MODE_CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pErrorStateCharacteristic = pStatusService->createCharacteristic(ERROR_STATE_CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  pFlightModeCharacteristic->addDescriptor(new BLE2902());
  pErrorStateCharacteristic->addDescriptor(new BLE2902());

  pFlightModeCharacteristic->setValue("Idle");
  pErrorStateCharacteristic->setValue("None");
  pStatusService->start();

  // === Start Advertising ===
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(DRONE_SERVICE_UUID);
  pAdvertising->addServiceUUID(BATTERY_SERVICE_UUID);
  pAdvertising->addServiceUUID(TELEMETRY_SERVICE_UUID);
  pAdvertising->addServiceUUID(STATUS_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("Drone BLE Server is up and running");
}

void loop() {
  // Simulate updating values every few seconds

  static uint8_t battery = 100;
  static float altitude = 0.0;
  static float speed = 0.0;
  static float lat = 59.3293; // Stockholm example
  static float lon = 18.0686;

  altitude += 0.1;
  speed += 0.05;
  battery = max(0, battery - 1);

  char altStr[10];
  sprintf(altStr, "%.1f m", altitude);
  pAltitudeCharacteristic->setValue(altStr);
  pAltitudeCharacteristic->notify();

  char speedStr[10];
  sprintf(speedStr, "%.1f m/s", speed);
  pSpeedCharacteristic->setValue(speedStr);
  pSpeedCharacteristic->notify();

  char gpsStr[30];
  sprintf(gpsStr, "%.5f, %.5f", lat, lon);
  pGPSCharacteristic->setValue(gpsStr);
  pGPSCharacteristic->notify();

  pBatteryLevelCharacteristic->setValue(&battery, 1);
  pBatteryLevelCharacteristic->notify();

  // Simulate mode change
  if (altitude > 1.0) {
    pFlightModeCharacteristic->setValue("Hover");
    pFlightModeCharacteristic->notify();
  }

  delay(5000);
}


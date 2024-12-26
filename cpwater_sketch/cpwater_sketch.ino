#include <ArduinoBLE.h>

BLEService sensorService("d888a9c2-f3cc-11ed-a05b-0242ac120003");
BLEFloatCharacteristic temperatureCharacteristic("d888a9c3-f3cc-11ed-a05b-0242ac120003", BLERead | BLENotify);

#define SensorPin A0 // pH meter Analog output is connected with the Arduino’s Analog

void setup() {
  Serial.begin(9600);
  
  if (!BLE.begin()) {
    Serial.println("BLE not started...");
    while (1);
  } else {
    Serial.println("BLE started...");

    BLE.setDeviceName("CPW-MD-1");
    BLE.setAdvertisedService(sensorService);
    sensorService.addCharacteristic(temperatureCharacteristic);

    BLE.addService(sensorService);

    BLE.advertise();
   }

  pinMode(13, OUTPUT);  
}

void loop() {
  // Continuously check if a central device (e.g., iOS app) connects to the BLE peripheral
  BLEDevice central = BLE.central();
  if (central && central.connected()) {
    Serial.print("Central connected to: ");
    Serial.println(central.address());

    float phValue = readPhValue();
    temperatureCharacteristic.writeValue(phValue);

    Serial.print("pH: ");  
    Serial.println(phValue, 2);
  } else {
    Serial.println("Central not connected");
  }

  delay(5000);
}

float readPhValue() {
  int buf[10], temp;

  // Get 10 sample value from the sensor for smooth the value
  for (int i = 0; i < 10; ++i) {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }

  // Sort the analog from small to large
  for (int i = 0; i < 9; ++i) {
    for (int j = i + 1; j < 10; ++j) {
      if (buf[i] > buf[j]) {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }

  unsigned long int averagePhValue = 0;
  for (int i = 2; i < 8; ++i) // Take the average value of 6 center sample
    averagePhValue += buf[i];

  float phValue = (float)averagePhValue * 5.0 / 1024 / 6; // Convert the analog into millivolt
  phValue = 3.5 * phValue; // Convert the millivolt into pH value

  return phValue;
}
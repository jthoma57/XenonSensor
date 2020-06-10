/*
 * Project XenonSensor
 * Description: Sets xenon to a BLE peripheral that sends sensor data.
 * Author: Justin Thomas
 * Date: 5/23/2020
 */

SYSTEM_MODE(MANUAL);

SerialLogHandler logHandler(115200, LOG_LEVEL_ERROR, {{"app", LOG_LEVEL_ALL}});

#define MAX_BATT_V 4.1
#define MIN_BATT_V 3.1
#define BATTERY_INTERVAL_MS 10000
#define SENSOR_INTERVAL_MS 500

// UUID for battery service
BleUuid batteryServiceUUID = BleUuid(0x180F);
BleUuid batteryCharUUID = BleUuid(0x2A19);

// UUID for sensor service + characteristics
const char *sensorServiceUuid = "1EDF9A1C-6426-4E87-9C6D-F7518017E1D7";
const char *sensorType = "1EDF9A1D-6426-4E87-9C6D-F7518017E1D7";
const char *sensorVoltage = "1EDF9A1E-6426-4E87-9C6D-F7518017E1D7";

// Set Sensor Service
BleUuid sensorService(sensorServiceUuid);

// Initialize char
BleCharacteristic batteryLevelCharacteristic;
BleCharacteristic sensorTypeCharacteristic;
BleCharacteristic sensorVoltageCharacteristic;

// Timer for measurements
system_tick_t batteryLastMeasurementMs = 0;
system_tick_t sensorLastMeasurementMs = 0;

// Variables for keeping state
int sensorPin = A5;

void batteryProcess()
{
  // Reset if overflow
  if (millis() < batteryLastMeasurementMs)
  {
    batteryLastMeasurementMs = millis();
  }

  // Check if it's time to make a measurement
  if (millis() > (batteryLastMeasurementMs + BATTERY_INTERVAL_MS))
  {
    batteryLastMeasurementMs = millis();

    float batteryVoltage = analogRead(BATT) * 0.0011224;
    float normalized = (batteryVoltage - MIN_BATT_V) / (MAX_BATT_V - MIN_BATT_V) * 100;

    // If normalized goes above or below the min/max, set to the min/max
    if (normalized > 100)
    {
      normalized = 100;
    }
    else if (normalized < 0)
    {
      normalized = 0;
    }

    // Set the battery value
    batteryLevelCharacteristic.setValue((uint8_t)normalized);

    // Print the results
    Log.info("Batt level: %d", (uint8_t)normalized);
  }
}

void getSensorData()
{
  // Reset if overflow
  if (millis() < sensorLastMeasurementMs)
  {
    sensorLastMeasurementMs = millis();
  }

  // Check if it's time to make a measurement
  if (millis() > (sensorLastMeasurementMs + SENSOR_INTERVAL_MS))
  {
    sensorLastMeasurementMs = millis();

    float voltage = (int)(analogRead(sensorPin) * 0.0008 * 100 + 0.5);
    //String x = String(voltage);

    // Set the sensor value
    sensorVoltageCharacteristic.setValue((float)voltage / 100);

    // Print the results
    //Log.info("Sensor voltage level: " + ((float)voltage/100));
  }
}

// setup() runs once, when the device is first turned on.
void setup()
{
  (void)logHandler;
  // Put initialization like pinMode and begin functions here.
  pinMode(sensorPin, INPUT);

  // Turn mesh off
  Mesh.off();

  // Enable app control of LED and turn it off
  RGB.control(true);
  RGB.color(0, 0, 0);

  // Set up characteristics
  sensorTypeCharacteristic = BleCharacteristic("Sensor Type", BleCharacteristicProperty::READ, sensorType, sensorServiceUuid);
  sensorVoltageCharacteristic = BleCharacteristic("Sensor Voltage", BleCharacteristicProperty::READ, sensorVoltage, sensorServiceUuid);
  batteryLevelCharacteristic = BleCharacteristic("bat", BleCharacteristicProperty::READ, batteryCharUUID, batteryServiceUUID);

  // Add the characteristics
  BLE.addCharacteristic(sensorTypeCharacteristic);
  BLE.addCharacteristic(sensorVoltageCharacteristic);
  BLE.addCharacteristic(batteryLevelCharacteristic);

  // Advertising data
  BleAdvertisingData advData;

  // Add device name
  advData.appendLocalName("SensorPad");

  // Add the RGB LED service
  advData.appendServiceUUID(sensorService);

  // Add the battery service
  advData.appendServiceUUID(batteryServiceUUID);

  // Start advertising!
  BLE.advertise(&advData);

  // Set sensor type
  sensorTypeCharacteristic.setValue("Chair");
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  getSensorData();
  batteryProcess();
}
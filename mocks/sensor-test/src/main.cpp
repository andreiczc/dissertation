#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <DHT.h>

class VibrationSensor
{
public:
  explicit VibrationSensor(short pinNumber) : pinNumber(pinNumber)
  {
    pinMode(this->pinNumber, INPUT);
  }

  boolean vibrationPresent() { return digitalRead(this->pinNumber) >= 1; }

private:
  short pinNumber;
};

class GasSensor
{
public:
  explicit GasSensor(short pinNumber) : pinNumber(pinNumber) {}

  int readValue() { return analogRead(this->pinNumber); }

private:
  short pinNumber;
};

static constexpr auto VIBRATION_PIN = 32;
static constexpr auto DHT_PIN       = 33;
static constexpr auto DHT_TYPE      = DHT11;
static constexpr auto GAS_PIN       = 35;

static DHT             dht(DHT_PIN, DHT_TYPE);
static VibrationSensor vibrationSensor(VIBRATION_PIN);
static GasSensor       gasSensor(GAS_PIN);

void setup()
{
  Serial.begin(115200);
  Serial.println("Commencing serial communication");

  dht.begin();
}

void loop()
{
  delay(5000);

  if (!dht.read())
  {
    Serial.println("Skipping one");

    return;
  }

  Serial.println("looping");
  Serial.printf("Temperature: %f\n", dht.readTemperature());
  Serial.printf("Humidity: %f\n", dht.readHumidity());
  Serial.printf("Vibration: %s\n",
                vibrationSensor.vibrationPresent() ? "True" : "False");
  Serial.printf("Gas: %d\n", gasSensor.readValue());
}
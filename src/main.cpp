// Weather station - NodeMCUv2 program for fetching weather data from sensor and sending it to AWS hosted API

// Last edit: 03.04.2022
// Author: Wiktor Szuca
// wszuc.github.io

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHTesp dht;
StaticJsonDocument<128> doc;
WiFiUDP udp;
NTPClient ntp(udp, "pool.ntp.org");

time_t downloadedTime = 0, dTime = 0, actualTime = downloadedTime + millis() / 1000 - dTime; // NTP fetched time, time elapsed since first launch, real calculated time
unsigned short temp, humid;                                                                  // sensor data
bool isClicked = 0;

const unsigned int buttonCooldown = 1000;

const std::string POSTPATH = "";

// Specific functions
// Formatting LED string

void monoButton()
{ // monostable button with cooldown
  Serial.println("Button pressed");
  isClicked = !isClicked;
}

void printSensorData(String firstLine, String secondLine, bool isWeatherData)
{
  lcd.clear();
  lcd.print(firstLine);
  if (isWeatherData)
  {
    lcd.print(temp);
    lcd.print(char(223));
    lcd.print("C");
  }
  lcd.setCursor(0, 1);
  lcd.print(secondLine);
  if (isWeatherData)
  {
    lcd.print(humid);
    lcd.print("%");
  }
}

time_t getTime()
{
  if (ntp.update())
  {
    return ntp.getEpochTime();
  }
  Serial.println("Error: time could not be fetched");
  return 0;
}

// JSON serialising

void serialise(unsigned short temp, unsigned short humid)
{ // serialise and save sensor data into json array
  doc["temperature"] = temp;
  doc["humidity"] = humid;
}

void setup()
{
  Serial.begin(115200);

  // activate button input

  pinMode(D6, INPUT);

  // LCD initialisation
  lcd.init();

  // Connecting to wifi
  const char *ssid = "UPC7823818";
  const char *pwd = "wTmpzYpyya3d";
  WiFi.begin(ssid, pwd);
  Serial.printf("Connecting to: ");
  Serial.printf(ssid);
  Serial.println(" ");
  lcd.print(" Connecting to:");
  lcd.setCursor(0, 1);
  lcd.print("   ");
  lcd.print(ssid);
  while (!WiFi.isConnected() && millis() % 1000 <= 5)
  {
    Serial.println(".");
  }
  Serial.println("\n*******\nConnected, your IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Fetching time");
  lcd.print("  Connected :)");

  // Setting timezone & fetching time
  ntp.setTimeOffset(1);
  downloadedTime = getTime();
  dTime = millis() / 1000;

  // ESP sensor initialisation
  dht.setup(0, DHTesp::DHT11);
}

void loop()
{
  // check state of the button
  if (digitalRead(12) == HIGH)
  {
    monoButton();
  }

  if (millis() % (dht.getMinimumSamplingPeriod() * 2) <= 10)
  {
    if (!isClicked)
    {
      temp = dht.getTemperature();
      humid = dht.getHumidity();

      Serial.println(downloadedTime);
      Serial.println(dht.getTemperature());
      Serial.println(dht.getHumidity());
      printSensorData((String)temp, (String)humid, true);
      serialise(temp, humid);
    }
    else
    {
      printSensorData("Voltage:", (String)(analogRead(A0)), false);
      Serial.println("Voltage: " + (String)analogRead(A0));
    }
  }
}
// Weather station - NodeMCUv2 program for fetching weather data from sensor and sending it to AWS hosted API

// Last edit: 03.04.2022
// Author: Wiktor Szuca
// wszuc.github.io

/*

TODO:

Strings dont work -> use c strings instead
Button is pressed initally -> fix

*/

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ctime>

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHTesp dht;
StaticJsonDocument<128> doc;
WiFiUDP udp;
NTPClient ntp(udp, "pool.ntp.org");

time_t downloadedTime = 0, dTime = 0, actualTime = 0, clickTime = 0, localTime = 0; // NTP fetched time, time elapsed since first launch, real calculated time
unsigned short temp, humid;                                                         // sensor data
bool isClicked = 0;

const unsigned int buttonCooldown = 1;

const std::string POSTPATH = "";

// Specific functions
// Formatting LED string

void monoButton(time_t actualTime)
{ // monostable button with cooldown
  if (clickTime + buttonCooldown <= actualTime)
  {
    isClicked = !isClicked;
    clickTime = actualTime;
    Serial.println("Monobutton");
  }
}

void printSensorData(String firstLine, String secondLine)
{
  lcd.clear();
  lcd.print(firstLine);
  lcd.setCursor(0, 1);
  lcd.print(secondLine);
}

void printSensorData(unsigned short firstLine, unsigned short secondLine, bool isWeatherData)
{
  lcd.clear();
  lcd.print("Temp: ");
  lcd.print(firstLine);
  if (isWeatherData)
  {
    lcd.print(char(223));
    lcd.print("C");
  }
  lcd.setCursor(0, 1);
  lcd.print("Humid: ");
  lcd.print(secondLine);
  if (isWeatherData)
  {
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
  while (!WiFi.isConnected())
  {
    Serial.println(".");
  }
  Serial.println("\n*******\nConnected, your IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Fetching time");
  lcd.clear();
  lcd.print("  Connected :)");

  // Setting timezone & fetching time
  ntp.setTimeOffset(1);
  downloadedTime = getTime();
  dTime = millis() / 1000;

  // ESP sensor initialisation
  dht.setup(0, DHTesp::DHT11);

  // Time elapsed since the start
  dTime = millis() / 1000;
}

void loop()
{
  actualTime = downloadedTime + millis() / 1000 - dTime;
  // Local time is 2h ahead of epoch time
  localTime = actualTime + 7200;

  // check state of the button
  if (digitalRead(12) == HIGH)
  {
    Serial.println("Button pressed");
    monoButton(actualTime);
  }

  if (millis() % (dht.getMinimumSamplingPeriod() * 2) <= 10)
  {
    if (!isClicked)
    {
      temp = dht.getTemperature();
      humid = dht.getHumidity();
      Serial.println(temp);
      printSensorData(temp, humid, true);
      serialise(temp, humid);
    }
    else
    {
      printSensorData("Voltage: " + (String)(analogRead(A0) / 204.8) + "V", std::asctime(std::localtime(&localTime)));
    }
  }
}
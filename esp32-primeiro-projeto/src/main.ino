// Uncomment the following line to enable serial debug output
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define DEBUG_ESP_PORT Serial
#define NODEBUG_WEBSOCKETS
#define NDEBUG
#endif

// ESP32 Wifi
#include <WiFi.h>
#include <Arduino.h>
#include <SinricPro.h>
#include "SmartWindow.h"

#define APP_KEY "ab811fde-cf36-48f2-ad19-d40d260c2684"
#define APP_SECRET "02f82d71-fe22-4839-92e4-65adec11fc29-5873019c-f4c9-47ad-8c49-a56ef52f4f22"
#define DEVICE_ID "613df2b62c014831f824a6dc"

#define SSID "Valmira"
#define PASS "@val515856"

#define Led1Pin 19 //  "Aberto"
#define Led2Pin 18 //  "Fechado"

#define SmokePin 12 //  "Gás"
#define RainPin 34  //  "Chuva"

bool command = true;

#define BAUD_RATE 9600

SmartWindow &smartWindow = SinricPro[DEVICE_ID];

/*************
 * Variables *
 ***********************************************
 * Global variables to store the device states *
 ***********************************************/

// ModeController
std::map<String, String> globalModes;

/*************
 * Callbacks *
 *************/

// ModeController
bool onSetMode(const String &deviceId, const String &instance, String &mode)
{
  Serial.printf("[Device: %s]: Modesetting for \"%s\" set to mode %s\r\n", deviceId.c_str(), instance.c_str(), mode.c_str());
  globalModes[instance] = mode;

  if (mode.charAt(0) == 'A')
  {
    digitalWrite(Led1Pin, HIGH);
    digitalWrite(Led2Pin, LOW);
  }
  else if (mode.charAt(0) == 'F')
  {
    digitalWrite(Led2Pin, HIGH);
    digitalWrite(Led1Pin, LOW);
  }

  return true;
}

/**********
 * Events *
 *************************************************
 * Examples how to update the server status when *
 * you physically interact with your device or a *
 * sensor reading changes.                       *
 *************************************************/

// ModeController
void updateMode(String instance, String mode)
{
  smartWindow.sendModeEvent(instance, mode, "PHYSICAL_INTERACTION");
}

/********* 
 * Setup *
 *********/

void setupSinricPro()
{
  // ModeController
  smartWindow.onSetMode("windowState", onSetMode);

  SinricPro.onConnected([]
                        { Serial.printf("[SinricPro]: Connected\r\n"); });
  SinricPro.onDisconnected([]
                           { Serial.printf("[SinricPro]: Disconnected\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
};

void setupWiFi()
{
  WiFi.begin(SSID, PASS);
  Serial.printf("[WiFi]: Connecting to %s", SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf(".");
    delay(250);
  }
  Serial.printf("connected\r\n");
}

void setup()
{
  Serial.begin(BAUD_RATE);
  pinMode(Led1Pin, OUTPUT);
  pinMode(Led2Pin, OUTPUT);

  pinMode(RainPin, INPUT); // Min value = 4096 / MaxValue = 0
  pinMode(SmokePin, INPUT);

  digitalWrite(Led1Pin, HIGH);
  setupWiFi();
  setupSinricPro();
}

/********
 * Loop *
 ********/

void loop()
{
  int rainValue = analogRead(RainPin);
  int smokeValue = analogRead(SmokePin);
  SinricPro.handle();

  //Serial.println(rainValue);
  //Serial.println(globalModes["windowState"]);

  if (rainValue < 3000) //  is raining
  {
    Serial.println(rainValue);

    if (isWindowOpen())
      closeWindow();
    //command = false;
  }

  /*
  if (smokeValue < 700) //  is smokey
  {
    command = true;
  }*/

  /*
  if (command && !isWindowOpen())
  {
    openWindow();
  }
  else if (!command && isWindowOpen())
  {
    closeWindow();
  }*/
}

bool isWindowOpen()
{
  if (globalModes.at("windowState") == NULL)
    return true;

  return globalModes.at("windowState") == "Aberto";
}

void openWindow()
{
  digitalWrite(Led1Pin, HIGH);
  digitalWrite(Led2Pin, LOW);
  // updateMode("windowState", "Aberto");
}

void closeWindow()
{
  digitalWrite(Led2Pin, HIGH);
  digitalWrite(Led1Pin, LOW);
  //updateMode("windowState", "Fechado");
}
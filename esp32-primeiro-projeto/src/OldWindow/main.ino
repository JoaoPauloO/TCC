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

#define SmokePin 32 //  "Gás"
#define RainPin 33  //  "Chuva"

#define motorA 27 // Pinos que irão controlar o estado do motor
#define motorB 26

char command = 'P';
bool isConnectedToSinric = false;
String state;
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
    digitalWrite(motorA, HIGH);
    digitalWrite(motorB, LOW);
    delay(5000);
    digitalWrite(motorA, LOW);
    digitalWrite(motorB, LOW);

    state = "Aberto";
  }
  else if (mode.charAt(0) == 'F')
  {
    digitalWrite(Led2Pin, HIGH);
    digitalWrite(Led1Pin, LOW);
    digitalWrite(motorA, LOW);
    digitalWrite(motorB, HIGH);
    delay(5000);
    digitalWrite(motorA, LOW);
    digitalWrite(motorB, LOW);

    state = "Fechado";
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
                        {
                          Serial.printf("[SinricPro]: Connected\r\n");
                          isConnectedToSinric = true;
                          //openWindow();
                        });
  SinricPro.onDisconnected([]
                           {
                             Serial.printf("[SinricPro]: Disconnected\r\n");
                             isConnectedToSinric = false;
                           });
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

  pinMode(RainPin, INPUT); // Min value = 4095 / MaxValue = 0
  pinMode(SmokePin, INPUT);

  pinMode(motorA, OUTPUT);
  pinMode(motorB, OUTPUT);

  digitalWrite(motorA, LOW);
  digitalWrite(motorB, LOW);

  setupWiFi();
  setupSinricPro();
}

/********
 * Loop *
 ********/

void loop()
{
  SinricPro.handle();
  if (isConnectedToSinric)
  {
    command = 'P';
    int rainValue = analogRead(RainPin);
    int smokeValue = analogRead(SmokePin);

    if (rainValue < 3000) //  is raining
    {
      command = 'F';
    }

    if (smokeValue > 700) // is smokey
    {
      command = 'A';
    }
    windowHandle();
  }
}

bool isWindowOpen()
{
  return state == "Aberto";
}

void windowHandle()
{
  if (command == 'A' && !isWindowOpen())
    openWindow();
  if (command == 'F' && isWindowOpen())
    closeWindow();
}

void openWindow()
{
  Serial.printf("Abrindo janela");
  digitalWrite(Led1Pin, HIGH);
  digitalWrite(Led2Pin, LOW);
  digitalWrite(motorA, HIGH);
  digitalWrite(motorB, LOW);
  delay(5000);
  digitalWrite(motorA, LOW);
  digitalWrite(motorB, LOW);
  state = "Aberto";

  updateMode("windowState", "Aberto");
}

void closeWindow()
{
  Serial.printf("Fechando janela");
  digitalWrite(Led2Pin, HIGH);
  digitalWrite(Led1Pin, LOW);
  digitalWrite(motorA, LOW);
  digitalWrite(motorB, HIGH);
  delay(5000);
  digitalWrite(motorA, LOW);
  digitalWrite(motorB, LOW);
  state = "Fechado";

  updateMode("windowState", "Fechado");
}
#ifdef ENABLE_DEBUG
#define DEBUG_ESP_PORT Serial
#define NODEBUG_WEBSOCKETS
#define NDEBUG
#endif

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#endif

#include <SinricPro.h>
#include "JanelaPRO.h"

#define APP_KEY "da5be0f3-aded-450b-b901-5cf168855331"
#define APP_SECRET "76f00ecd-b2fc-419e-91f9-81bb41369b10-d196d710-b7c8-4053-a306-904050563052"
#define DEVICE_ID "6185f2ff0e8d611820f4a43f"

#define SSID "NOME_WIFI"
#define PASS "SENHA"

#define BAUD_RATE 9600

#define SmokePin 32 //  "Gás"
#define RainPin 33  //  "Chuva"

#define motorA 27 // Pinos que irão controlar o motor
#define motorB 26

#define hallPin1 34
#define hallPin2 35

char command = 'P';
bool isConnectedToSinric = false;
int rainDetectionValue = 3000;
int smokeDetectionValue = 700;

String state;

JanelaPRO &janelaPRO = SinricPro[DEVICE_ID];


// ModeController
std::map<String, String> globalModes;

// RangeController
std::map<String, int> globalRangeValues;


// ModeController
bool onSetMode(const String &deviceId, const String &instance, String &mode)
{
  Serial.printf("[Device: %s]: Modesetting for \"%s\" set to mode %s\r\n", deviceId.c_str(), instance.c_str(), mode.c_str());
  globalModes[instance] = mode;

  if (mode.charAt(0) == 'A')
  {
    openWindow();
  }
  else if (mode.charAt(0) == 'F')
  {
    closeWindow();
  }

  return true;
}

// RangeController
bool onRangeValue(const String &deviceId, const String &instance, int &rangeValue)
{
  Serial.printf("[Device: %s]: Value for \"%s\" changed to %d\r\n", deviceId.c_str(), instance.c_str(), rangeValue);
  globalRangeValues[instance] = rangeValue;

  String inst = instance.c_str();
 
  if (inst == "gasRange")
    smokeDetectionValue = rangeValue;
  else
    rainDetectionValue = rangeValue;

  return true;
}

bool onAdjustRangeValue(const String &deviceId, const String &instance, int &valueDelta)
{
  globalRangeValues[instance] += valueDelta;
  Serial.printf("[Device: %s]: Value for \"%s\" changed about %d to %d\r\n", deviceId.c_str(), instance.c_str(), valueDelta, globalRangeValues[instance]);
  globalRangeValues[instance] = valueDelta;
  return true;
}


// ModeController
void updateMode(String instance, String mode)
{
  janelaPRO.sendModeEvent(instance, mode, "PHYSICAL_INTERACTION");
}

// RangeController
void updateRangeValue(String instance, int value)
{
  janelaPRO.sendRangeValueEvent(instance, value);
}

/********* 
 * Setup *
 *********/

void setupSinricPro()
{

  // ModeController
  janelaPRO.onSetMode("windowState", onSetMode);

  // RangeController
  janelaPRO.onRangeValue("rainRange", onRangeValue);
  janelaPRO.onAdjustRangeValue("rainRange", onAdjustRangeValue);
  janelaPRO.onRangeValue("gasRange", onRangeValue);
  janelaPRO.onAdjustRangeValue("gasRange", onAdjustRangeValue);

  SinricPro.onConnected([]
                        {
                          Serial.printf("[SinricPro]: Connected\r\n");
                          isConnectedToSinric = true;
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

  pinMode(RainPin, INPUT); 
  pinMode(SmokePin, INPUT);

  pinMode(motorA, OUTPUT);
  pinMode(motorB, OUTPUT);

  pinMode(hallPin1, INPUT);
  pinMode(hallPin2, INPUT);

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

     if (rainValue < rainDetectionValue) //  is raining
     {
       command = 'F';
     }

     if (smokeValue > smokeDetectionValue) // is smokey
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
  {
    openWindow();
    updateMode("windowState", "Aberto");
  }
  if (command == 'F' && isWindowOpen())
  {
    closeWindow();
    updateMode("windowState", "Fechado");
  }
}

void openWindow()
{
  Serial.printf("Abrindo janela");

  digitalWrite(motorA, HIGH);
  digitalWrite(motorB, LOW);

  Serial.println(digitalRead(hallPin2));
  while (analogRead(hallPin2) > 2500)
  {
  }

  digitalWrite(motorA, LOW);
  digitalWrite(motorB, LOW);
  state = "Aberto";
}

void closeWindow()
{
  Serial.printf("Fechando janela");

  digitalWrite(motorA, LOW);
  digitalWrite(motorB, HIGH);

  Serial.println(digitalRead(hallPin1));
  while (analogRead(hallPin1) > 2500)
  {
  }

  digitalWrite(motorA, LOW);
  digitalWrite(motorB, LOW);
  state = "Fechado";
}
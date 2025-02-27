#include <Arduino.h>
#include <vector>
#include <Arduino_FreeRTOS.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

#define LED_Older_1 3
#define LED_Older_2 4
#define LED_Older_3 5
#define LED_Older_4 6
#define LED_Older_5 7

#define LED_Kid_1 8
#define LED_Kid_2 9
#define LED_Kid_3 10


#define LED_Kid_4 11
#define LED_Kid_5 12
#define LED_Kid_6 13

#define Button 2


TaskHandle_t ledTaskHandle;
TaskHandle_t buttonTaskHandle;
TaskHandle_t selectStatusHandle;
TaskHandle_t selectButtonHandle;
TaskHandle_t mqttRconnectTaskHandle;

volatile bool buttonPressed = false; 
volatile int currentMode = 0; 
volatile bool taskStopped = false;
volatile bool selectButtonPressed = false;
volatile int currentOlederLEDIndex_case1 = 0;
volatile int currentOlederLEDIndex_case2 = 5;
volatile int currentKidLEDIndex_case1 = 0;
volatile int currentKidLEDIndex_case2 = 3;

std::vector<int> olderLEDPins = {LED_Older_1, LED_Older_2, LED_Older_3, LED_Older_4, LED_Older_5};
std::vector<int> kidLEDPins1 = {LED_Kid_1,LED_Kid_2,LED_Kid_3};
std::vector<int> kidLEDPins2 = {LED_Kid_4,LED_Kid_5,LED_Kid_6};
std::vector<int> activeLEDs;

std::vector<int> getCurrentActiveLEDs() {
      return activeLEDs;
  }

int getCurrentMode() {
    return currentMode;
}

//mqtt setting
const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = SECRET_MQTTSERVER;
const int mqtt_port       = SECRET_MQTT_HOST;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
const char* subscribeTopic1 = "oldLED/currentStatus";
const char* subscribeTopic2 = "kidLED1/currentStatus";
const char* subscribeTopic3 = "kidLED2/currentStatus";

void setup_wifi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("MQTT message received [");
    Serial.print(topic);
    Serial.print("]: ");
    
    String receivedMessage = "";
    for (int i = 0; i < length; i++) {
        receivedMessage += (char)payload[i];
    }
    Serial.println(receivedMessage);

    if (String(topic) == subscribeTopic1) {
        Serial.println("Processing Older LED status...");
        int ledIndex = receivedMessage.toInt();
        if (ledIndex >= 1 && ledIndex <= 5) {
            for (int pin : olderLEDPins) {
                digitalWrite(pin, LOW);
            }
            int ledPin = olderLEDPins[ledIndex - 1]; 
            Serial.print("Turning ON Older LED: ");
            Serial.println(ledPin);
            digitalWrite(ledPin, HIGH);
        } else {
            Serial.println("Invalid LED index received.");
        }
    } else if (String(topic) == subscribeTopic2) {
        Serial.println("Processing Kid LED1 status...");
         int ledIndex = receivedMessage.toInt();
        if (ledIndex >= 1 && ledIndex <= 3) {
            for (int pin : kidLEDPins1) {
                digitalWrite(pin, LOW);
            }
            int ledPin = kidLEDPins1[ledIndex - 1]; 
            Serial.print("Turning ON kid LED: ");
            Serial.println(ledPin);
            digitalWrite(ledPin, HIGH);
        } else {
            Serial.println("Invalid LED index received.");
        }
    } else if (String(topic) == subscribeTopic3) {
        Serial.println("Processing Kid LED2 status...");
        int ledIndex = receivedMessage.toInt();
        if (ledIndex >= 1 && ledIndex <= 3) {
            for (int pin : kidLEDPins2) {
                digitalWrite(pin, LOW);
            }
            int ledPin = kidLEDPins2[ledIndex - 1]; 
            Serial.print("Turning ON kid LED: ");
            Serial.println(ledPin);
            digitalWrite(ledPin, HIGH);
        }
    }
}

void mqttsendmessage() {
  if (!client.connected()) {
    Serial.println("MQTT client not connected. Message not sent.");
    return;
  }

  int mode = getCurrentMode();
  const char* topic = nullptr;
  int ledindex = -1;

  std::vector<int> activeLEDsNow = getCurrentActiveLEDs();
  if (activeLEDsNow.empty()) {
    Serial.println("No active LEDs to publish.");
    return;
  }

  int activePin = activeLEDsNow[0]; 

  switch (mode) {
    case 1:
      topic = subscribeTopic1;
      for (size_t i = 0; i < olderLEDPins.size(); ++i) {
        if (olderLEDPins[i] == activePin) {
          ledindex = i + 1;
          break;
        }
      }
      break;
    case 2:
      topic = subscribeTopic2;
      for (size_t i = 0; i < kidLEDPins1.size(); ++i) {
        if (kidLEDPins1[i] == activePin) {
          ledindex = i + 1;
          break;
        }
      }
      break;
    case 3:
      topic = subscribeTopic3;
      for (size_t i = 0; i < kidLEDPins2.size(); ++i) {
        if (kidLEDPins2[i] == activePin) {
          ledindex = i + 1;
          break;
        }
      }
      break;
    default:
      Serial.println("Invalid mode. Message not sent.");
      return;
  }

  if (ledindex == -1) {
    Serial.println("Active LED not recognized for current mode.");
    return;
  }

  String message = String(ledindex);
  if (client.publish(topic, message.c_str())) {
    Serial.print("Published to ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(message);
  } else {
    Serial.println("Publish failed.");
  }
}

void mqttReconnect(void *parameter){
  while (true) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, reconnecting...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("\nWiFi reconnected.");
    }

    if (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        while (!client.connected()) {
            if (client.connect("mqttClient", mqtt_username, mqtt_password)) {
                Serial.println("Connected to MQTT server");
                client.subscribe(subscribeTopic1);
                client.subscribe(subscribeTopic2);
                client.subscribe(subscribeTopic3);
                Serial.println("Subscribed to topics");

                // if (client.connected()) {
                //     client.subscribe(subscribeTopic1);
                //     client.subscribe(subscribeTopic2);
                //     client.subscribe(subscribeTopic3);
                //     Serial.println("Subscribed to topics");
                // }
            } else {
                Serial.print("Failed, status: ");
                Serial.print(client.state());
                Serial.println(" Retrying in 2 seconds...");
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
        }
    }
    client.loop();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void checkButton1(void *parameter) {
    while (1) {
        int buttonState = digitalRead(Button);

        if (!buttonPressed && buttonState == LOW) { 
            buttonPressed = true;
            taskStopped = true; 

            for (int pin : {LED_Older_1, LED_Older_2, LED_Older_3, LED_Older_4, LED_Older_5,
                            LED_Kid_1, LED_Kid_2, LED_Kid_3, LED_Kid_4, LED_Kid_5, LED_Kid_6}) {
              digitalWrite(pin, LOW);
            }

            if (ledTaskHandle != NULL) {
                vTaskDelete(ledTaskHandle);
                ledTaskHandle = NULL;
            }

            if (selectStatusHandle == NULL) {
                xTaskCreate(selectStatus, "selectStatus", 256, NULL, 1, &selectStatusHandle);
            }

            vTaskDelay(pdMS_TO_TICKS(500));
        }

        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void selectStatus(void *parameter) {
  vTaskDelay(pdMS_TO_TICKS(1000));
  xTaskCreate(selectButton, "selectButtonCheck", 256, NULL, 1, &selectButtonHandle);
  while (1) {
    switch (currentMode) {
        case 1:
          for (int i = 0; i < 5; i++) {
              if (selectButtonPressed) break;
              digitalWrite(olderLEDPins[currentOlederLEDIndex_case1], HIGH);
              vTaskDelay(pdMS_TO_TICKS(500));
              digitalWrite(olderLEDPins[currentOlederLEDIndex_case1], LOW);
              vTaskDelay(pdMS_TO_TICKS(500));
          }

          if (!selectButtonPressed) {
              activeLEDs.clear();
              digitalWrite(olderLEDPins[currentOlederLEDIndex_case1], HIGH);
              activeLEDs.push_back(olderLEDPins[currentOlederLEDIndex_case1]);
              mqttsendmessage();
          }

          while (!selectButtonPressed) {
              vTaskDelay(pdMS_TO_TICKS(100));
          }

          digitalWrite(olderLEDPins[currentOlederLEDIndex_case1], LOW);
          selectButtonPressed = false;

          // activeLEDs.clear();

          currentOlederLEDIndex_case1 = (currentOlederLEDIndex_case1 + 1) % 5;
          // activeLEDs.push_back(olderLEDPins[currentOlederLEDIndex_case1]);
          // mqttsendmessage();
          // client.subscribe(subscribeTopic2);
          // client.subscribe(subscribeTopic3);
          break;

        case 2:
         for (int i = 0; i < 5; i++) {
            if (selectButtonPressed) break;
              digitalWrite(kidLEDPins1[currentKidLEDIndex_case1], HIGH);
              vTaskDelay(pdMS_TO_TICKS(500));
              digitalWrite(kidLEDPins1[currentKidLEDIndex_case1], LOW);
              vTaskDelay(pdMS_TO_TICKS(500));
            }

            if (!selectButtonPressed) {
              activeLEDs.clear();
              digitalWrite(kidLEDPins1[currentKidLEDIndex_case1], HIGH);
              activeLEDs.push_back(kidLEDPins1[currentKidLEDIndex_case1]);
              mqttsendmessage();
            }

            while (!selectButtonPressed) {
              vTaskDelay(pdMS_TO_TICKS(100));
            }

            digitalWrite(kidLEDPins1[currentKidLEDIndex_case1], LOW);
            selectButtonPressed = false;

            // activeLEDs.clear();
            currentKidLEDIndex_case1 = (currentKidLEDIndex_case1 + 1) % 3;
            // activeLEDs.push_back(kidLEDPins1[currentKidLEDIndex_case1]);
            // mqttsendmessage();
            // client.subscribe(subscribeTopic1);
            // client.subscribe(subscribeTopic3);
            break;

        case 3:
         for (int i = 0; i < 5; i++) {
            if (selectButtonPressed) break;
              digitalWrite(kidLEDPins2[currentKidLEDIndex_case2], HIGH);
              vTaskDelay(pdMS_TO_TICKS(500));
              digitalWrite(kidLEDPins2[currentKidLEDIndex_case2], LOW);
              vTaskDelay(pdMS_TO_TICKS(500));
            }

            if (!selectButtonPressed) {
              activeLEDs.clear();
              digitalWrite(kidLEDPins2[currentKidLEDIndex_case2], HIGH);
              activeLEDs.push_back(kidLEDPins2[currentKidLEDIndex_case2 - 3]);
            
              mqttsendmessage();
            }

            while (!selectButtonPressed) {
              vTaskDelay(pdMS_TO_TICKS(100));
            }

            digitalWrite(kidLEDPins2[currentKidLEDIndex_case2], LOW);
            selectButtonPressed = false;

            // activeLEDs.clear();

            currentKidLEDIndex_case2 = 3 + ((currentKidLEDIndex_case2 - 3 + 1) % 3);
            // activeLEDs.push_back(kidLEDPins2[currentKidLEDIndex_case2 - 3]);
            
            // mqttsendmessage();
            // client.subscribe(subscribeTopic1);
            // client.subscribe(subscribeTopic2);
            break;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void selectButton(void *parameter) {
    while (1) {  
        int buttonState = digitalRead(Button);
        if (!selectButtonPressed && buttonState == LOW) { 
            selectButtonPressed = true;
        }
        vTaskDelay(pdMS_TO_TICKS(100));  
    }
}



void ledTask(void *parameter) {
   TickType_t xLastWakeTime = xTaskGetTickCount();
  while(1){
    if (taskStopped) { 
      vTaskDelete(NULL); 
    }

    currentMode = 1;
    for (int i = 0; i < 5; i++) {
        if (taskStopped) break;
        for (int pin : {LED_Older_1, LED_Older_2, LED_Older_3, LED_Older_4, LED_Older_5}) {
          digitalWrite(pin, HIGH);
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
        for (int pin : {LED_Older_1, LED_Older_2, LED_Older_3, LED_Older_4, LED_Older_5}) {
          digitalWrite(pin, LOW);
        }
        activeLEDs.clear();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }

    currentMode = 2;
    for (int i = 0; i < 5; i++) {
        if (taskStopped) break;
        for (int pin : {LED_Kid_1, LED_Kid_2, LED_Kid_3}) {
            digitalWrite(pin, HIGH);
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
        for (int pin : {LED_Kid_1, LED_Kid_2, LED_Kid_3}) {
            digitalWrite(pin, LOW);
        }
        activeLEDs.clear();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }

    currentMode = 3;
    for (int i = 0; i < 5; i++) {
        if (taskStopped) break;
        for (int pin : {LED_Kid_4, LED_Kid_5, LED_Kid_6}) {
          digitalWrite(pin, HIGH);
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
        for (int pin : {LED_Kid_4, LED_Kid_5, LED_Kid_6}) {
          digitalWrite(pin, LOW);
        }
        activeLEDs.clear();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
  }  
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  for (int pin : {LED_Older_1, LED_Older_2, LED_Older_3, LED_Older_4, LED_Older_5,
                    LED_Kid_1, LED_Kid_2, LED_Kid_3, LED_Kid_4, LED_Kid_5, LED_Kid_6}) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
  }
  pinMode(Button, INPUT_PULLUP);  

  xTaskCreate(ledTask, "LED Task", 256, NULL, 1, &ledTaskHandle);
  xTaskCreate(checkButton1, "Check Button", 256, NULL, 1, &buttonTaskHandle);
  xTaskCreate(mqttReconnect, "mqttReconnect", 512, NULL, 1, &mqttRconnectTaskHandle);
  vTaskStartScheduler();
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(100)); 
}
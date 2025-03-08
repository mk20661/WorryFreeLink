#include <WiFiS3.h>
#include <Arduino.h>
#include <vector>
#include <Arduino_FreeRTOS.h>
#include "arduino_secrets.h"
#include <WiFi.h>
#include <PubSubClient.h>


#define Button 11
#define RestButton 12

#define LED_Older_1 0
#define LED_Older_2 1
#define LED_Older_3 4

#define LED_working_1 5
#define LED_free_to_connect_1 8

std::vector<int> olderLEDPins = {LED_Older_3,LED_Older_1, LED_Older_2};
std::vector<int> kidLEDPins1 = {LED_working_1,LED_free_to_connect_1};

std::vector<int> activeLEDs;
std::vector<int> getCurrentActiveLEDs() {
      return activeLEDs;
  }

WiFiServer server(80);
const char *apSSID = "WiFi_Config";
const char *apPassword = "";

char staSSID[32] = "";    
char staPassword[32] = "";

//mqtt setting
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = SECRET_MQTTSERVER;
const int mqtt_port       = SECRET_MQTT_HOST;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
const char* subscribeTopic1 = "oldLED/currentStatus";
const char* subscribeTopic2 = "kidLED1/currentStatus";

TaskHandle_t wifiTaskHandle;
TaskHandle_t selectStatusHandle;
TaskHandle_t restButtonTaskHandle;
TaskHandle_t selectButtonHandle;
TaskHandle_t mqttRconnectTaskHandle;

volatile bool selectButtonPressed = false;
volatile int currentKidLED1Index_case = 0;

void setup() {
   Serial.begin(115200);
   delay(1000);
  
   for (int pin : {LED_Older_1, LED_Older_2, LED_Older_3,
                  LED_working_1, LED_free_to_connect_1 }) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
   }
    pinMode(Button, INPUT_PULLUP);
    pinMode(RestButton, INPUT_PULLUP);  
  if (xTaskCreate(WiFiTask, "WiFi Task", 512, NULL, 1, &wifiTaskHandle) != pdPASS) {
      Serial.println("Failed to create WiFiTask!");
  } else {
      Serial.println("WiFiTask created successfully.");
  }

  if (xTaskCreate(restButtonTask, "restbutton", 128, NULL, 1, &restButtonTaskHandle) != pdPASS) {
    Serial.println("Failed to create restButtonTask Task!");
  } else 
  {
    Serial.println("restButtonTask Task created successfully.");
  }
   client.setCallback(callback);
   vTaskStartScheduler();  
}


void loop() {
}

void WiFiTask(void *parameter) {
    WiFi.beginAP(apSSID, apPassword);
    Serial.println("AP mode started, SSID: " + String(apSSID));
    Serial.println("Connect to this WiFi and go to 192.168.4.1 in your browser.");
    server.begin();
    
    while (true) {
        WiFiClient client = server.available();
        if (client) {
            Serial.println("Client connected");
            String request = "";
            while (client.available()) {
                char c = client.read();
                request += c;
            }
            
            if (request.indexOf("GET / ") >= 0) {
                sendWiFiForm(client);
            } else if (request.indexOf("GET /config?ssid=") >= 0) {
                handleWiFiConfig(client, request);
            }
            client.stop();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void selectStatus(void *parameter) {
  vTaskDelay(pdMS_TO_TICKS(1000));
  xTaskCreate(selectButton, "selectButtonCheck", 128, NULL, 1, &selectButtonHandle);
  while (1) {
    for (int i = 0; i < 5; i++) {
        if (selectButtonPressed) break;
        digitalWrite(kidLEDPins1[currentKidLED1Index_case], HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(kidLEDPins1[currentKidLED1Index_case], LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (!selectButtonPressed) {
          activeLEDs.clear();
          digitalWrite(kidLEDPins1[currentKidLED1Index_case], HIGH);
          activeLEDs.push_back(kidLEDPins1[currentKidLED1Index_case]);
          mqttsendmessage();
    }

    while (!selectButtonPressed) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    digitalWrite(kidLEDPins1[currentKidLED1Index_case], LOW);
    selectButtonPressed = false;
    currentKidLED1Index_case = (currentKidLED1Index_case + 1) % 2;
        
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void selectButton(void *parameter) {
    while (1) {  
        int buttonState = digitalRead(Button);
        if (!selectButtonPressed && buttonState == HIGH) { 
            selectButtonPressed = true;
        }
        vTaskDelay(pdMS_TO_TICKS(100));  
    }
}

void restButtonTask(void *pvParameters) {
    while (1) {
        if (digitalRead(RestButton) == HIGH) { 
            vTaskDelay(pdMS_TO_TICKS(1000)); 
            NVIC_SystemReset();
        }
        vTaskDelay(pdMS_TO_TICKS(100));  
    }
}

void mqttReconnect(void *parameter) {
    Serial.println("start mqtt");
   while (true) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, reconnecting...");
      WiFi.disconnect();
      WiFi.begin(staSSID, staPassword);
      while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(500));  
        Serial.print(".");
      }
      Serial.println("\nWiFi reconnected.");
    }
    while (!client.connected()) {
      if (client.connect("mqttClient", mqtt_username, mqtt_password)) {
          Serial.println("Connected to MQTT server");
          client.subscribe(subscribeTopic1);
          client.subscribe(subscribeTopic2);
          Serial.println("Subscribed to topics");
      } else {
          Serial.print("Failed, status: ");
          Serial.print(client.state());
          Serial.println(" Retrying in 0.5 seconds...");
          vTaskDelay(pdMS_TO_TICKS(500));
      }
    }
    client.loop();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void handleWiFiConfig(WiFiClient &client, String request) {
    int ssidStart = request.indexOf("ssid=") + 5;
    int ssidEnd = request.indexOf("&", ssidStart);
    String ssid = request.substring(ssidStart, ssidEnd);

    int passStart = request.indexOf("password=") + 9;
    int passEnd = request.indexOf(" ", passStart);
    String password = request.substring(passStart, passEnd);

    Serial.println("Selected WiFi SSID: " + ssid);
    Serial.println("Entered WiFi Password: " + password);
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<html><head><meta charset='utf-8'><title>WiFi Setup</title>");
    client.println("<meta http-equiv='refresh' content='5;url=/'>"); 
    client.println("</head><body>");
    client.println("<h1>WiFi Connecting......</h1>");
    client.println("<p>Connecting to: " + ssid + "</p>");
    client.println("<p>Please wait...</p>");
    client.println("</body></html>");
    client.flush();

    ssid.toCharArray(staSSID, sizeof(staSSID));
    password.toCharArray(staPassword, sizeof(staPassword));
    Serial.println("Attempting to connect to STA: " + String(staSSID));

    delay(1000);

    WiFi.end();
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("Connecting to WiFi...");
    
    int retry = 0;
    while (retry < 15) {
        if (WiFi.status() == WL_CONNECTED) {

            Serial.println("WiFi Connected!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            for (int i = 0; i < 3; i++) {
                digitalWrite(olderLEDPins[0], HIGH);
                digitalWrite(olderLEDPins[1], HIGH);
                digitalWrite(olderLEDPins[2], HIGH);
                vTaskDelay(pdMS_TO_TICKS(500));
                digitalWrite(olderLEDPins[0], LOW);
                digitalWrite(olderLEDPins[1], LOW);
                digitalWrite(olderLEDPins[2], LOW);
                vTaskDelay(pdMS_TO_TICKS(500));
            }
            startApplicationTasks(); 
            vTaskDelete(wifiTaskHandle); 
            return;
        }
        Serial.println("Trying to connect...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        retry++;
    }
    Serial.println("WiFi Connection Failed!");
    NVIC_SystemReset();

}

String getWiFiList() {
    String wifiOptions = "";
    int numNetworks = WiFi.scanNetworks();
    String seenSSIDs[numNetworks];
    int seenCount = 0;

    for (int i = 0; i < numNetworks; i++) {
        String ssid = WiFi.SSID(i);
        bool isDuplicate = false;
        for (int j = 0; j < seenCount; j++) {
            if (seenSSIDs[j] == ssid) {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate && ssid.length() > 0) {
            seenSSIDs[seenCount++] = ssid;
            wifiOptions += "<option value='" + ssid + "'>" + ssid + "</option>";
        }
    }
    return wifiOptions;
}

void sendWiFiForm(WiFiClient &client) {
    String wifiOptions = getWiFiList();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<html><head><meta charset='utf-8'><title>WiFi Setup</title></head><body>");
    client.println("<h1>WiFi Configuration</h1>");
    client.println("<form action='/config' method='GET'>");
    client.println("Select WiFi Network: <select name='ssid'>" + wifiOptions + "</select><br>");
    client.println("WiFi Password: <input type='text' name='password'><br>");
    client.println("<input type='submit' value='Connect WiFi'>");
    client.println("</form>");
    client.println("</body></html>");
}

void startApplicationTasks() {
  client.setServer(mqtt_server, mqtt_port);
  if (xTaskCreate(mqttReconnect, "mqttReconnect", 512, NULL, 1, &mqttRconnectTaskHandle) != pdPASS) {
    Serial.println("Failed to create mqttReconnect Task!");
  } else 
  {
    Serial.println("mqttReconnect Task created successfully.");
  }

  if (xTaskCreate(selectStatus, "selectStatus", 256, NULL, 1, &selectStatusHandle) != pdPASS) {
    Serial.println("Failed to create selectStatus Task!");
  } else 
  {
    Serial.println("selectStatust Task created successfully.");
  }
}

void mqttsendmessage() {
  if (!client.connected()) {
    Serial.println("MQTT client not connected. Message not sent.");
    return;
  }

  std::vector<int> activeLEDsNow = getCurrentActiveLEDs();
  if (activeLEDsNow.empty()) {
    Serial.println("No active LEDs to publish.");
    return;
  }

  int ledindex = -1;
  int activePin = activeLEDsNow[0]; 
  const char* topic = subscribeTopic2;
  for (size_t i = 0; i < kidLEDPins1.size(); ++i) {
    if (kidLEDPins1[i] == activePin) {
      ledindex = i + 1;
    }
  }
  if (ledindex == -1) {
    Serial.println("Active LED not recognized for current mode.");
    return;
  }
  String message = String(ledindex);
  for(int i = 0; i < 10; i++){
    if (client.publish(topic, message.c_str())) {
      Serial.print("Published to ");
      Serial.print(topic);
      Serial.print(": ");
      Serial.println(message);
    } else {
      Serial.println("Publish failed.");
    }
    vTaskDelay(pdMS_TO_TICKS(200)); 
  }
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
        Serial.println("Processing older LED status...");
         int ledIndex = receivedMessage.toInt();
        if (ledIndex >= 1 && ledIndex <= 3) {
            for (int pin : olderLEDPins) {
                digitalWrite(pin, LOW);
            }
            int ledPin = olderLEDPins[ledIndex - 1]; 
            Serial.print("Turning ON kid LED: ");
            Serial.println(ledPin);
            digitalWrite(ledPin, HIGH);
        } else {
            Serial.println("Invalid LED index received.");
        }
    } 
}
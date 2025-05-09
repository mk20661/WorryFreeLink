#include <WiFi.h>
#include <WebServer.h>
#include "arduino_secrets.h"
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

#define restButton 27
#define LED_strip 26
#define older_led 25

Adafruit_NeoPixel kid_strip(8, 26, NEO_GRB + NEO_KHZ800);

WebServer server(80);

const char *apSSID = "WiFi_Config";
const char *apPassword = "";
String wifiOptions = "";

char staSSID[32] = "";    
char staPassword[32] = "";

const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = SECRET_MQTTSERVER;
const int mqtt_port       = SECRET_MQTT_HOST;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

const char* subscribeTopic1 = "oldLED/currentStatus";
const char* subscribeTopic2 = "kidLED1/currentStatus";
const char* subscribeTopic3 = "kidLED2/currentStatus";
const char* subscribeTopic4 = "kidLED3/currentStatus";

void handleWiFiConfig();
void sendWiFiForm();

TaskHandle_t wifiTaskHandle;
TaskHandle_t mqttRconnectTaskHandle;
TaskHandle_t buttonTaskTaskHandle;


bool ledState = false; 
bool lastButtonState = HIGH;  
bool currentButtonState;
unsigned long lastDebounceTime = 0;  
const unsigned long debounceDelay = 50; 

std::vector<int> activeLEDs;
std::vector<int> getCurrentActiveLEDs() {
      return activeLEDs;
  }

void setup() {
    Serial.begin(115200);
    kid_strip.begin();           
    kid_strip.setBrightness(30);
    kid_strip.fill(kid_strip.Color(0, 0, 0));
    kid_strip.show();

    pinMode(older_led, OUTPUT);
    pinMode(restButton, INPUT_PULLUP);

    WiFi.softAP(apSSID, apPassword);
    Serial.println("AP mode started, SSID: " + String(apSSID));
    Serial.println("Connect to this WiFi and go to 192.168.4.1 in your browser.");

    server.on("/", sendWiFiForm);
    server.on("/config", handleWiFiConfig);
    server.begin();

    xTaskCreate(WiFiTask, "WiFiTask", 4096, NULL, 1, &wifiTaskHandle);
    client.setCallback(callback);
}

void loop() {
}

void WiFiTask(void *parameter) {
    while (true) {
        int numNetworks = WiFi.scanNetworks();
        String tempOptions = "";
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
                tempOptions += "<option value='" + ssid + "'>" + ssid + "</option>";
            }
        }
        wifiOptions = tempOptions;

        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(10000));  
    }
}

void mqttReconnect(void *parameter) {
    Serial.println("MQTT Task Started");

    while (true) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected, reconnecting...");
            WiFi.disconnect();
            WiFi.begin(staSSID, staPassword);

            int retryCount = 0;
            while (WiFi.status() != WL_CONNECTED && retryCount < 10) {
                vTaskDelay(pdMS_TO_TICKS(500));
                Serial.print(".");
                retryCount++;
            }

            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("WiFi reconnect failed, restarting ESP32...");
                ESP.restart(); 
            }
        }

        if (!client.connected()) {
            Serial.println("Connecting to MQTT...");
            if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
                Serial.println("MQTT connected!");
                client.subscribe("your/topic");
            } else {
                Serial.print("MQTT connection failed, rc=");
                Serial.println(client.state());
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void sendWiFiForm() {
    String html = "<html><head><meta charset='utf-8'><title>WiFi Setup</title></head><body>";
    html += "<h1>WiFi Configuration</h1>";
    html += "<form action='/config' method='GET'>";
    html += "Select WiFi Network: <select name='ssid'>" + wifiOptions + "</select><br>";
    html += "WiFi Password: <input type='password' name='password'><br>";
    html += "<input type='submit' value='Connect WiFi'>";
    html += "</form></body></html>";
    server.send(200, "text/html", html);
}

void handleWiFiConfig() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    ssid.toCharArray(staSSID, sizeof(staSSID));
    password.toCharArray(staPassword, sizeof(staPassword));

    Serial.println("Selected WiFi SSID: " + ssid);
    Serial.println("Entered WiFi Password: " + password);

    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());

    int timeout = 15;  
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        Serial.print(".");
        timeout--;
    }

    String response = "<html><head><meta charset='utf-8'><title>WiFi Setup</title></head><body>";
    if (WiFi.status() == WL_CONNECTED) {
        response += "<h1> WiFi Connected!</h1>";
        response += "<p>Connected to: " + ssid + "</p>";
        response += "<p> IP Address: " + WiFi.localIP().toString() + "</p>";
        Serial.println("\n WiFi Connected!");
        Serial.print(" IP Address: ");
        Serial.println(WiFi.localIP());
        startApplicationTasks();
        vTaskDelete(wifiTaskHandle);
        return;
    } else {
        response += "<h1> WiFi Connection Failed!</h1>";
        response += "<p>Incorrect password or connection issue.</p>";
        response += "<p><a href='/'>Try Again</a></p>";
        Serial.println("\n WiFi Connection Failed! Incorrect password?");
    }
    response += "</body></html>";
    server.send(200, "text/html", response);
}

void mqttsendmessage(int message) {
  if (!client.connected()) {
    Serial.println("MQTT client not connected. Message not sent.");
    return;
  }

  std::vector<int> activeLEDsNow = getCurrentActiveLEDs();
  if (activeLEDsNow.empty()) {
    Serial.println("No active LEDs to publish.");
    return;
  }
  for(int i = 0; i < 10; i++){
    if (client.publish(subscribeTopic1, String(message).c_str())) {
      Serial.print("Published to ");
      Serial.print(subscribeTopic1);
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

    if (String(topic) == subscribeTopic2) {
        Serial.println("Processing Kid LED1 status...");
        int tmpMessage = receivedMessage.toInt();
        if (tmpMessage == 1) {
            kid_strip.setPixelColor(0, kid_strip.Color(0, 255, 0));
            kid_strip.show(); 
            Serial.print("Turning ON kid1 LED");
        } else if (tmpMessage == 0){
            kid_strip.setPixelColor(0, kid_strip.Color(0, 0, 0));
            kid_strip.show(); 
            Serial.println("Turning OFF kid1 LED");
        } else{
            Serial.println('invalid index');
        }
    } else if (String(topic) == subscribeTopic3) {
        Serial.println("Processing Kid LED2 status...");
        int ledIndex = receivedMessage.toInt();
       int tmpMessage = receivedMessage.toInt();
        if (tmpMessage == 1) {
            kid_strip.setPixelColor(2, kid_strip.Color(0, 255, 0));
            kid_strip.show(); 
            Serial.print("Turning ON kid2 LED");
        } else if (tmpMessage == 0){
            kid_strip.setPixelColor(2, kid_strip.Color(0, 0, 0));
            kid_strip.show(); 
            Serial.println("Turning OFF kid2 LED");
        } else{
            Serial.println('invalid index');
        }
    } else if (String(topic) == subscribeTopic4) {
        Serial.println("Processing Kid LED3 status...");
        int ledIndex = receivedMessage.toInt();
        int tmpMessage = receivedMessage.toInt();
        if (tmpMessage == 1) {
            kid_strip.setPixelColor(4, kid_strip.Color(0, 255, 0));
            kid_strip.show(); 
            Serial.print("Turning ON kid3 LED");
        } else if (tmpMessage == 0){
            kid_strip.setPixelColor(4, kid_strip.Color(0, 0, 0));
            kid_strip.show(); 
            Serial.println("Turning OFF kid3 LED");
        } else{
            Serial.println('invalid index');
        }
    }  
}

void buttonTask(void *parameter){
  while(true){
   currentButtonState = digitalRead(restButton);

    if (currentButtonState != lastButtonState) {
        lastDebounceTime = millis(); 
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (currentButtonState == LOW && lastButtonState == HIGH) {
            ledState = !ledState; 
            digitalWrite(older_led, ledState); 
        }
    }

    if(ledState){
      mqttsendmessage(1);
    }else{
      mqttsendmessage(0);
    }

    lastButtonState = currentButtonState;
  }
}

void startApplicationTasks() {
  client.setServer(mqtt_server, mqtt_port);
  client.subscribe(subscribeTopic1);
  client.subscribe(subscribeTopic2);
  client.subscribe(subscribeTopic3);
  client.subscribe(subscribeTopic4);
  if (xTaskCreate(mqttReconnect, "mqttReconnect", 2048, NULL, 1, &mqttRconnectTaskHandle) != pdPASS) {
    Serial.println("Failed to create mqttReconnect Task!");
  } else 
  {
    Serial.println("mqttReconnect Task created successfully.");
  }
  xTaskCreate(buttonTask, "button",1024, NULL, 1, &buttonTaskTaskHandle);
}
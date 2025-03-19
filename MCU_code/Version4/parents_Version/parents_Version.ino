#include <WiFi.h>
#include <WebServer.h>
#include "arduino_secrets.h"
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <DNSServer.h>
#include <vector>

#define restButton 27
#define LED_strip 26
#define older_led 25

Adafruit_NeoPixel elder_strip(8, 25, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel kid_strip(8, 26, NEO_GRB + NEO_KHZ800);

Preferences preferences;
WebServer server(80);
DNSServer dnsServer;

const char *apSSID = "Wifi_config_parents";
const char *apPassword = "";
const byte DNS_PORT = 53;

const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = SECRET_MQTTSERVER;
const int mqtt_port       = SECRET_MQTT_HOST;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

const char* subscribeTopic1 = "student/group1/oldLED/currentStatus";
const char* subscribeTopic2 = "student/group1/kidLED1/currentStatus";
const char* subscribeTopic3 = "student/group1/kidLED2/currentStatus";
const char* subscribeTopic4 = "student/group1/kidLED3/currentStatus";

bool ledState = false; 
bool lastButtonState = HIGH;  
bool currentButtonState;
unsigned long lastDebounceTime = 0;  
const unsigned long debounceDelay = 50; 
bool wifi_Connect =false;
unsigned long lastPressTime = 0;
const unsigned long timeoutPeriod = 7200000;

void handleRoot() {
    String html = "<html><body><h2>Wi-Fi Configuration</h2>";
    html += "<form action='/save' method='POST'>";
    html += "SSID: <select name='ssid'>";
    
    int numNetworks = WiFi.scanNetworks();
    std::vector<String> uniqueSSIDs;
    
    for (int i = 0; i < numNetworks; i++) {
        String ssid = WiFi.SSID(i);
        if (std::find(uniqueSSIDs.begin(), uniqueSSIDs.end(), ssid) == uniqueSSIDs.end()) {
            uniqueSSIDs.push_back(ssid);
            html += "<option value='" + ssid + "'>" + ssid + "</option>";
        }
    }
    
    html += "</select><br>";
    html += "Password: <input type='password' name='password'><br>";
    html += "<input type='submit' value='Save'>";
    html += "</form></body></html>";
    server.send(200, "text/html", html);
}

void handleSave() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    if (ssid.length() > 0 && password.length() > 0) {
        preferences.putString("wifi_ssid", ssid);
        preferences.putString("wifi_pass", password);
        server.send(200, "text/html", "<h3>Wi-Fi information saved, rebooting...</h3>");
        delay(2000);
        ESP.restart();
    } else {
        server.send(200, "text/html", "<h3>Please fill in all fields!</h3>");
    }
}

void handleRedirect() {
    server.sendHeader("Location", String("http://192.168.4.1"), true);
    server.send(302, "text/plain", "");
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(apSSID, apPassword);
    pinMode(restButton, INPUT_PULLUP);
    pinMode(older_led, OUTPUT);
    kid_strip.begin();           
    kid_strip.setBrightness(255);
    kid_strip.fill(kid_strip.Color(0, 0, 0));
    kid_strip.show();
    elder_strip.setBrightness(255);
    elder_strip.fill(kid_strip.Color(0, 0, 0));
    elder_strip.show();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    
    preferences.begin("wifi_config", false);
    
    String ssid = preferences.getString("wifi_ssid", "");
    String password = preferences.getString("wifi_pass", "");
    
    if (ssid.length() > 0) {
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.println("Connecting to Wi-Fi...");
        int attempt = 0;
        while (WiFi.status() != WL_CONNECTED && attempt < 20) {
            delay(500);
            Serial.print(".");
            attempt++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWi-Fi connected!");
            Serial.println(WiFi.localIP());
            wifi_Connect = true;
          for (int i = 0; i < 3; i++) {
            elder_strip.setPixelColor(0, elder_strip.Color(0, 255, 0));
            elder_strip.show();
            kid_strip.setPixelColor(0, kid_strip.Color(0, 255, 0));
            kid_strip.setPixelColor(2, kid_strip.Color(0, 255, 0));
            kid_strip.setPixelColor(4, kid_strip.Color(0, 255, 0));
            kid_strip.show();
            
            delay(500);

            elder_strip.setPixelColor(0, elder_strip.Color(0, 0, 0));
            elder_strip.show();
            kid_strip.setPixelColor(0, kid_strip.Color(0, 0, 0));
            kid_strip.setPixelColor(2, kid_strip.Color(0, 0, 0));
            kid_strip.setPixelColor(4, kid_strip.Color(0, 0, 0));
            kid_strip.show();
            
            delay(500);
          }
            elder_strip.setPixelColor(0, elder_strip.Color(255, 0, 0));
            elder_strip.show();
            kid_strip.setPixelColor(0, kid_strip.Color(255, 0, 0));
            kid_strip.setPixelColor(2, kid_strip.Color(255, 0, 0));
            kid_strip.setPixelColor(4, kid_strip.Color(255, 0, 0));
            kid_strip.show();
            return;
        }
    }
    
    Serial.println("\nWi-Fi connection failed, starting AP mode...");
    WiFi.softAP(apSSID, apPassword);
    Serial.println("AP started, IP Address:");
    Serial.println(WiFi.softAPIP());
    
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    WiFi.scanNetworks();
    
    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.onNotFound(handleRedirect);
    server.begin();

}

void loop() {
    dnsServer.processNextRequest();
    server.handleClient();

    if (!wifi_Connect) {  
        Serial.println("Waiting for WiFi connection...");
        delay(1000);
        return; 
    }
    if (!client.connected()) {
        reconnectMQTT();
    }
    handleButtonPress();

    if (millis() - lastPressTime > timeoutPeriod && ledState == 1) {
        elder_strip.setPixelColor(0, elder_strip.Color(255, 0, 0));
        elder_strip.show();
        ledState = !ledState;
        mqttsendmessage(0);
    }
    client.loop();    
}

void reconnectMQTT() {
   if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, reconnecting...");
        
        preferences.begin("wifi", true);
        String ssid = preferences.getString("ssid", "");
        String password = preferences.getString("password", "");
        preferences.end();
        
        WiFi.disconnect();
        WiFi.begin(ssid.c_str(), password.c_str());

        int retryCount = 0;
        while (WiFi.status() != WL_CONNECTED && retryCount < 10) {
            vTaskDelay(pdMS_TO_TICKS(500));
            Serial.print(".");
            retryCount++;
        }

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi reconnect failed, entering AP mode...");
            WiFi.softAP(apSSID, apPassword);
        }
    }
    Serial.println("Connecting to MQTT...");
    String clientID = "ESP32Client-" + String(random(1000, 9999)); 
    if (client.connect(clientID.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("MQTT connected!");
        client.subscribe(subscribeTopic2);
        client.subscribe(subscribeTopic3);
        client.subscribe(subscribeTopic4);
    } else {
        Serial.println("MQTT connection failed, rc=");
        Serial.println(client.state());
    }
}

void handleButtonPress() {
    bool reading = digitalRead(restButton); 

    if ((millis() - lastDebounceTime) > debounceDelay) { 
        if (reading == HIGH && lastButtonState == LOW) {  
            ledState = !ledState; 
            if (ledState == 1) {
              elder_strip.setPixelColor(0, elder_strip.Color(0, 255, 0));
              lastPressTime = millis();
              mqttsendmessage(1);
            } else {
              elder_strip.setPixelColor(0, elder_strip.Color(255, 0, 0));
              mqttsendmessage(0);
            }
            elder_strip.show(); 
            lastDebounceTime = millis();  
            Serial.print("Button Pressed! LED State: ");
            Serial.println(ledState); 
        }
    }

    lastButtonState = reading;
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
            kid_strip.setPixelColor(0, kid_strip.Color(255, 0, 0));
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
            kid_strip.setPixelColor(2, kid_strip.Color(255, 0, 0));
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
            kid_strip.setPixelColor(4, kid_strip.Color(255, 0, 0));
            kid_strip.show(); 
            Serial.println("Turning OFF kid3 LED");
        } else{
            Serial.println('invalid index');
        }
    }  
}

void mqttsendmessage(int message) {
  if (!client.connected()) {
    Serial.println("MQTT client not connected. Message not sent.");
    return;
  }
    if (client.publish(subscribeTopic1, String(message).c_str())) {
      Serial.print("Published to ");
      Serial.print(subscribeTopic1);
      Serial.print(": ");
      Serial.println(message);
    } else {
      Serial.println("Publish failed.");
    }
}
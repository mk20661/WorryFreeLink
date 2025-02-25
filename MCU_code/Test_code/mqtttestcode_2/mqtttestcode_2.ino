#include <WiFi.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = SECRET_MQTTPASS;
const int mqtt_port       = SECRET_MQTT_HOST;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

const char* publishTopic = "kidLED/currentStatus";
const char* subscribeTopic = "oldLED/currentStatus";

void setup_wifi() {
    delay(10);
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
    delay(1000);
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        if (client.connect("mqttClient", mqtt_username, mqtt_password)) {
            Serial.println("Connected");
            client.subscribe(subscribeTopic);
            Serial.print("Subscribed to: ");
            Serial.println(subscribeTopic);
        } else {
            Serial.print("Failed, status: ");
            Serial.print(client.state());
            Serial.println(" Retrying in 2 seconds...");
            delay(2000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    static unsigned long lastMsg = 0;
    if (millis() - lastMsg > 500) {
        lastMsg = millis();
        String message = "1";
        client.publish(publishTopic, message.c_str());
        Serial.println("Message sent: " + message);
    }

}
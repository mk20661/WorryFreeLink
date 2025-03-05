#include <WiFiS3.h>
#include <Arduino.h>
#include <vector>
#include <Arduino_FreeRTOS.h>
#include <SoftwareSerial.h>

WiFiServer server(80);
const char *apSSID = "WiFi_Config";
const char *apPassword = "";

TaskHandle_t wifiTaskHandle;
TaskHandle_t ledTaskHandle;
TaskHandle_t buttonTaskHandle;
TaskHandle_t selectStatusHandle;
TaskHandle_t selectButtonHandle;

//aduio chip
SoftwareSerial playerSerial(2,3);


#define Button A3
#define RestButton A2

#define OlderSelected 0
#define KidSelected_1 8
#define KidSelected_2 12

#define LED_Older_1 1
#define LED_Older_2 4
#define LED_Older_3 5
#define LED_Older_4 6
#define LED_Older_5 7

#define LED_Kid_1 9
#define LED_Kid_2 10
#define LED_Kid_3 11

#define LED_Kid_4 13
#define LED_Kid_5 ((int)A5)
#define LED_Kid_6 ((int)A4)

#define eating "/eating.mp3"
#define exercising "/exercising.mp3"
#define sleeping "/sleeping.mp3"
#define social "/social.mp3"
#define others "/others.mp3"

std::vector<int> selectPins = {OlderSelected,KidSelected_1,KidSelected_2};
std::vector<int> olderLEDPins = {LED_Older_1, LED_Older_2, LED_Older_3, LED_Older_4, LED_Older_5};
std::vector<int> kidLEDPins1 = {LED_Kid_1,LED_Kid_2,LED_Kid_3};
std::vector<int> kidLEDPins2 = {LED_Kid_4,LED_Kid_5,LED_Kid_6};
std::vector<String> attentionAudio = {eating,sleeping,exercising,social,others};

volatile int currentMode = 0; 
volatile bool taskStopped = false;
volatile bool buttonPressed = false;
volatile bool selectButtonPressed = false;
volatile int currentOlederLEDIndex_case1 = 0;
volatile int currentKidLEDIndex_case1 = 0;
volatile int currentKidLEDIndex_case2 = 0; 

void setup() {
   Serial.begin(115200);
   playerSerial.begin(9600);
   delay(1000);
   sendCommand("AT+PLAYMODE=3");
   sendCommand("AT+LED=ON");
   sendCommand("AT+VOL=13");


   for (int pin : {OlderSelected,KidSelected_1,KidSelected_2,
                  LED_Older_1, LED_Older_2, LED_Older_3, LED_Older_4, LED_Older_5,
                  LED_Kid_1, LED_Kid_2, LED_Kid_3, LED_Kid_4, LED_Kid_5, LED_Kid_6}) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
   }
   pinMode(Button, INPUT_PULLUP);  
   if (xTaskCreate(WiFiTask, "WiFi Task", 1024, NULL, 1, &wifiTaskHandle) != pdPASS) {
        Serial.println("Failed to create WiFiTask!");
    } else {
        Serial.println("WiFiTask created successfully.");
    }
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
    client.println("<meta http-equiv='refresh' content='5;url=/'>"); // 5秒后自动刷新
    client.println("</head><body>");
    client.println("<h1>WiFi Connecting......</h1>");
    client.println("<p>Connecting to: " + ssid + "</p>");
    client.println("<p>Please wait...</p>");
    client.println("</body></html>");
    client.flush();

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
            startApplicationTasks();
            vTaskDelete(wifiTaskHandle);
            return;
        }
        Serial.println("Trying to connect...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        retry++;
    }

    Serial.println("WiFi Connection Failed!");
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

void ledTask(void *parameter) {
  while(1){
    if (taskStopped) { 
      vTaskDelete(NULL); 
    }

    currentMode = 1;
    for (int i = 0; i < 5; i++) {
        if (taskStopped) { vTaskDelete(NULL); }
        for (int pin : {OlderSelected,LED_Older_1, LED_Older_2, LED_Older_3, LED_Older_4, LED_Older_5}) {
          digitalWrite(pin, HIGH);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        for (int pin : {OlderSelected,LED_Older_1, LED_Older_2, LED_Older_3, LED_Older_4, LED_Older_5}) {
          digitalWrite(pin, LOW);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    currentMode = 2;
    for (int i = 0; i < 5; i++) {
        if (taskStopped) { vTaskDelete(NULL); }
        for (int pin : {KidSelected_1,LED_Kid_1, LED_Kid_2, LED_Kid_3}) {
            digitalWrite(pin, HIGH);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        for (int pin : {KidSelected_1,LED_Kid_1, LED_Kid_2, LED_Kid_3}) {
            digitalWrite(pin, LOW);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    currentMode = 3;
    for (int i = 0; i < 5; i++) {
        if (taskStopped) { vTaskDelete(NULL); }
        for (int pin : {KidSelected_2,LED_Kid_4, LED_Kid_5, LED_Kid_6}) {
          digitalWrite(pin, HIGH);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        for (int pin : {KidSelected_2,LED_Kid_4, LED_Kid_5, LED_Kid_6}) {
          digitalWrite(pin, LOW);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
  }  
}

void checkButton1(void *parameter) {
    while (1) {
        int buttonState = digitalRead(Button);

        if (!buttonPressed && buttonState == HIGH) { 
            buttonPressed = true;
            taskStopped = true; 

            for (int pin : {OlderSelected,KidSelected_1,KidSelected_2,
                            LED_Older_1, LED_Older_2, LED_Older_3, LED_Older_4, LED_Older_5,
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
        digitalWrite(OlderSelected,HIGH);
          for (int i = 0; i < 5; i++) {
              if (selectButtonPressed) break;
              digitalWrite(olderLEDPins[currentOlederLEDIndex_case1], HIGH);
              vTaskDelay(pdMS_TO_TICKS(500));
              digitalWrite(olderLEDPins[currentOlederLEDIndex_case1], LOW);
              vTaskDelay(pdMS_TO_TICKS(500));
              if(i == 4) {
                playFile(attentionAudio[currentOlederLEDIndex_case1]);
              }
          }

          if (!selectButtonPressed) {
              digitalWrite(olderLEDPins[currentOlederLEDIndex_case1], HIGH);
          }

          while (!selectButtonPressed) {
              vTaskDelay(pdMS_TO_TICKS(100));
          }

          digitalWrite(olderLEDPins[currentOlederLEDIndex_case1], LOW);
          selectButtonPressed = false;
          currentOlederLEDIndex_case1 = (currentOlederLEDIndex_case1 + 1) % 5;
          break;

        case 2:
        digitalWrite(KidSelected_1,HIGH);
         for (int i = 0; i < 5; i++) {
            if (selectButtonPressed) break;
              digitalWrite(kidLEDPins1[currentKidLEDIndex_case1], HIGH);
              vTaskDelay(pdMS_TO_TICKS(500));
              digitalWrite(kidLEDPins1[currentKidLEDIndex_case1], LOW);
              vTaskDelay(pdMS_TO_TICKS(500));
            }

            if (!selectButtonPressed) {
              digitalWrite(kidLEDPins1[currentKidLEDIndex_case1], HIGH);
            }

            while (!selectButtonPressed) {
              vTaskDelay(pdMS_TO_TICKS(100));
            }

            digitalWrite(kidLEDPins1[currentKidLEDIndex_case1], LOW);
            selectButtonPressed = false;
            currentKidLEDIndex_case1 = (currentKidLEDIndex_case1 + 1) % 3;
            break;

        case 3:
        digitalWrite(KidSelected_2,HIGH);
         for (int i = 0; i < 5; i++) {
            if (selectButtonPressed) break;
              digitalWrite(kidLEDPins2[currentKidLEDIndex_case2], HIGH);
              vTaskDelay(pdMS_TO_TICKS(500));
              digitalWrite(kidLEDPins2[currentKidLEDIndex_case2], LOW);
              vTaskDelay(pdMS_TO_TICKS(500));
            }

            if (!selectButtonPressed) {
              digitalWrite(kidLEDPins2[currentKidLEDIndex_case2], HIGH);
            }

            while (!selectButtonPressed) {
              vTaskDelay(pdMS_TO_TICKS(100));
            }

            digitalWrite(kidLEDPins2[currentKidLEDIndex_case2], LOW);
            selectButtonPressed = false;
            currentKidLEDIndex_case2 = 3 + ((currentKidLEDIndex_case2 - 3 + 1) % 3);
            break;
    }
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
    pinMode(RestButton, INPUT_PULLUP);
    while (1) {
        if (digitalRead(RestButton) == HIGH) { 
            vTaskDelay(pdMS_TO_TICKS(1000)); 
            NVIC_SystemReset();
        }
        vTaskDelay(pdMS_TO_TICKS(100));  
    }
}

void playFile(String fileName) {
  sendCommand("AT+PLAYFILE=" + fileName);
}

void sendCommand(String command) {
  playerSerial.print(command + "\r\n");
  Serial.println("Sent: " + command);
}

void startApplicationTasks() {
    xTaskCreate(ledTask, "LED Task", 256, NULL, 1, &ledTaskHandle);
    xTaskCreate(checkButton1, "Check Button", 256, NULL, 1, &buttonTaskHandle);
    xTaskCreate(restButtonTask, "restbutton", 256, NULL, 1, NULL);
}
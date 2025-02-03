#include <Arduino.h>
#include <vector>

#define LED_Older_1 1   
#define LED_Older_2 25  
#define LED_Kid_1 26    
#define Button 27       

TaskHandle_t ledTaskHandle = NULL;
TaskHandle_t buttonTaskHandle = NULL;
TaskHandle_t selectStatusHandle = NULL;
TaskHandle_t selectButtonHandle = NULL;

volatile bool buttonPressed = false; 
volatile int currentMode = 0; 
volatile bool taskStopped = false;
volatile int olderStatus = 6;
volatile bool selectButtonPressed = false;
volatile int currentLEDIndex = 0;

std::vector<int> olderLEDPins = {LED_Older_1, LED_Older_2};
std::vector<int> kidLEDPins = {LED_Kid_1};

void checkButton1(void *parameter) {
    while (1) {
        int buttonState = digitalRead(Button);

        if (!buttonPressed && buttonState == LOW) { 
            buttonPressed = true;
            taskStopped = true; 

            digitalWrite(LED_Older_1, LOW);
            digitalWrite(LED_Older_2, LOW);
            digitalWrite(LED_Kid_1, LOW);

            if (ledTaskHandle != NULL) {
                vTaskDelete(ledTaskHandle);
                ledTaskHandle = NULL;
            }

            if (selectStatusHandle == NULL) {
                xTaskCreate(selectStatus, "selectStatus", 1024, NULL, 1, &selectStatusHandle);
            }

            vTaskDelay(pdMS_TO_TICKS(500));
        }

        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void selectStatus(void *parameter) {
  vTaskDelay(pdMS_TO_TICKS(1000));
  xTaskCreate(selectButton, "selectButtonCheck", 1024, NULL, 1, &selectButtonHandle);
  while (1) {
    switch (currentMode) {
        case 1:
          for (int i = 0; i < 5; i++) {
              if (selectButtonPressed) break;
              digitalWrite(olderLEDPins[currentLEDIndex], HIGH);
              vTaskDelay(pdMS_TO_TICKS(500));
              digitalWrite(olderLEDPins[currentLEDIndex], LOW);
              vTaskDelay(pdMS_TO_TICKS(500));
          }

          if (!selectButtonPressed) {
              digitalWrite(olderLEDPins[currentLEDIndex], HIGH);
          }

          while (!selectButtonPressed) {
              vTaskDelay(pdMS_TO_TICKS(100));
          }

          digitalWrite(olderLEDPins[currentLEDIndex], LOW);
          selectButtonPressed = false;
          currentLEDIndex = (currentLEDIndex + 1) % olderLEDPins.size();
          break;

        case 2:
         for (int i = 0; i < 5; i++) {
            if (selectButtonPressed) break;
              digitalWrite(kidLEDPins[0], HIGH);
              vTaskDelay(pdMS_TO_TICKS(500));
              digitalWrite(kidLEDPins[0], LOW);
              vTaskDelay(pdMS_TO_TICKS(500));
            }

            if (!selectButtonPressed) {
              digitalWrite(kidLEDPins[0], HIGH);
            }

            while (!selectButtonPressed) {
              vTaskDelay(pdMS_TO_TICKS(100));
            }

            digitalWrite(kidLEDPins[0], LOW);
            selectButtonPressed = false;
            currentLEDIndex = (currentLEDIndex + 1) % kidLEDPins.size();
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
    while (1) {
        if (taskStopped) { 
            vTaskDelete(NULL); 
        }

        currentMode = 1;
        for (int i = 0; i < 5; i++) {
            if (taskStopped) { vTaskDelete(NULL); }
            digitalWrite(LED_Older_1, HIGH);
            digitalWrite(LED_Older_2, HIGH);
            vTaskDelay(pdMS_TO_TICKS(500));
            digitalWrite(LED_Older_1, LOW);
            digitalWrite(LED_Older_2, LOW);
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        currentMode = 2;
        for (int i = 0; i < 5; i++) {
            if (taskStopped) { vTaskDelete(NULL); }
            digitalWrite(LED_Kid_1, HIGH);
            vTaskDelay(pdMS_TO_TICKS(500));
            digitalWrite(LED_Kid_1, LOW);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void setup() {
    pinMode(LED_Older_1, OUTPUT);
    pinMode(LED_Older_2, OUTPUT);
    pinMode(LED_Kid_1, OUTPUT);
    pinMode(Button, INPUT_PULLUP);  

    xTaskCreate(ledTask, "LED Task", 1024, NULL, 1, &ledTaskHandle);
    xTaskCreate(checkButton1, "Check Button", 1024, NULL, 1, &buttonTaskHandle);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(100)); 
}
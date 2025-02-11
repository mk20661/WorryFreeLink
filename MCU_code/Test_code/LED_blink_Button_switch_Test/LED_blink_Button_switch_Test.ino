#include <Arduino.h>
#include <vector>

#include <Arduino_FreeRTOS.h>

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

volatile bool buttonPressed = false; 
volatile int currentMode = 0; 
volatile bool taskStopped = false;
volatile bool selectButtonPressed = false;
volatile int currentOlederLEDIndex_case1 = 0;
volatile int currentOlederLEDIndex_case2 = 5;
volatile int currentKidLEDIndex_case1 = 0;
volatile int currentKidLEDIndex_case2 = 3;

std::vector<int> olderLEDPins = {LED_Older_1, LED_Older_2, LED_Older_3, LED_Older_4, LED_Older_5};
std::vector<int> kidLEDPins = {LED_Kid_1,LED_Kid_2,LED_Kid_3,LED_Kid_4,LED_Kid_5,LED_Kid_6};

void checkButton1(void *parameter) {
    while (1) {
        int buttonState = digitalRead(Button);

        if (!buttonPressed && buttonState == LOW) { 
            buttonPressed = true;
            taskStopped = true; 

            digitalWrite(LED_Older_1, LOW);
            digitalWrite(LED_Older_2, LOW);
            digitalWrite(LED_Older_3, LOW);
            digitalWrite(LED_Older_4, LOW);
            digitalWrite(LED_Older_5, LOW);

            digitalWrite(LED_Kid_1, LOW);
            digitalWrite(LED_Kid_2, LOW);
            digitalWrite(LED_Kid_3, LOW);
            digitalWrite(LED_Kid_4, LOW);
            digitalWrite(LED_Kid_5, LOW);
            digitalWrite(LED_Kid_6, LOW);

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
         for (int i = 0; i < 5; i++) {
            if (selectButtonPressed) break;
              digitalWrite(kidLEDPins[currentKidLEDIndex_case1], HIGH);
              vTaskDelay(pdMS_TO_TICKS(500));
              digitalWrite(kidLEDPins[currentKidLEDIndex_case1], LOW);
              vTaskDelay(pdMS_TO_TICKS(500));
            }

            if (!selectButtonPressed) {
              digitalWrite(kidLEDPins[currentKidLEDIndex_case1], HIGH);
            }

            while (!selectButtonPressed) {
              vTaskDelay(pdMS_TO_TICKS(100));
            }

            digitalWrite(kidLEDPins[currentKidLEDIndex_case1], LOW);
            selectButtonPressed = false;
            currentKidLEDIndex_case1 = (currentKidLEDIndex_case1 + 1) % 3;
            break;

        case 3:
         for (int i = 0; i < 5; i++) {
            if (selectButtonPressed) break;
              digitalWrite(kidLEDPins[currentKidLEDIndex_case2], HIGH);
              vTaskDelay(pdMS_TO_TICKS(500));
              digitalWrite(kidLEDPins[currentKidLEDIndex_case2], LOW);
              vTaskDelay(pdMS_TO_TICKS(500));
            }

            if (!selectButtonPressed) {
              digitalWrite(kidLEDPins[currentKidLEDIndex_case2], HIGH);
            }

            while (!selectButtonPressed) {
              vTaskDelay(pdMS_TO_TICKS(100));
            }

            digitalWrite(kidLEDPins[currentKidLEDIndex_case2], LOW);
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
        if (!selectButtonPressed && buttonState == LOW) { 
            selectButtonPressed = true;
        }
        vTaskDelay(pdMS_TO_TICKS(100));  
    }
}

void ledTask(void *parameter) {
  while(1){
    if (taskStopped) { 
      vTaskDelete(NULL); 
    }

    currentMode = 1;
    for (int i = 0; i < 5; i++) {
        if (taskStopped) { vTaskDelete(NULL); }
        digitalWrite(LED_Older_1, HIGH);
        digitalWrite(LED_Older_2, HIGH);
        digitalWrite(LED_Older_3, HIGH);
        digitalWrite(LED_Older_4, HIGH);
        digitalWrite(LED_Older_5, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(LED_Older_1, LOW);
        digitalWrite(LED_Older_2, LOW);
        digitalWrite(LED_Older_3, LOW);
        digitalWrite(LED_Older_4, LOW);
        digitalWrite(LED_Older_5, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    currentMode = 2;
    for (int i = 0; i < 5; i++) {
        if (taskStopped) { vTaskDelete(NULL); }
        digitalWrite(LED_Kid_1, HIGH);
        digitalWrite(LED_Kid_2, HIGH);
        digitalWrite(LED_Kid_3, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(LED_Kid_1, LOW);
        digitalWrite(LED_Kid_2, LOW);
        digitalWrite(LED_Kid_3, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    currentMode = 3;
    for (int i = 0; i < 5; i++) {
        if (taskStopped) { vTaskDelete(NULL); }
        digitalWrite(LED_Kid_4, HIGH);
        digitalWrite(LED_Kid_5, HIGH);
        digitalWrite(LED_Kid_6, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(LED_Kid_4, LOW);
        digitalWrite(LED_Kid_5, LOW);
        digitalWrite(LED_Kid_6, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
  }  
}

void setup() {
  pinMode(LED_Older_1, OUTPUT);
  pinMode(LED_Older_2, OUTPUT);
  pinMode(LED_Older_3, OUTPUT);
  pinMode(LED_Older_4, OUTPUT);
  pinMode(LED_Older_5, OUTPUT);

  pinMode(LED_Kid_1, OUTPUT);
  pinMode(LED_Kid_2, OUTPUT);
  pinMode(LED_Kid_3, OUTPUT);
  
  pinMode(LED_Kid_4, OUTPUT);
  pinMode(LED_Kid_5, OUTPUT);
  pinMode(LED_Kid_6, OUTPUT);
  
  pinMode(Button, INPUT_PULLUP);  

  xTaskCreate(ledTask, "LED Task", 256, NULL, 1, &ledTaskHandle);
  xTaskCreate(checkButton1, "Check Button", 256, NULL, 1, &buttonTaskHandle);
  vTaskStartScheduler();
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(100)); 
}
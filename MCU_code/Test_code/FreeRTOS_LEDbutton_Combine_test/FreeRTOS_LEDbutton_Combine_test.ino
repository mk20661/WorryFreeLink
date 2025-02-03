#include <Arduino.h>

#define LED_Older_1 1   // LED 1
#define LED_Older_2 25  // LED 2
#define LED_Kid_1 26    // LED 3
#define Button 27       // Button 

// Task handles
TaskHandle_t ledTaskHandle = NULL;
TaskHandle_t buttonTaskHandle = NULL;

volatile bool buttonPressed = false; 
volatile int currentMode = 0; 
volatile bool taskStopped = false;  

void checkButton(void *parameter) {
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

            vTaskDelay(pdMS_TO_TICKS(500));
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

    xTaskCreate(checkButton, "Check Button", 1024, NULL, 1, &buttonTaskHandle);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(100)); 
}
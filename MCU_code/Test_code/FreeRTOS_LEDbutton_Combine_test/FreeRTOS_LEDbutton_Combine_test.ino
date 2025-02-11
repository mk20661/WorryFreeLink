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

// Task handles
TaskHandle_t ledTaskHandle ;
TaskHandle_t buttonTaskHandle ;


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

  xTaskCreate(ledTask, "LED Task", 1024, NULL, 1, &ledTaskHandle);

  xTaskCreate(checkButton, "Check Button", 1024, NULL, 1, &buttonTaskHandle);
  vTaskStartScheduler();
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(100)); 
}
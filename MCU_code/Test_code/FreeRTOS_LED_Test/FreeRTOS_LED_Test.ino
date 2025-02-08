#include <Arduino_FreeRTOS.h>

#define LED_Older_1 6
#define LED_Older_2 7
#define LED_Older_3 8
#define LED_Older_4 9
#define LED_Older_5 10

#define LED_Kid_1 11
#define LED_Kid_2 12
#define LED_Kid_3 13

#define LED_Older_6 0
#define LED_Older_7 1
#define LED_Older_8 2
#define LED_Older_9 3
#define LED_Older_10 4

#define LED_Kid_4 A3
#define LED_Kid_5 A4
#define LED_Kid_6 A5

#define Button 5


TaskHandle_t blinkOlder_1LEDHandle;
TaskHandle_t blinkOlder_2LEDHandle;
TaskHandle_t blinkKid_1LEDHandle;
TaskHandle_t blinkKid_2LEDHandle;


void blinkOlder_1LED(void *parameter) {
    while (1) {
        for (int i = 0; i < 10; i++) { 
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
        xTaskCreate(blinkOlder_2LED, "Blink LED_older_2", 128, NULL, 1, &blinkOlder_2LEDHandle);
        vTaskDelete(NULL); 
    }
}

void blinkOlder_2LED(void *parameter) {
    while (1) {
        for (int i = 0; i < 10; i++) { 
            digitalWrite(LED_Older_6, HIGH);
            digitalWrite(LED_Older_7, HIGH);
            digitalWrite(LED_Older_8, HIGH);
            digitalWrite(LED_Older_9, HIGH);
            digitalWrite(LED_Older_10, HIGH);
            vTaskDelay(pdMS_TO_TICKS(500));
            digitalWrite(LED_Older_6, LOW);
            digitalWrite(LED_Older_7, LOW);
            digitalWrite(LED_Older_8, LOW);
            digitalWrite(LED_Older_9, LOW);
            digitalWrite(LED_Older_10, LOW);
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        xTaskCreate(blinkKid_1LED, "Blink LED_Kid_1", 128, NULL, 1, &blinkKid_1LEDHandle);
        vTaskDelete(NULL); 
    }
}

void blinkKid_1LED(void *parameter) {
    while (1) {
        for (int i = 0; i < 10; i++) {
            digitalWrite(LED_Kid_1, HIGH);
            digitalWrite(LED_Kid_2, HIGH);
            digitalWrite(LED_Kid_3, HIGH);
            vTaskDelay(pdMS_TO_TICKS(500));
            digitalWrite(LED_Kid_1, LOW);
            digitalWrite(LED_Kid_2, LOW);
            digitalWrite(LED_Kid_3, LOW);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        xTaskCreate(blinkKid_2LED, "Blink Two LEDs", 128, NULL, 1, &blinkKid_2LEDHandle);
        vTaskDelete(NULL);
    }
}

void blinkKid_2LED(void *parameter) {
    while (1) {
        for (int i = 0; i < 10; i++) {
            digitalWrite(LED_Kid_4, HIGH);
            digitalWrite(LED_Kid_5, HIGH);
            digitalWrite(LED_Kid_6, HIGH);
            vTaskDelay(pdMS_TO_TICKS(500));
            digitalWrite(LED_Kid_4, LOW);
            digitalWrite(LED_Kid_5, LOW);
            digitalWrite(LED_Kid_6, LOW);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        xTaskCreate(blinkOlder_1LED, "Blink Two LEDs", 128, NULL, 1, &blinkOlder_1LEDHandle);
        vTaskDelete(NULL);
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

  pinMode(LED_Older_6, OUTPUT);
  pinMode(LED_Older_7, OUTPUT);
  pinMode(LED_Older_8, OUTPUT);
  pinMode(LED_Older_9, OUTPUT);
  pinMode(LED_Older_10, OUTPUT);

  pinMode(LED_Kid_4, OUTPUT);
  pinMode(LED_Kid_5, OUTPUT);
  pinMode(LED_Kid_6, OUTPUT);

  Serial.begin(115200);

  xTaskCreate(blinkOlder_1LED, "Blink Two LEDs", 128, NULL, 1, &blinkOlder_1LEDHandle);
  vTaskStartScheduler();
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
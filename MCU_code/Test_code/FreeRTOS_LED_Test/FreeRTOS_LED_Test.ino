#include <Arduino.h>

#define LED_Older_1 1   // LED 1
#define LED_Older_2 25  // LED 2
#define LED_Kid_1 26    // LED 3

// 任务句柄
TaskHandle_t blinkTwoLEDsHandle;
TaskHandle_t blinkSingleLEDHandle;

// 任务1: 两个 LED (LED_Older_1, LED_Older_2) 同时闪烁 5 秒
void blinkTwoLEDs(void *parameter) {
    while (1) {
        for (int i = 0; i < 10; i++) { // 5s 闪烁（500ms * 10 次）
            digitalWrite(LED_Older_1, HIGH);
            digitalWrite(LED_Older_2, HIGH);
            vTaskDelay(pdMS_TO_TICKS(500));
            digitalWrite(LED_Older_1, LOW);
            digitalWrite(LED_Older_2, LOW);
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        // 任务结束前确保 LED 关闭
        digitalWrite(LED_Older_1, LOW);
        digitalWrite(LED_Older_2, LOW);

        // 切换到 LED_Kid_1 任务
        xTaskCreate(blinkSingleLED, "Blink LED_Kid_1", 1024, NULL, 1, &blinkSingleLEDHandle);
        vTaskDelete(NULL); // 删除当前任务
    }
}

// 任务2: LED_Kid_1 闪烁 5 秒
void blinkSingleLED(void *parameter) {
    while (1) {
        for (int i = 0; i < 10; i++) { // 5s 闪烁（500ms * 10 次）
            digitalWrite(LED_Kid_1, HIGH);
            vTaskDelay(pdMS_TO_TICKS(500));
            digitalWrite(LED_Kid_1, LOW);
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        // 任务结束前确保 LED 关闭
        digitalWrite(LED_Kid_1, LOW);

        // 切换回 LED_Older_1 和 LED_Older_2 任务
        xTaskCreate(blinkTwoLEDs, "Blink Two LEDs", 1024, NULL, 1, &blinkTwoLEDsHandle);
        vTaskDelete(NULL); // 删除当前任务
    }
}

void setup() {
    // 设置 LED 引脚为输出模式
    pinMode(LED_Older_1, OUTPUT);
    pinMode(LED_Older_2, OUTPUT);
    pinMode(LED_Kid_1, OUTPUT);

    Serial.begin(115200);

    // **开始第一个任务**
    xTaskCreate(blinkTwoLEDs, "Blink Two LEDs", 1024, NULL, 1, &blinkTwoLEDsHandle);
}

void loop() {
    // 任务由 FreeRTOS 负责管理，loop() 不执行任何操作
    vTaskDelay(pdMS_TO_TICKS(1000));
}
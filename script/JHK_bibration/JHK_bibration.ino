#include <Arduino_FreeRTOS.h>

const int sensorPin = 13;  // 디지털 핀으로 변경
bool triggered = false;

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);

  xTaskCreate(
    vibrationTask,
    "VibrationTask",
    128,
    NULL,
    1,
    NULL
  );
}

void loop() {
}

void vibrationTask(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    int sensorValue = digitalRead(sensorPin);
    Serial.print("센서 신호: ");
    Serial.println(sensorValue);

    if (sensorValue == LOW && !triggered) {  // 보통 D0는 충격 시 LOW
      Serial.println("충돌이 감지되었습니다!");
      triggered = true;
      vTaskDelay(1000 / portTICK_PERIOD_MS);  // 1초 대기
    }

    if (sensorValue == HIGH) {
      triggered = false;  // 신호 회복되면 다시 감지 가능
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);  // 빠른 루프 주기
  }
}

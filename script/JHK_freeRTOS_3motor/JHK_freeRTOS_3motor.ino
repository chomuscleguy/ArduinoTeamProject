#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <SoftwareSerial.h>

// 모터 핀
#define A_1A 6
#define A_1B 5

// 시리얼 통신 핀
#define BT_RX 2
#define BT_TX 3
SoftwareSerial mySerial(BT_RX, BT_TX);

// 모터 속도
int speed = 200;

// 모터 명령 정의
typedef enum {
  MOTOR_STOP,
  MOTOR_FORWARD,
  MOTOR_BACKWARD
} MotorCommand;

QueueHandle_t motorQueue;

// 모터 제어 Task
void MotorTask(void *pvParameters) {
  pinMode(A_1A, OUTPUT);
  pinMode(A_1B, OUTPUT);
  MotorCommand command;

  Serial.println("MotorTask 시작됨");

  while (1) {
    if (xQueueReceive(motorQueue, &command, portMAX_DELAY) == pdPASS) {

      // 먼저 모터를 완전히 꺼줌 (브레이크 방지)
      analogWrite(A_1A, 0);
      analogWrite(A_1B, 0);
      vTaskDelay(pdMS_TO_TICKS(5));  // 짧은 안정 대기

      // 명령 수행
      switch (command) {
        case MOTOR_FORWARD:
          analogWrite(A_1A, speed);
          Serial.println("전진");
          break;

        case MOTOR_BACKWARD:
          analogWrite(A_1B, speed);
          Serial.println("후진");
          break;

        case MOTOR_STOP:
        default:
          // 둘 다 이미 꺼졌음
          Serial.println("정지");
          break;
      }
    }
  }
}

// 명령 수신 Task
void SerialCommandTask(void *pvParameters) {
  char c;

  Serial.println("SerialCommandTask 시작됨");

  while (1) {
    if (Serial.available()) {
      c = Serial.read();
      Serial.print("입력(USB): "); Serial.println(c);
    } else if (mySerial.available()) {
      c = mySerial.read();
      Serial.print("입력(BT): "); Serial.println(c);
    } else {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    MotorCommand cmd = MOTOR_STOP;
    switch (c) {
      case 'a': cmd = MOTOR_FORWARD; break;
      case 'b': cmd = MOTOR_STOP; break;
      case 'c': cmd = MOTOR_BACKWARD; break;
      case 'd': cmd = MOTOR_STOP; break;
    }

    xQueueSend(motorQueue, &cmd, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// SETUP
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  Serial.println("RTOS 모터 제어 시작");

  // 큐 생성
  motorQueue = xQueueCreate(5, sizeof(MotorCommand));
  if (!motorQueue) {
    Serial.println("motorQueue 생성 실패");
    while (1); // 중단
  }

  // Task 생성
  xTaskCreate(MotorTask, "Motor", 256, NULL, 2, NULL);
  xTaskCreate(SerialCommandTask, "SerialCmd", 256, NULL, 1, NULL);

  vTaskStartScheduler();
}

void loop() {
}

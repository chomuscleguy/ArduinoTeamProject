#include <Arduino_FreeRTOS.h> // Arduino용 FreeRTOS 라이브러리 포함
#include "queue.h"

// 핀 설정
#define A_1A 6
#define A_1B 7
#define B_1A 4
#define B_1B 5
#define ECHO 8
#define TRIG 9

int speed = 200; // 모터 속도 설정

int greenButton = 2;
int redButton = 3;

bool greenState = false;
bool redState = false;
bool stopFlag = false; // 거리로 인한 정지 상태 저장

QueueHandle_t motorQueue; // 모터 제어 명령을 전달하는 Queue

typedef enum {
  MOTOR_STOP,
  MOTOR_FORWARD,
  MOTOR_BACKWARD
} MotorCommand;

// 모터 제어 Task
void MotorControlTask(void *pvParameters) {
  MotorCommand command;
  
  pinMode(A_1A, OUTPUT);
  pinMode(A_1B, OUTPUT);
  pinMode(B_1A, OUTPUT);
  pinMode(B_1B, OUTPUT);
  
  while (1) {
    if (xQueueReceive(motorQueue, &command, portMAX_DELAY) == pdPASS) {
      if (stopFlag) {
        command = MOTOR_STOP;
      }

      switch (command) {
        case MOTOR_FORWARD:
          // 전진
          analogWrite(A_1A, speed);
          analogWrite(A_1B, 0);
          analogWrite(B_1A, speed);
          analogWrite(B_1B, 0);
          break;
          
        case MOTOR_BACKWARD:
          // 후진
          analogWrite(A_1A, 0);
          analogWrite(A_1B, speed);
          analogWrite(B_1A, 0);
          analogWrite(B_1B, speed);
          break;
          
        case MOTOR_STOP:
        default:
          // 정지
          analogWrite(A_1A, 0);
          analogWrite(A_1B, 0);
          analogWrite(B_1A, 0);
          analogWrite(B_1B, 0);
          break;
      }
    }
  }
}

// 전진 버튼 Task
void ForwardTask(void *pvParameters) {
  pinMode(greenButton, INPUT_PULLUP);
  bool lastState = HIGH;
  
  while (1) {
    bool currentState = digitalRead(greenButton);

    if (lastState == HIGH && currentState == LOW) {
      greenState = !greenState;
      redState = false;

      MotorCommand command = greenState ? MOTOR_FORWARD : MOTOR_STOP;
      xQueueSend(motorQueue, &command, portMAX_DELAY);
      delay(100); // 디바운싱 처리
    }

    lastState = currentState;
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// 후진 버튼 Task
void BackwardTask(void *pvParameters) {
  pinMode(redButton, INPUT_PULLUP);
  bool lastState = HIGH;
  
  while (1) {
    bool currentState = digitalRead(redButton);

    if (lastState == HIGH && currentState == LOW) {
      redState = !redState;
      greenState = false;

      MotorCommand command = redState ? MOTOR_BACKWARD : MOTOR_STOP;
      xQueueSend(motorQueue, &command, portMAX_DELAY);
      delay(100); // 디바운싱 처리
    }

    lastState = currentState;
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// 초음파 센서 거리 측정 Task
void UltrasonicTask(void *pvParameters) {
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  while (1) {
    // 초음파 트리거 신호 발생
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    long duration = pulseIn(ECHO, HIGH);
    long distance = duration / 58;

    Serial.print("Distance : ");
    Serial.print(distance);
    Serial.println("cm");

    if (distance > 0 && distance < 10) {
      stopFlag = true;

      MotorCommand command = MOTOR_STOP;
      xQueueSend(motorQueue, &command, portMAX_DELAY);
    } else {
      stopFlag = false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void setup() {
  Serial.begin(9600);

  motorQueue = xQueueCreate(5, sizeof(MotorCommand));

  // Task 생성
  xTaskCreate(MotorControlTask, "MotorControl", 128, NULL, 2, NULL);
  xTaskCreate(ForwardTask, "Forward", 128, NULL, 1, NULL);
  xTaskCreate(BackwardTask, "Backward", 128, NULL, 1, NULL);
  xTaskCreate(UltrasonicTask, "Ultrasonic", 128, NULL, 3, NULL);

  vTaskStartScheduler(); // FreeRTOS 스케줄러 시작
}

void loop() {
  // FreeRTOS가 모든 Task를 관리하므로 여기는 필요하지 않음
}

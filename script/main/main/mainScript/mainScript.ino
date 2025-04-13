#include <Servo.h>
#include <SoftwareSerial.h>
// #include <Arduino_FreeRTOS.h>

// ---------- 핀 설정 ----------
#define sensorFront A0
#define sensorLeft A1
#define sensorRight A2
#define PIEZO 2
#define motorB1B 3
#define motorB1A 4
#define motorA1B 5
#define motorA1A 6
#define SHOCK 7
#define SERVO_PIN 8
#define RELAY_PIN 9
#define L_LED 10
#define R_LED 11

//-------페에조 음 설정-------
#define NOTE_E5 659
#define NOTE_DS5 622
#define NOTE_B4 494
#define NOTE_D5 587
#define NOTE_C5 523
#define NOTE_A4 440
#define NOTE_C4 262
#define NOTE_E4 330
#define NOTE_GS4 415
#define NOTE_B3 247

// ---------- 전역 객체 ----------
SoftwareSerial mySerial(12, 13);  // RX, TX
Servo doorServo;

// TaskHandle_t vibrationTaskHandle = NULL;

bool isDoorOpen = false;
bool isAlarmOn = false;
bool isAlarmLed = false;
bool triggered = false;
bool isAutoParking = false;
unsigned int alarmLastToggle = 0;
const int threshold = 1000;
int speed = 0;
int sensorValue = 0;
const int thresholdDistance = 15;


// ---------- 전역 함수 ----------
void DoubleLED(int pin1, int pin2, int state);
void BlinkLED(unsigned int lastToggle, int count, bool state);
void EngineSound(int interval, int count);
void ParkingSound();
void EmergencySound(int threshold);
void AutoParking();
void vibration();
// void PausevibrationTask();

// ---------- 상태 인터페이스 ----------
class IState {
public:
  virtual void enter() {}
  virtual void update() {}
  virtual void handleInput(char input) {}
  virtual void exit() {}
};

// ---------- FSM 컨트롤러 ----------
class FSMController {
private:
  IState* currentState;

public:
  FSMController()
    : currentState(nullptr) {}

  void changeState(IState* newState) {
    if (currentState) currentState->exit();
    currentState = newState;
    if (currentState) currentState->enter();
  }

  void update() {
    if (currentState) currentState->update();
  }

  void handleInput(char input) {
    if (currentState) currentState->handleInput(input);
  }
};

FSMController fsm;

// ---------- BaseState ----------
class BaseState : public IState {
public:
  void handleInput(char input) override {
    // 도어 제어
    if (input == '1' && isDoorOpen) {
      isDoorOpen = false;
      doorServo.write(90);
      Serial.println("문 닫힘");
    } else if (input == '2' && !isDoorOpen) {
      isDoorOpen = true;
      doorServo.write(0);
      Serial.println("문 열림");
    }
  }
  void update() {
    if (mySerial.available()) {
      char c = mySerial.read();
      fsm.handleInput(c);
    }

    if (isAlarmOn && millis() - alarmLastToggle > 300) {
      alarmLastToggle = millis();
      isAlarmLed = !isAlarmLed;
      DoubleLED(L_LED, R_LED, isAlarmLed);
    }
  }
};

// ---------- 상태 클래스들 정의 ----------
class EngineState;

class IdleState : public BaseState {
private:
  unsigned int lastToggle = 0;
  int count = 2;
  bool ledState = false;
  unsigned long lastCollisionTime = 0;
  const unsigned long collisionDelay = 1000;
public:
  void enter() override {
    Serial.println("🔵 상태: Idle (엔진 OFF)");
    digitalWrite(RELAY_PIN, LOW);
    BlinkLED(lastToggle, count, ledState);
    EngineSound(1000, 3);
    // vTaskResume(vibrationTaskHandle);
  }

  void handleInput(char input) override;

  void update() override {
    BaseState::update();
    EmergencySound(threshold);
    sensorValue = analogRead(SHOCK);
    Serial.print("센서 신호: ");
    Serial.println(sensorValue);
    if (sensorValue >= threshold && !triggered)
      vibration();
    if (sensorValue < threshold) {
      triggered = false;
    }
  }
};

class EngineState : public BaseState {
private:
  unsigned int lastToggle = 0;
  int count = 2;
  bool ledState = false;

public:
  void enter() override {
    Serial.println("🟢 상태: Engine ON");
    digitalWrite(RELAY_PIN, HIGH);
    BlinkLED(lastToggle, count, ledState);
    EngineSound(1200, 2);
    DoubleLED(L_LED, R_LED, HIGH);
    // PausevibrationTask();
  }

  void update() override {
    BaseState::update();
    BlinkLED(lastToggle, count, ledState);
    if (isAutoParking) {
      AutoParking();
    }
  }

  void handleInput(char input) override;

  void exit() override {
    Serial.println("🛑 엔진 종료");
    digitalWrite(RELAY_PIN, LOW);
    DoubleLED(L_LED, R_LED, LOW);
  }
};

IdleState idleState;
EngineState engineState;

// ---------- 상태 메서드 구현 ----------
void IdleState::handleInput(char input) {
  BaseState::handleInput(input);
  switch (input) {
    case 'd':
      //시동킴
      fsm.changeState(&engineState);
      break;
    case '9':
      isAlarmOn = true;
      break;
    case '0':
      isAlarmOn = false;
      break;
  }
}

void EngineState::handleInput(char input) {
  BaseState::handleInput(input);
  switch (input) {
    case 'p':
      //시동끔
      fsm.changeState(&idleState);
      break;
    case 'a':
      //자동주차
      isAutoParking = true;
      ParkingSound();
      break;
  }
}

// ---------- 유틸 ----------
void DoubleLED(int pin1, int pin2, int state) {
  digitalWrite(pin1, state);
  digitalWrite(pin2, state);
}

void BlinkLED(unsigned int lastToggle, int count, bool state) {
  if (millis() - lastToggle > 250 && count) {
    lastToggle = millis();
    state = !state;
    DoubleLED(L_LED, R_LED, state);
    count--;
  }
}

void EngineSound(int interval, int count) {
  for (int i = 0; i < count; i++) {
    tone(PIEZO, interval, 100);
    delay(200);
  }
  noTone(PIEZO);
}

void EmergencySound(int threshold) {
  int sensorValue = analogRead(SHOCK);

  if (sensorValue < threshold) return;
  if (isAlarmOn) {
    for (int i = 0; i < 10; i++) {
      if (isAlarmOn == false) {
        noTone(PIEZO);
        break;
      }

      tone(PIEZO, 1000, 200);
      delay(250);
    }
  }

  noTone(PIEZO);
}

// void vibrationTask(void *pvParameters) {
//   (void) pvParameters;

//   for (;;) {
//     int sensorValue = digitalRead(SHOCK);
//     Serial.print("센서 신호: ");
//     Serial.println(sensorValue);

//     if (sensorValue == LOW && !triggered) {  // 보통 D0는 충격 시 LOW
//       Serial.println("충돌이 감지되었습니다!");
//       triggered = true;
//       vTaskDelay(1000 / portTICK_PERIOD_MS);  // 1초 대기
//     }

//     if (sensorValue == HIGH) {
//       triggered = false;  // 신호 회복되면 다시 감지 가능
//     }

//     vTaskDelay(50 / portTICK_PERIOD_MS);  // 빠른 루프 주기
//   }
// }

// void PausevibrationTask() {
//   if (millis() > 5000) {
//     Serial.println("5초 지남 - VibrationTask 다시 시작!");
//     // vTaskResume(vibrationTaskHandle);
//   }
// }

void ParkingSound() {
  tone(PIEZO, NOTE_E5, 150);
  delay(200);
  tone(PIEZO, NOTE_DS5, 150);
  delay(200);
  tone(PIEZO, NOTE_E5, 150);
  delay(200);
  tone(PIEZO, NOTE_DS5, 150);
  delay(200);
  tone(PIEZO, NOTE_E5, 150);
  delay(200);
  tone(PIEZO, NOTE_B4, 150);
  delay(200);
  tone(PIEZO, NOTE_D5, 150);
  delay(200);
  tone(PIEZO, NOTE_C5, 150);
  delay(200);
  tone(PIEZO, NOTE_A4, 300);
  delay(350);

  tone(PIEZO, NOTE_C4, 150);
  delay(200);
  tone(PIEZO, NOTE_E4, 150);
  delay(200);
  tone(PIEZO, NOTE_A4, 150);
  delay(200);
  tone(PIEZO, NOTE_B4, 300);
  delay(350);

  tone(PIEZO, NOTE_E4, 150);
  delay(200);
  tone(PIEZO, NOTE_GS4, 150);
  delay(200);
  tone(PIEZO, NOTE_B4, 150);
  delay(200);
  tone(PIEZO, NOTE_C5, 300);
  delay(350);

  tone(PIEZO, NOTE_E4, 150);
  delay(200);
  tone(PIEZO, NOTE_E5, 150);
  delay(200);
  tone(PIEZO, NOTE_DS5, 150);
  delay(200);
  tone(PIEZO, NOTE_E5, 150);
  delay(200);
  tone(PIEZO, NOTE_DS5, 150);
  delay(200);
  tone(PIEZO, NOTE_E5, 150);
  delay(200);
  tone(PIEZO, NOTE_B4, 150);
  delay(200);
  tone(PIEZO, NOTE_D5, 150);
  delay(200);
  tone(PIEZO, NOTE_C5, 150);
  delay(200);
  tone(PIEZO, NOTE_A4, 300);
  delay(350);

  noTone(PIEZO);
}

// ---------- 라이프 사이클 ----------
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PIEZO, OUTPUT);
  pinMode(L_LED, OUTPUT);
  pinMode(R_LED, OUTPUT);
  pinMode(motorA1A, OUTPUT);
  pinMode(motorA1B, OUTPUT);
  pinMode(motorB1A, OUTPUT);
  pinMode(motorB1B, OUTPUT);
  pinMode(SHOCK, INPUT);

  doorServo.attach(SERVO_PIN);
  doorServo.write(90);

  // xTaskCreate(
  //   vibrationTask,
  //   "VibrationTask",
  //   128,
  //   NULL,
  //   1,
  //   NULL
  // );
  fsm.changeState(&idleState);
}

void loop() {
  fsm.update();
}

void AutoParking() {
  int analogFront = analogRead(sensorFront);
  int analogLeft = analogRead(sensorLeft);
  int analogRight = analogRead(sensorRight);

  // 아날로그 → 거리 변환 (센서에 따라 보정 필요)
  float distanceFront = 4800.0 / analogFront;
  float distanceLeft = 4800.0 / analogLeft;
  float distanceRight = 4800.0 / analogRight;

  // 디버깅용 출력
  Serial.print("Front: ");
  Serial.print(distanceFront);
  Serial.print(" cm, Left: ");
  Serial.print(distanceLeft);
  Serial.print(" cm, Right: ");
  Serial.println(distanceRight);

  // 15cm 이하일 경우 장애물로 판단 → 정지
  if (distanceFront <= thresholdDistance || distanceLeft <= thresholdDistance || distanceRight <= thresholdDistance) {
    stopMotors();
    isAutoParking = false;
    noTone(PIEZO);
  } else {
    moveForward();
  }

  delay(100);
}

void moveForward() {
  digitalWrite(motorA1A, HIGH);
  digitalWrite(motorA1B, LOW);
  digitalWrite(motorB1A, HIGH);
  digitalWrite(motorB1B, LOW);
}

void stopMotors() {
  digitalWrite(motorA1A, LOW);
  digitalWrite(motorA1B, LOW);
  digitalWrite(motorB1A, LOW);
  digitalWrite(motorB1B, LOW);
}



void vibration() {
  Serial.println("충돌이 감지되었습니다!");
  triggered = true;
}

#include <Servo.h>
#include <SoftwareSerial.h>
#include <Arduino_FreeRTOS.h>

// ---------- 핀 설정 ----------
#define PIEZO 4
#define LeftWheel 5
#define RightWheel 6
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

TaskHandle_t vibrationTaskHandle = NULL;

bool isDoorOpen = false;
bool isAlarmOn = false;
bool isAlarmLed = false;
bool triggered = false;
unsigned int alarmLastToggle = 0;
const int threshold = 1000;
int speed = 0;

// ---------- 전역 함수 ----------
void DoubleLED(int pin1, int pin2, int state);
void BlinkLED(unsigned int lastToggle, int count, bool state);
void EngineSound(int interval, int count);
void MoveSound();
void EmergencySound(int threshold);
void PausevibrationTask();

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
    PausevibrationTask();
  }

  void update() override {
    BaseState::update();
    BlinkLED(lastToggle, count, ledState);
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

void vibrationTask(void* pvParameters) {
  (void)pvParameters;

  for (;;) {
    int sensorValue = digitalRead(SHOCK);
    Serial.print("센서 신호: ");
    Serial.println(sensorValue);

    if (sensorValue == LOW && !triggered) {
      Serial.println("충돌이 감지되었습니다!");
      triggered = true;

      Serial.println("VibrationTask를 일시 중지합니다.");
      vTaskSuspend(NULL);  // 자기 자신을 일시 중지
    }

    if (sensorValue == HIGH) {
      triggered = false;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void PausevibrationTask() {
  if (millis() > 5000) {
    Serial.println("5초 지남 - VibrationTask 다시 시작!");
    vTaskResume(vibrationTaskHandle);
  }
}

void MoveSound() {
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

void forward() {
  digitalWrite(LeftWheel, LOW);
  analogWrite(LeftWheel, speed);
  digitalWrite(RightWheel, LOW);
  analogWrite(RightWheel, speed);
}

void backward() {
  digitalWrite(LeftWheel, HIGH);
  analogWrite(LeftWheel, 255 - speed);
  digitalWrite(RightWheel, HIGH);
  analogWrite(RightWheel, 255 - speed);
}

void turnLeft() {
  digitalWrite(LeftWheel, LOW);
  analogWrite(LeftWheel, speed);
  digitalWrite(RightWheel, HIGH);
  analogWrite(RightWheel, 255 - speed);
}

void turnRight() {
  digitalWrite(LeftWheel, HIGH);
  analogWrite(LeftWheel, 255 - speed);
  digitalWrite(RightWheel, LOW);
  analogWrite(RightWheel, speed);
}

void stop() {
  digitalWrite(LeftWheel, LOW);
  analogWrite(LeftWheel, 0);
  digitalWrite(RightWheel, LOW);
  analogWrite(RightWheel, 0);
}
// ---------- 라이프 사이클 ----------
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PIEZO, OUTPUT);
  pinMode(L_LED, OUTPUT);
  pinMode(R_LED, OUTPUT);
  pinMode(LeftWheel, OUTPUT);
  pinMode(RightWheel, OUTPUT);
  pinMode(SHOCK, INPUT);

  doorServo.attach(SERVO_PIN);
  doorServo.write(90);

  xTaskCreate(
    vibrationTask,
    "VibrationTask",
    128,
    NULL,
    1,
    &vibrationTaskHandle);
  
  Serial.println("작동");
  fsm.changeState(&idleState);
}

void loop() {
  fsm.update();
}

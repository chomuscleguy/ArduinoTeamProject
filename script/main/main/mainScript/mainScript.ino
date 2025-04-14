#include <Servo.h>
#include <SoftwareSerial.h>
// #include <Arduino_FreeRTOS.h>

// ---------- 핀 설정 ----------
#define sensorFront A1
#define sensorBack A0
#define sensorRight A2
#define PIEZO 2
#define motorB1B 3
#define motorB1A 4
#define motorA1B 5
#define motorA1A 6
#define SHOCK A5
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

bool isDoorOpen = false;
bool isAlarmOn = false;
bool isAlarmLed = false;
bool triggered = false;
bool ledState = false;
bool isAutoParking = false;
bool toggle = false;
unsigned int lastToggle = 0;
const int threshold = 1000;
int speed = 0;
int sensorValue = 0;
int direction = -1;
int soundCount = 0;
const int thresholdDistance = 15;
// BlinkLED 전용 변수
bool isBlinking = false;
int blinkTargetCount = 0;
int blinkCurrentCount = 0;
unsigned long blinkLastTime = 0;
bool blinkLedState = false;

const int melody[] = {
  NOTE_E5, NOTE_DS5, NOTE_E5, NOTE_DS5, NOTE_E5,
  NOTE_B4, NOTE_D5, NOTE_C5, NOTE_A4,
  NOTE_C4, NOTE_E4, NOTE_A4, NOTE_B4,
  NOTE_E4, NOTE_GS4, NOTE_B4, NOTE_C5,
  NOTE_E4, NOTE_E5, NOTE_DS5, NOTE_E5, NOTE_DS5,
  NOTE_E5, NOTE_B4, NOTE_D5, NOTE_C5, NOTE_A4
};

const int noteDurations[] = {
  150, 200, 150, 200, 150,
  200, 200, 200, 350,
  150, 200, 150, 350,
  150, 200, 150, 350,
  150, 200, 150, 200, 150,
  200, 200, 200, 200, 350
};

const int totalNotes = sizeof(melody) / sizeof(melody[0]);

int currentNoteIndex = 0;
unsigned long lastNoteTime = 0;
bool isPlayingParkingSound = false;

// ---------- 전역 함수 ----------
void DoubleLED(int pin1, int pin2, int state);
void BlinkLED(int count);
void EngineSound(int interval, int count);
void ParkingSound();
void EmergencySound();
void AutoParking();
void vibration();
void startBlinking(int count);
void updateBlinking();

// ---------- 상태 인터페이스 ----------
class IState {
public:
  virtual void enter() = 0;
  virtual void update() = 0;
  virtual void handleInput(char input) = 0;
  virtual void exit() = 0;
};

// ---------- FSM 컨트롤러 ----------
class StateMachine {
private:
  IState* currentState;

public:
  StateMachine()
    : currentState(nullptr) {
  }

  void changeState(IState* newState) {
    if (currentState) {
      currentState->exit();
    }
    currentState = newState;
    if (currentState) {
      currentState->enter();
    }
  }

  void update() {
    if (currentState) currentState->update();
  }

  void handleInput(char input) {
    if (currentState) currentState->handleInput(input);
  }
};

StateMachine fsm;

// ---------- BaseState ----------
class BaseState : public IState {
public:
  virtual void enter() override {
    // 필요한 초기화 작업
  }
  virtual void handleInput(char input) override {
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
  virtual void update() override {
    if (mySerial.available()) {
      char c = mySerial.read();
      fsm.handleInput(c);
    }
    if (isAlarmOn) {
      if (millis() - lastToggle > 300) {
        lastToggle = millis();
        isAlarmLed = !isAlarmLed;
      } else if (!isAlarmOn) {
        lastToggle = 0;
      }
    }
  }

  virtual void exit() override {
    // 필요 시 상태 종료 작업
  }
};

// ---------- 상태 클래스들 정의 ----------
class EngineState;

class IdleState : public BaseState {
private:
  unsigned long lastCollisionTime = 0;
  const unsigned long collisionDelay = 1000;
public:
  void enter() override {
    digitalWrite(RELAY_PIN, LOW);
    EngineSound(1000, 3);
    startBlinking(2);
  }

  void handleInput(char input) override;

  void update() override {
    BaseState::update();
    updateBlinking();
    sensorValue = analogRead(SHOCK);
    if (sensorValue >= threshold && !triggered) {
      vibration();
      isAlarmOn = true;
    }
    if (sensorValue < threshold) {
      triggered = false;
    }
    EmergencySound();
  }
  void exit() override {
  }
};

class EngineState : public BaseState {
private:
public:
  void enter() override {
    digitalWrite(RELAY_PIN, HIGH);
    EngineSound(1200, 2);
    ledState = true;
    startBlinking(3);
  }

  void update() override {
    BaseState::update();
    updateBlinking();
    if (isAutoParking) {
      AutoParking();
    }
  }

  void handleInput(char input) override;

  void exit() override {
    DoubleLED(L_LED, R_LED, LOW);
    digitalWrite(RELAY_PIN, LOW);
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
    case 'b':
      //자동주차
      isAutoParking = true;
      isPlayingParkingSound = true;
      //ParkingSound();
      direction = 0;
      break;
    case 'f':
      isAutoParking = true;
      isPlayingParkingSound = true;
      direction = 1;
  }
}

// ---------- 유틸 ----------
void DoubleLED(int pin1, int pin2, int state) {
  digitalWrite(pin1, state);
  digitalWrite(pin2, state);
}

void EngineSound(int interval, int count) {
  for (int i = 0; i < count; i++) {
    tone(PIEZO, interval, 100);
    delay(200);
  }
  noTone(PIEZO);
}

void startBlinking(int count) {
  isBlinking = true;
  blinkTargetCount = count;
  blinkCurrentCount = 0;
  blinkLastTime = millis();
  blinkLedState = false;
}

void updateBlinking() {
  if (!isBlinking) return;

  unsigned long now = millis();
  if (now - blinkLastTime >= 250) {
    blinkLastTime = now;
    blinkLedState = !blinkLedState;
    DoubleLED(L_LED, R_LED, blinkLedState);

    if (blinkLedState) {  // ON일 때만 카운트 증가
      blinkCurrentCount++;
      Serial.println(blinkCurrentCount);
    }

    if (blinkCurrentCount >= blinkTargetCount) {
      isBlinking = false;
      DoubleLED(L_LED, R_LED, LOW);
    }
  }
}

void EmergencySound() {
  if (!isAlarmOn) {
    DoubleLED(L_LED, R_LED, LOW);
    return;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastToggle >= 250 && soundCount < 10) {
    lastToggle = currentMillis;

    tone(PIEZO, 1000, 200);
    ledState = !ledState;
    DoubleLED(L_LED, R_LED, ledState);

    soundCount++;
  }

  if (soundCount >= 10) {
    noTone(PIEZO);
    DoubleLED(L_LED, R_LED, LOW);
    isAlarmOn = false;
    soundCount = 0;
    lastToggle = 0;
  }
}

void updateParkingSound() {
  if (!isPlayingParkingSound) {
    currentNoteIndex = 0;
    return;
  }

  unsigned long currentMillis = millis();
  int duration = noteDurations[currentNoteIndex];

  if (currentMillis - lastNoteTime >= duration) {
    currentNoteIndex++;

    if (currentNoteIndex >= totalNotes) {
      noTone(PIEZO);
      currentNoteIndex = 0;
    }

    tone(PIEZO, melody[currentNoteIndex], noteDurations[currentNoteIndex]);
    lastNoteTime = currentMillis;
  }
}

void startParkingSound() {
  isPlayingParkingSound = true;
  currentNoteIndex = 0;
  lastNoteTime = millis();
  tone(PIEZO, melody[0], noteDurations[0]);
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

  fsm.changeState(&idleState);
}

void loop() {
  fsm.update();
}

void AutoParking() {
  int analogFront = analogRead(sensorFront);
  int analogBack = analogRead(sensorBack);
  int analogRight = analogRead(sensorRight);

  float distanceFront = 4800.0 / analogFront;
  float distanceBack = 4800.0 / analogBack;
  float distanceRight = 4800.0 / analogRight;

  if (direction == 0 && distanceFront > thresholdDistance) {
    moveForward();
    updateParkingSound();
  } else if (direction == 1 && distanceBack > thresholdDistance) {
    moveBackward();
    updateParkingSound();
  } else {
    stopMotors();
    isPlayingParkingSound = false;
    isAutoParking = false;
    noTone(PIEZO);
    direction = -1;
  }

  delay(100);
}

void moveForward() {
  digitalWrite(motorA1A, HIGH);
  digitalWrite(motorA1B, LOW);
  digitalWrite(motorB1A, HIGH);
  digitalWrite(motorB1B, LOW);
}

void moveBackward() {
  digitalWrite(motorA1A, LOW);
  digitalWrite(motorA1B, HIGH);
  digitalWrite(motorB1A, LOW);
  digitalWrite(motorB1B, HIGH);
}

void stopMotors() {
  digitalWrite(motorA1A, LOW);
  digitalWrite(motorA1B, LOW);
  digitalWrite(motorB1A, LOW);
  digitalWrite(motorB1B, LOW);
}

void vibration() {
  isAlarmOn = true;
  triggered = true;
}

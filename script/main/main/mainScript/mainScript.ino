#include <SoftwareSerial.h>
#include <Servo.h>

#define PIEZO 4
#define A_1B 5
#define A_1A 6
#define Shock 7
#define SERVO_PIN 8
#define relayPin2 9
#define L_LED 10
#define R_LED 11

SoftwareSerial mySerial(12, 13);  // RX, TX
Servo doorServo;

bool isDoorOpen = false;
bool lastButtonState = HIGH;

int speed = 200;
int time = 50;
int x = 100;

int sensorValue = 0;

void DoubleLED(int Pin1, int Pin2, int state, int time);
void RemoteControl(char input);
void EmergencyBell();
void EngineState(int input);
void StopBell();
void OpenDoor();
void CloseDoor();

void setup() {
  pinMode(relayPin2, OUTPUT);
  pinMode(L_LED, OUTPUT);
  pinMode(R_LED, OUTPUT);
  pinMode(A_1A, OUTPUT);
  pinMode(A_1B, OUTPUT);
  pinMode(PIEZO, OUTPUT);
  doorServo.attach(SERVO_PIN);
  doorServo.write(0);
  mySerial.begin(9600);
  Serial.begin(9600);
}

void loop() {
  if (mySerial.available()) {
    char c = mySerial.read();
    EngineState(c);
    Serial.print(c);
    if (digitalRead(relayPin2) == HIGH) {
      //여기에 기능 다 넣기
      Serial.println("시동 켜짐");
      RemoteControl(c);
    } else if (digitalRead(relayPin2) == LOW) {
      // sensorValue = digitalRead(Shock);
      Serial.println("시동 꺼짐");
      // EmergencyBell();
      if (c == '9') {
        EmergencyBell();
      }
    }
  }
}

//BT에서 어플로 신호 보내는 기능
void RemoteControl(char input) {
  switch (input) {
    //1부터 4까지는 자동차 움직임
    case '1':
      OpenDoor();
      break;
    case '2':
      CloseDoor();
      break;
    case '3':
      //문닫힘
      break;
    case '4':
      //전진
      break;
    case '6':
      //후진
      break;
    case '7':

      break;
    case '8':
      break;
    case '9':
      EmergencyBell();
      break;
    case '0':
      StopBell();
      break;
  }
}

void EmergencyBell() {
  int step = 0;
  bool soundOn = false;
  unsigned long startTime = millis();

  while (true) {  // 무한 반복
    unsigned long currentTime = millis();

    // '0' 들어오면 경보 중단
    if (mySerial.available()) {
      char input = mySerial.read();
      if (input == '0') {
        noTone(PIEZO);
        Serial.println("비상 경고 중단됨");
        return;
      }
    }

    // 첫 번째 음 (2093Hz)
    if (!soundOn && currentTime - startTime >= 0) {
      tone(PIEZO, 2093);
      soundOn = true;
      startTime = currentTime;
    }

    // 두 번째 음으로 넘어가기
    if (soundOn && currentTime - startTime >= 250) {
      noTone(PIEZO);
      delay(200);
      tone(PIEZO, 1567);
      delay(250);
      noTone(PIEZO);
      delay(200);

      soundOn = false;
      startTime = millis();
    }
  }
}

void StopBell() {
  noTone(PIEZO);
  Serial.println("사이렌 꺼짐");
}

void DoubleLED(int Pin1, int Pin2, int state) {
  digitalWrite(Pin1, state);
  digitalWrite(Pin2, state);
}

void EngineLED(int Pin1, int Pin2, int count) {
  const unsigned long interval = 250;
  unsigned long previousMillis = 0;
  bool ledState = false;
  int blinkCount = 0;

  while (blinkCount < count * 2) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      ledState = !ledState;
      DoubleLED(Pin1, Pin2, ledState ? HIGH : LOW);
      blinkCount++;
    }
  }
}

void EngineState(int input) {
  switch (input) {
    case 'd':
      digitalWrite(relayPin2, HIGH);
      EngineLED(L_LED, R_LED, 2);
      break;
    case 'p':
      digitalWrite(relayPin2, LOW);
  }
}

void OpenDoor() {
  doorServo.write(90);
  isDoorOpen = true;
  Serial.println("문 열림");
}

void CloseDoor() {
  doorServo.write(0);
  isDoorOpen = false;
  Serial.println("문 닫힘");
}

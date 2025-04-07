//결과 정리
// red버튼 누름 -> 후진  // 한번더 누르면 정지
// green버튼 누르므 -> 전진 // 한번더 누르면 정지
// 거리가 10cm 이하이면 정지

#define A_1A 7
#define A_1B 6
#define B_1A 4
#define B_1B 5
#define ECHO 8
#define TRIG 9

int speed = 200;

int greenButton = 2;
int redButton = 3;

bool greenState = false;
bool greenLastState = HIGH;

bool redState = false;
bool redLastState = HIGH;

bool stopFlag = false; // 거리로 인한 정지 상태 저장

void setup() {
  Serial.begin(9600);

  pinMode(greenButton, INPUT_PULLUP);
  pinMode(redButton, INPUT_PULLUP);

  pinMode(A_1A, OUTPUT);
  pinMode(A_1B, OUTPUT);
  pinMode(B_1A, OUTPUT);
  pinMode(B_1B, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
}

void loop() {
  bool currentGreen = digitalRead(greenButton);
  bool currentRed = digitalRead(redButton);

  // 초음파 거리 측정
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH);

  if(duration == 0)
  {
    return;
  }
    
  long distance = duration / 58;

  Serial.print("Distance : ");
  Serial.print(distance);
  Serial.println("cm");

  // 거리가 10cm 미만이면 정지 상태로 전환
  if (distance < 10) {
    stopFlag = true;
  }

  // 정지 상태일 경우 모터 정지 및 버튼 눌림 대기
  if (stopFlag) {
    analogWrite(A_1A, 0);
    analogWrite(A_1B, 0);
    analogWrite(B_1A, 0);
    analogWrite(B_1B, 0);

    if ((greenLastState == HIGH && currentGreen == LOW) ||
        (redLastState == HIGH && currentRed == LOW)) {
      stopFlag = false;
      greenState = false;
      redState = false;
      delay(100);
    }

    greenLastState = currentGreen;
    redLastState = currentRed;
    return;
  }

  // 버튼 눌림 감지 및 상태 토글
  if (greenLastState == HIGH && currentGreen == LOW) {
    greenState = !greenState;
    redState = false;
    delay(100);
  }

  if (redLastState == HIGH && currentRed == LOW) {
    redState = !redState;
    greenState = false;
    delay(100);
  }

  greenLastState = currentGreen;
  redLastState = currentRed;

  // 모터 동작 처리
  if (greenState) {
    // 전진
    analogWrite(A_1A, speed);
    analogWrite(A_1B, 0);
    analogWrite(B_1A, speed);
    analogWrite(B_1B, 0);
  } else if (redState) {
    // 후진
    analogWrite(A_1A, 0);
    analogWrite(A_1B, speed);
    analogWrite(B_1A, 0);
    analogWrite(B_1B, speed);
  } else {
    // 정지
    analogWrite(A_1A, 0);
    analogWrite(A_1B, 0);
    analogWrite(B_1A, 0);
    analogWrite(B_1B, 0);
  }
}

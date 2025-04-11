// 센서 핀
const int sensorFront = A0;
const int sensorLeft = A1;
const int sensorRight = A2;

// 모터 핀
const int motorA1A = 7;
const int motorA1B = 6;
const int motorB1A = 5;
const int motorB1B = 4;

// 거리 기준 (cm)
const int thresholdDistance = 15;

void setup() {
  // 시리얼 모니터 디버깅용
  Serial.begin(9600);

  // 모터 출력
  pinMode(motorA1A, OUTPUT);
  pinMode(motorA1B, OUTPUT);
  pinMode(motorB1A, OUTPUT);
  pinMode(motorB1B, OUTPUT);
}

void loop() {
  // 아날로그 값 읽기
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
  if (distanceFront <= thresholdDistance ||
      distanceLeft <= thresholdDistance ||
      distanceRight <= thresholdDistance) {
    stopMotors();
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

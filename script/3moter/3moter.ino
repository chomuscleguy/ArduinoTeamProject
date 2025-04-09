#include <SoftwareSerial.h>

#define A_1A 6  // 모터드라이브 A_1A 6번핀 설정
#define A_1B 5  // 모터드라이브 A_1B 5번핀 설정

SoftwareSerial mySerial(2, 3);

int speed = 200;  // 모터스피드 200으로 변수 설정

void setup() {
  mySerial.begin(9600);
  pinMode(A_1A, OUTPUT);  // 출력 핀모드 A_1A
  pinMode(A_1B, OUTPUT);  // 출력 핀모드 A_1B
  Serial.begin(9600);
}

void loop() {
  if (mySerial.available()) {
    char c = mySerial.read();
    mySerial.println(c);
    switch (c) {
      case 'a':
        analogWrite(A_1A, speed);  // 정방향 5초 동작
        analogWrite(A_1B, 0);
        break;
      case 'b':
        analogWrite(A_1A, 0);  // 1초 정지
        analogWrite(A_1B, 0);
        break;
      case 'c':
        analogWrite(A_1A, 0);  // 역방향 5초 동작
        analogWrite(A_1B, speed);
        break;
      case 'd':
        analogWrite(A_1A, 0);  // 1초 정지
        analogWrite(A_1B, 0);
        break;
    }
  }

  if (Serial.available()) {
    char c = Serial.read();
    Serial.println(c);
    switch (c) {
      case 'a':
        analogWrite(A_1A, speed);  // 정방향 5초 동작
        analogWrite(A_1B, 0);
        break;
      case 'b':
        analogWrite(A_1A, 0);  // 1초 정지
        analogWrite(A_1B, 0);
        break;
      case 'c':
        analogWrite(A_1A, 0);  // 역방향 5초 동작
        analogWrite(A_1B, speed);
        break;
      case 'd':
        analogWrite(A_1A, 0);  // 1초 정지
        analogWrite(A_1B, 0);
        break;
    }
  }
}

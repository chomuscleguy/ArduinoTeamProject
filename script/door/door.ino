#include <Servo.h> // 서보 모터 라이브러리 추가

Servo myservo;  // 서보 모터 제어를 위한 객체 생성

int yellowbutton = 2;
bool currentState = false;     // 현재 위치 상태 (false = 0도, true = 90도)
bool lastButtonState = HIGH;   // 이전 버튼 상태

void setup() {
  myservo.attach(6);           // 서보 모터 핀 설정
  pinMode(yellowbutton, INPUT_PULLUP); // 버튼 입력 핀 (풀업 저항 사용)
  myservo.write(0);            // 초기 위치는 0도로 설정
}

void loop() {
  bool buttonState = digitalRead(yellowbutton);

  // 버튼이 눌리는 순간 감지 (HIGH → LOW)
  if (lastButtonState == HIGH && buttonState == LOW) {
    currentState = !currentState; // 상태 반전

    if (currentState) {
      myservo.write(90); // 90도로 이동
    } else {
      myservo.write(0);  // 0도로 이동
    }

    delay(200); // 서보 대기
  }

  lastButtonState = buttonState;
}

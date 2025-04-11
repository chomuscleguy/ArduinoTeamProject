int greenled = 3;
int redled = 4;
int frontbutton = 13;
int backbutton=12;

int frontdistance = 0;
int backdistance = 0;

void setup() {
  Serial.begin(9600);
  pinMode(greenled, OUTPUT);
  pinMode(redled, OUTPUT);
  pinMode(frontbutton, INPUT_PULLUP);  // 내부 풀업 사용
  pinMode(backbutton, INPUT_PULLUP);  // 내부 풀업 사용

  digitalWrite(greenled, LOW);
  digitalWrite(redled, LOW);
}

void loop() {
  int frontcurrentbutton = digitalRead(frontbutton);
  int backcurrentbutton = digitalRead(backbutton);

// //전진
//   if (frontcurrentbutton == LOW) {  // 버튼이 눌렸을 때만
//     digitalWrite(greenled, HIGH);

//     int frontvolt = map(analogRead(A0), 0, 1023, 0, 5000);
//     frontdistance = (27.61 / (frontvolt - 0.1696)) * 1000;
//     int backvolt = map(analogRead(A1), 0, 1023, 0, 5000);
//     backdistance = (27.61 / (backvolt - 0.1696)) * 1000;

//     Serial.print("front : ");
//     Serial.print(frontdistance);
//     Serial.print(" cm ");
//     Serial.print("back : ");
//     Serial.print(backdistance);
//     Serial.println(" cm");

//     if (frontdistance <= 15 ) {
//       digitalWrite(greenled, LOW);
//       digitalWrite(redled, HIGH);
//     } else {
//       digitalWrite(redled, LOW);
//     }

//   } else {
//     // 버튼에서 손을 떼면 LED들 꺼짐
//     digitalWrite(greenled, LOW);
//     digitalWrite(redled, LOW);
//   }

  //후진
  if (backcurrentbutton == LOW) {  // 버튼이 눌렸을 때만
    digitalWrite(greenled, HIGH);

    int frontvolt = map(analogRead(A0), 0, 1023, 0, 5000);
    frontdistance = (27.61 / (frontvolt - 0.1696)) * 1000;
    int backvolt = map(analogRead(A1), 0, 1023, 0, 5000);
    backdistance = (27.61 / (backvolt - 0.1696)) * 1000;

    Serial.print("front : ");
    Serial.print(frontdistance);
    Serial.print(" cm ");
    Serial.print("back : ");
    Serial.print(backdistance);
    Serial.println(" cm");

    if ( backdistance <= 15) {
      digitalWrite(greenled, LOW);
      digitalWrite(redled, HIGH);
    } else {
      digitalWrite(redled, LOW);
    }

  } else {
    // 버튼에서 손을 떼면 LED들 꺼짐
    digitalWrite(greenled, LOW);
    digitalWrite(redled, LOW);
  }

  delay(100);  // 깜빡임 방지 및 너무 잦은 측정 방지
}

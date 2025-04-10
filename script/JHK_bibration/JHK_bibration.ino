const int sensorPin = A1;  
const int threshold = 1000;     // 센서 구조상 낮아질 때 충격이라고 가정

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
}

void loop() {
  int sensorValue = analogRead(sensorPin);
  Serial.print("진동 값: ");
  Serial.println(sensorValue);

  if (sensorValue < threshold) {
    Serial.println("충돌이 감지되었습니다!");
    delay(1000); 
  }

  delay(100);
}

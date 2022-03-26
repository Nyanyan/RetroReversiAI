#define hw 8
#define led 8

const int outs[hw] = {11, 12, 13, A0, A1, A2, A3, A4};
const int ins[hw] = {2, 3, 4, 5, 6, 7, 9, 10};

void setup() {
  for (int i = 0; i < hw; i++) {
    pinMode(outs[i], OUTPUT);
    digitalWrite(outs[i], LOW);
    pinMode(ins[i], INPUT);
  }
  pinMode(led, OUTPUT);
  Serial.begin(1200);
}

void loop() {
  while (!Serial.available());
  Serial.write((byte)255);
  digitalWrite(led, HIGH);
  while (Serial.available())
    Serial.read();
  int y = -1;
  int x = -1;
  bool flag = false;
  while (!flag) {
    for (int i = 0; i < hw; i++) {
      digitalWrite(outs[i], HIGH);
      delay(1);
      for (int j = 0; j < hw; j++) {
        if (digitalRead(ins[j]) == HIGH) {
          y = i;
          x = j;
          flag = true;
          break;
        }
      }
      digitalWrite(outs[i], LOW);
      if (flag)
        break;
    }
  }
  Serial.write((byte)y);
  Serial.write((byte)x);
  digitalWrite(led, LOW);
}

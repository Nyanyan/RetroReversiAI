#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX

void setup() {
  Serial.begin(9600); // ハードウェアシリアルを準備
  Serial.print(0);
  mySerial.begin(9600); // ソフトウェアシリアルの初期化
  //mySerial.println("Hello, world?");
}

void loop() {
  while (mySerial.available()) Serial.write(mySerial.read() + 1);
  while (Serial.available()) mySerial.write(Serial.read() + 1);
}

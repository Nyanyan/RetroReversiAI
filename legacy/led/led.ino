#define DATAPIN    (9)   // 74HC595のDSへ
#define LATCHPIN  (11)  // 74HC595のST_CPへ
#define CLOCKPIN  (12)  // 74HC595のSH_CPへ

void MyShiftOut( int dataPin, int clockPin, int bit, int val )
{
  for( int i = 0; i < bit; i++ )
  {
    digitalWrite(dataPin, !!(val & (1 << i)));
      
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }
}

void setup()
{
  pinMode(DATAPIN, OUTPUT);
  pinMode(LATCHPIN, OUTPUT);
  pinMode(CLOCKPIN, OUTPUT);
}

void loop()
{
  for( int i = 0; i < 8; i++ )
  {
    digitalWrite(LATCHPIN, LOW);    // 送信中はLATCHPINをLOWに

    // シフト演算を使って点灯するLEDを選択しています
    MyShiftOut( DATAPIN, CLOCKPIN, 8, 1 << i );

    digitalWrite(LATCHPIN, HIGH);   // 送信後はLATCHPINをHIGHに戻す

    delay(500);
  }
}

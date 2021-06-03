#define DATAPIN 9
#define LATCHPIN 11
#define CLOCKPIN 12

void MyShiftOut( int dataPin, int clockPin, int bt, unsigned long val )
{
  for( int i = 0; i < bt; i++ )
  {
    digitalWrite(dataPin, !!(val & (1L << i)));
      
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
  for( int i = 0; i < 16; i++ )
  {
    digitalWrite(LATCHPIN, LOW);
    MyShiftOut( DATAPIN, CLOCKPIN, 16, 1L << i );
    digitalWrite(LATCHPIN, HIGH);

    delay(100);
  }
}

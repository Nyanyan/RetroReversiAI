#define DATAPIN1 2
#define LATCHPIN1 3
#define CLOCKPIN1 4
#define DATAPIN2 9
#define LATCHPIN2 10
#define CLOCKPIN2 11
#define mx_bt 64

const int arr_g[64] = {
  62, 61, 60, 59, 58, 57, 56, 63,
  54, 53, 52, 51, 50, 49, 48, 55,
  46, 45, 44, 43, 42, 41, 40, 47,
  38, 37, 36, 35, 34, 33, 32, 39,
  30, 29, 28, 27, 26, 25, 24, 31,
  22, 21, 20, 19, 18, 17, 16, 23,
  14, 13, 12, 11, 10, 9, 8, 15,
  6, 5, 4, 3, 2, 1, 0, 7
};

const int arr_r[64] = {
  47, 40, 41, 42, 43, 44, 45, 46,
  55, 48, 49, 50, 51, 52, 53, 54,
  63, 56, 57, 58, 59, 60, 61, 62,
  23, 16, 17, 18, 19, 20, 21, 22,
  31, 24, 25, 26, 27, 28, 29, 30,
  39, 32, 33, 34, 35, 36, 37, 38,
  7, 0, 1, 2, 3, 4, 5, 6,
  15, 8, 9, 10, 11, 12, 13, 14
};

void MyShiftOut( int dataPin, int clockPin, bool* arr)
{
  for ( int i = 0; i < mx_bt; i++ )
  {
    digitalWrite(dataPin, arr[i]);
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }
}

void setup()
{
  pinMode(DATAPIN1, OUTPUT);
  pinMode(LATCHPIN1, OUTPUT);
  pinMode(CLOCKPIN1, OUTPUT);
  pinMode(DATAPIN2, OUTPUT);
  pinMode(LATCHPIN2, OUTPUT);
  pinMode(CLOCKPIN2, OUTPUT);
  bool arr[64];
  for (int i = 0; i < 64; i++)
    arr[i] = false;
  digitalWrite(LATCHPIN1, LOW);    // 送信中はLATCHPINをLOWに
  MyShiftOut( DATAPIN1, CLOCKPIN1, arr);
  digitalWrite(LATCHPIN1, HIGH);   // 送信後はLATCHPINをHIGHに戻す
  digitalWrite(LATCHPIN2, LOW);    // 送信中はLATCHPINをLOWに
  MyShiftOut( DATAPIN2, CLOCKPIN2, arr);
  digitalWrite(LATCHPIN2, HIGH);   // 送信後はLATCHPINをHIGHに戻す
}

void loop()
{
  bool arr[64], arr2[64];
  /*
    int t = 64;
    for (int i = 0; i < t; i++)
    arr[i] = true;
    for (int i = t; i < 64; i++)
    arr[i] = false;
    for (int i = 0; i < 64; i++)
    arr2[arr_r[i]] = arr[i];
    digitalWrite(LATCHPIN2, LOW);    // 送信中はLATCHPINをLOWに

    // シフト演算を使って点灯するLEDを選択しています
    MyShiftOut( DATAPIN2, CLOCKPIN2, arr2);

    digitalWrite(LATCHPIN2, HIGH);   // 送信後はLATCHPINをHIGHに戻す

    delay(100);
  */
  for (int u = 0; u < 64; u++) {
    for (int i = 0; i < 64; i++)
      arr[i] = false;
    arr[u] = true;
    for (int i = 0; i < 64; i++)
      arr2[arr_g[i]] = arr[i];
    digitalWrite(LATCHPIN1, LOW);    // 送信中はLATCHPINをLOWに
    MyShiftOut( DATAPIN1, CLOCKPIN1, arr2);
    digitalWrite(LATCHPIN1, HIGH);   // 送信後はLATCHPINをHIGHに戻す
    delay(50);
  }
  for (int i = 0; i < 64; i++)
    arr[i] = false;
  for (int i = 0; i < 64; i++)
    arr2[arr_g[i]] = arr[i];
  digitalWrite(LATCHPIN1, LOW);    // 送信中はLATCHPINをLOWに
  MyShiftOut( DATAPIN1, CLOCKPIN1, arr2);
  digitalWrite(LATCHPIN1, HIGH);   // 送信後はLATCHPINをHIGHに戻す
  for (int u = 0; u < 64; u++) {
    for (int i = 0; i < 64; i++)
      arr[i] = false;
    arr[u] = true;
    for (int i = 0; i < 64; i++)
      arr2[arr_r[i]] = arr[i];
    digitalWrite(LATCHPIN2, LOW);    // 送信中はLATCHPINをLOWに
    MyShiftOut( DATAPIN2, CLOCKPIN2, arr2);
    digitalWrite(LATCHPIN2, HIGH);   // 送信後はLATCHPINをHIGHに戻す
    delay(50);
  }
  for (int i = 0; i < 64; i++)
    arr[i] = false;
  for (int i = 0; i < 64; i++)
    arr2[arr_r[i]] = arr[i];
  digitalWrite(LATCHPIN2, LOW);    // 送信中はLATCHPINをLOWに
  MyShiftOut( DATAPIN2, CLOCKPIN2, arr2);
  digitalWrite(LATCHPIN2, HIGH);   // 送信後はLATCHPINをHIGHに戻す
}

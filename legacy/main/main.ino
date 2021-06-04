#include <TM1637Display.h>
#include <SoftwareSerial.h>

#define CLK 2
#define DIO 3
TM1637Display display(CLK, DIO);

#define DATAPIN1 4
#define LATCHPIN1 5
#define CLOCKPIN1 6
#define DATAPIN2 7
#define LATCHPIN2 8
#define CLOCKPIN2 9

SoftwareSerial ai(10, 11); // RX, TX

#define hw 8
#define hw2 64

void output_led(int lpin, int dpin, int cpin, uint64_t val) {
  digitalWrite(lpin, LOW);
  for (int i = 0; i < hw2; i++) {
    digitalWrite(dpin, !!(val & ((uint64_t)1 << i)));
    digitalWrite(cpin, HIGH);
    digitalWrite(cpin, LOW);
  }
  digitalWrite(lpin, HIGH);
}
/*
  void print_board(const int* p, const int* o, const int* m){
  uint64_t pp, oo, mm;
  for (int i = 0; i < hw; i++){
    pp <<= hw;
    oo <<= hw;
    mm <<= hw;
    pp |= p[i];
    oo |= o[i];
    mm |= m[i];
  }
  output_led(LATCHPIN1, DATAPIN1, CLOCKPIN1, pp | mm);
  output_led(LATCHPIN2, DATAPIN2, CLOCKPIN2, oo | mm);
  int num = pop_count(p) * 100 + pop_count(o);
  display.showNumberDecEx(num,0x40,true);
  }
*/
void print_board(const int* p, const int* o, const int* m) {
  Serial.println("  a b c d e f g h ");
  for (int i = 0; i < hw; i++) {
    Serial.print(i + 1);
    Serial.print(" ");
    for (int j = hw - 1; j >= 0; j--) {
      if (1 & (p[i] >> j))
        Serial.print("0 ");
      else if (1 & (o[i] >> j))
        Serial.print("1 ");
      else if (1 & (m[i] >> j))
        Serial.print("* ");
      else
        Serial.print(". ");
    }
    Serial.println("");
  }
}

bool inside(int y, int x) {
  return 0 <= y && y < hw && 0 <= x && x < hw;
}

int check_mobility_line(int p, int o) {
  int p1 = p << 1;
  return (~(p1 | o)) & (p1 + o);
}

int reverse(int a) {
  int res = 0;
  for (int i = 0; i < hw; i++) {
    res <<= 1;
    res |= 1 & (a >> i);
  }
  return res;
}

void check_mobility(const int* me, const int* op, int* mobility) {
  for (int i = 0; i < hw; i++)
    mobility[i] = 0;
  int mobility_tmp, p, o;
  for (int i = 0; i < hw; i++) {
    mobility[i] |= check_mobility_line(me[i], op[i]);
    mobility[i] |= reverse(check_mobility_line(reverse(me[i]), reverse(op[i])));
  }
  for (int i = 0; i < hw; i++) {
    p = 0;
    o = 0;
    for (int j = 0; j < hw; j++) {
      p <<= 1;
      o <<= 1;
      p |= 1 & (me[j] >> i);
      o |= 1 & (op[j] >> i);
    }
    mobility_tmp = check_mobility_line(p, o);
    mobility_tmp |= reverse(check_mobility_line(reverse(p), reverse(o)));
    for (int j = 0; j < hw; j++)
      mobility[j] |= (1 & (mobility_tmp >> (hw - 1 - j))) << i;
  }
  for (int i = 2; i < hw * 2 - 2; i++) {
    p = 0;
    o = 0;
    for (int j = 0; j < hw; j++) {
      p <<= 1;
      o <<= 1;
      if (i - j < 0)
        continue;
      if (i - j >= hw)
        continue;
      p |= (1 & (me[j] >> (i - j)));
      o |= (1 & (op[j] >> (i - j)));
    }
    mobility_tmp = check_mobility_line(p, o);
    mobility_tmp |= reverse(check_mobility_line(reverse(p), reverse(o)));
    for (int j = 0; j < hw; j++) {
      if (i - j < 0)
        continue;
      if (i - j >= hw)
        continue;
      mobility[j] |= (1 & (mobility_tmp >> (hw - 1 - j))) << (i - j);
    }
  }
  for (int i = 2; i < hw * 2 - 2; i++) {
    p = 0;
    o = 0;
    for (int j = 0; j < hw; j++) {
      p <<= 1;
      o <<= 1;
      if (hw - 1 - i + j < 0)
        continue;
      if (hw - 1 - i + j >= hw)
        continue;
      p |= (1 & (me[j] >> (hw - 1 - i + j)));
      o |= (1 & (op[j] >> (hw - 1 - i + j)));
    }
    mobility_tmp = check_mobility_line(p, o);
    mobility_tmp |= reverse(check_mobility_line(reverse(p), reverse(o)));
    for (int j = 0; j < hw; j++) {
      if (hw - 1 - i + j < 0)
        continue;
      if (hw - 1 - i + j >= hw)
        continue;
      mobility[j] |= (1 & (mobility_tmp >> (hw - 1 - j))) << (hw - 1 - i + j);
    }
  }
  for (int i = 0; i < hw; i++) {
    mobility[i] &= ~me[i];
    mobility[i] &= ~op[i];
  }
}

void trans(const int* pt, const int k, int* res) {
  switch (k) {
    case 0:
      for (int i = 0; i < hw - 1; i++)
        res[i] = pt[i + 1];
      res[hw - 1] = 0;
      break;
    case 1:
      for (int i = 0; i < hw - 1; i++)
        res[i] = pt[i + 1] >> 1;
      res[hw - 1] = 0;
      break;
    case 2:
      for (int i = 0; i < hw; i++)
        res[i] = pt[i] >> 1;
      break;
    case 3:
      for (int i = 1; i < hw; i++)
        res[i] = pt[i - 1] >> 1;
      res[0] = 0;
      break;
    case 4:
      for (int i = 1; i < hw; i++)
        res[i] = pt[i - 1];
      res[0] = 0;
      break;
    case 5:
      for (int i = 1; i < hw; i++)
        res[i] = pt[i - 1] << 1;
      res[0] = 0;
      break;
    case 6:
      for (int i = 0; i < hw; i++)
        res[i] = pt[i] << 1;
      break;
    case 7:
      for (int i = 0; i < hw - 1; i++)
        res[i] = pt[i + 1] << 1;
      res[hw - 1] = 0;
      break;
  }
}

void move_board(const int* me, const int* op, const int* pt, int* me_res, int* op_res) {
  int rev[hw] = {0, 0, 0, 0, 0, 0, 0, 0};
  int rev2[hw];
  int mask[hw], tmp[hw];
  bool flag1, flag2, flag3;
  for (int k = 0; k < 8; k++) {
    for (int i = 0; i < hw; i++)
      rev2[i] = 0;
    trans(pt, k, mask);
    flag1 = false;
    flag2 = false;
    for (int i = 0; i < hw; i++) {
      if (mask[i])
        flag1 = true;
      if (mask[i] & op[i])
        flag2 = true;
    }
    while (flag1 && flag2) {
      for (int i = 0; i < hw; i++) {
        rev2[i] |= mask[i];
        tmp[i] = mask[i];
      }
      trans(tmp, k, mask);
      flag3 = false;
      for (int i = 0; i < hw; i++)
        if (mask[i] & me[i])
          flag3 = true;
      if (flag3)
        for (int i = 0; i < hw; i++)
          rev[i] |= rev2[i];
      flag1 = false;
      flag2 = false;
      for (int i = 0; i < hw; i++) {
        if (mask[i])
          flag1 = true;
        if (mask[i] & op[i])
          flag2 = true;
      }
    }
  }
  for (int i = 0; i < hw; i++) {
    me_res[i] = me[i] ^ (pt[i] | rev[i]);
    op_res[i] = op[i] ^ rev[i];
  }
}

int pop_count(const int* x) {
  int res = 0;
  for (int i = 0; i < hw; i++)
    for (int j = 0; j < hw; j++)
      res += 1 & (x[i] >> j);
  return res;
}\

void play() {
  int black[hw] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00001000,
    0b00010000,
    0b00000000,
    0b00000000,
    0b00000000
  };
  int white[hw] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00010000,
    0b00001000,
    0b00000000,
    0b00000000,
    0b00000000
  };
  int pt[hw] = {0, 0, 0, 0, 0, 0, 0, 0};
  int skip_cnt = 0;
  int mobility[hw];
  int turn = 0;
  int y, x;
  int stones = 5;
  while (skip_cnt < 2) {
    if (turn == 0)
      check_mobility(black, white, mobility);
    else
      check_mobility(white, black, mobility);
    if (pop_count(mobility))
      skip_cnt = 0;
    else {
      Serial.println("Skip!");
      skip_cnt++;
      turn = 1 - turn;
      continue;
    }
    print_board(black, white, mobility);
    if (turn == 0) {
      y = -1;
      x = 0;
      while (!(inside(y, x) && mobility[y] & (1 << (hw - 1 - x)))) {
        //Serial.println("input a move");
        while (!Serial.available());
        x = (int)(Serial.read() - 'a');
        while (!Serial.available());
        y = (int)(Serial.read() - '1');
        Serial.print((char)(x + 'a'));
        Serial.println(y + 1);
      }
      for (int i = 0; i < hw; i++)
        pt[i] = 0;
      pt[y] |= 1 << (hw - 1 - x);
      move_board(black, white, pt, black, white);
    } else {
      ai.listen();
      for (int i = 0; i < hw * 4; i++)
        ai.write((byte)0);
      for (int i = 0; i < hw; i++)
        ai.print(white[i]);
      for (int i = 0; i < hw; i++)
        ai.print(black[i]);
      int zero_cnt = 0;
      while (zero_cnt < hw * 2) {
        if (ai.available()) {
          byte tmp = ai.read();
          Serial.print(tmp);
          Serial.print(" ");
          if (tmp)
            zero_cnt = 0;
          else
            zero_cnt++;
          Serial.println(zero_cnt);
        }
      }
      for (int i = 0; i < hw; i++)
        pt[i] = (int)ai.read();
      print_board(pt, pt, pt);
      move_board(white, black, pt, white, black);
    }
    ++stones;
    turn = 1 - turn;
  }
  print_board(black, white, mobility);
  Serial.print("black(0): ");
  Serial.println(pop_count(black));
  Serial.print("white(1): ");
  Serial.println(pop_count(white));
}

void setup() {
  Serial.begin(9600);
  ai.begin(9600);
  pinMode(DATAPIN1, OUTPUT);
  pinMode(LATCHPIN1, OUTPUT);
  pinMode(CLOCKPIN1, OUTPUT);
  pinMode(DATAPIN2, OUTPUT);
  pinMode(LATCHPIN2, OUTPUT);
  pinMode(CLOCKPIN2, OUTPUT);
  display.setBrightness(0x0f);
}

void loop() {
  play();
}

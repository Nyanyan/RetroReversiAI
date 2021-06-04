#include <Wire.h>
#include <TM1637Display.h>
#include <SoftwareSerial.h>

#define CLK 2
#define DIO 3
TM1637Display display(CLK, DIO);

#define DATAPIN1 4
#define LATCHPIN1 6
#define CLOCKPIN1 7
#define DATAPIN2 9
#define LATCHPIN2 10
#define CLOCKPIN2 11

SoftwareSerial button(10, 11); // RX, TX

#define hw 8
#define hw2 64
#define n_slaves 8
#define slave_depth 1
#define max_depth 4

const int slaves[n_slaves] = {8, 9, 10, 11, 12, 13, 14, 15};

const float weight[hw][hw] = {
  {3.35, -0.65, 2.6, -0.45, -0.45, 2.6, -0.65, 3.35},
  { -0.65, -3.15, -1.7, -0.4, -0.4, -1.7, -3.15, -0.65},
  {2.6, -1.7, 1.25, 0.35, 0.35, 1.25, -1.7, 2.6},
  { -0.45, -0.4, 0.35, -0.95, -0.95, 0.35, -0.4, -0.45},
  { -0.45, -0.4, 0.35, -0.95, -0.95, 0.35, -0.4, -0.45},
  {2.6, -1.7, 1.25, 0.35, 0.35, 1.25, -1.7, 2.6},
  { -0.65, -3.15, -1.7, -0.4, -0.4, -1.7, -3.15, -0.65},
  {3.35, -0.65, 2.6, -0.45, -0.45, 2.6, -0.65, 3.35}
};

float weight_weight = 0.5;
float canput_weight = 0.5;

bool busy[n_slaves];

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

void print_board(const int* p, const int* o) {
  Serial.println("  a b c d e f g h ");
  for (int i = 0; i < hw; i++) {
    Serial.print(i + 1);
    Serial.print(" ");
    for (int j = hw - 1; j >= 0; j--) {
      if (1 & (p[i] >> j))
        Serial.print("0 ");
      else if (1 & (o[i] >> j))
        Serial.print("1 ");
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
}

float evaluate(const int* me, const int* op, int canput) {
  int me_cnt = 0, op_cnt = 0;
  float weight_me = 0, weight_op = 0;
  int mobility[hw];
  int canput_all = canput;
  for (int i = 0; i < hw; i++) {
    for (int j = 0; j < hw; j++) {
      if (1 & (me[i] >> (hw - 1 - j))) {
        weight_me += weight[i][j];
        me_cnt++;
      } else if (1 & (op[i] >> (hw - 1 - j))) {
        weight_op += weight[i][j];
        op_cnt++;
      }
    }
  }
  check_mobility(me, op, mobility);
  canput_all += pop_count(mobility);
  float weight_proc, canput_proc;
  weight_proc = weight_me / me_cnt - weight_op / op_cnt;
  canput_proc = (float)(canput_all - canput) / max(1, canput_all) - (float)canput / max(1, canput_all);
  return max(-0.999, min(0.999, weight_proc * weight_weight + canput_proc * canput_weight));
}

float end_game(const int* me, const int* op) {
  return (float)(pop_count(me) - pop_count(op));
}

int send_slave(const int* me, const int* op, float alpha, float beta, int i) {
  Wire.beginTransmission(slaves[i]);
  for (int j = 0; j < hw; j++)
    Wire.write(me[j]);
  for (int j = 0; j < hw; j++)
    Wire.write(op[j]);
  Wire.write((int)alpha);
  Wire.write((int)((alpha - (float)((int)alpha)) * 100.0));
  Wire.write((int)beta);
  Wire.write((int)((beta - (float)((int)beta)) * 100.0));
  Wire.endTransmission();
  busy[i] = true;
}

float nega_alpha(const int* me, const int* op, int depth, float alpha, float beta, int skip_cnt, int canput) {
  if (skip_cnt == 2)
    return end_game(me, op);
  /*
    else if (depth == -slave_depth)
    return evaluate(me, op, canput);
  */
  int mobility[hw];
  check_mobility(me, op, mobility);
  int n_canput = pop_count(mobility);
  if (n_canput == 0)
    return nega_alpha(op, me, depth, alpha, beta, skip_cnt + 1, 0);
  int n_me[hw], n_op[hw];
  int pt[hw] = {0, 0, 0, 0, 0, 0, 0, 0};
  float val = -65.0, v;
  if (depth == 0) { //  && n_canput >= 2
    int n_vals = 0;
    int val_idxes[32];
    bool done[32];
    for (int i = 0; i < hw; i++) {
      for (int j = 0; j < hw; j++) {
        if (1 & (mobility[i] >> j)) {
          int use_slave = -1;
          while (use_slave == -1) {
            for (int k = 0; k < n_slaves; k++)
              if (!busy[k])
                use_slave = k;
            if (use_slave != -1)
              break;
            for (int k = 0; k < n_vals; k++) {
              if (done[k])
                continue;
              if (!busy[val_idxes[k]])
                continue;
              Wire.requestFrom(slaves[val_idxes[k]], 3);
              if (Wire.read()) {
                int8_t tmp1 = Wire.read(), tmp2 = Wire.read();
                v = -(float)tmp1 - (float)tmp2 * 0.01;
                busy[val_idxes[k]] = false;
                done[k] = true;
                if (beta <= v) {
                  int cnt = 0;
                  while (cnt < n_vals) {
                    cnt = 0;
                    for (int l = 0; l < n_vals; l++) {
                      if (busy[val_idxes[l]]) {
                        Wire.requestFrom(slaves[val_idxes[l]], 3);
                        if (Wire.read()) {
                          busy[val_idxes[l]] = false;
                          ++cnt;
                        }
                        Wire.read();
                        Wire.read();
                      } else
                        ++cnt;
                    }
                  }
                  return v;
                }
                alpha = max(alpha, v);
                if (val < v)
                  val = v;
              } else {
                Wire.read();
                Wire.read();
              }
            }
          }
          pt[i] = 1 << j;
          move_board(me, op, pt, n_me, n_op);
          pt[i] = 0;
          send_slave(n_op, n_me, -beta, -alpha, use_slave);
          val_idxes[n_vals] = use_slave;
          done[n_vals] = false;
          //Serial.print(use_slave);
          //Serial.println(n_vals);
          ++n_vals;
        }
      }
    }
    //Serial.print(n_vals);
    //Serial.print(" ");
    int cnt = 0;
    while (cnt < n_vals) {
      cnt = 0;
      for (int k = 0; k < n_vals; k++) {
        if (done[k]) {
          ++cnt;
          continue;
        }
        if (!busy[val_idxes[k]])
          continue;
        Wire.requestFrom(slaves[val_idxes[k]], 3);
        if (Wire.read()) {
          int8_t tmp1 = Wire.read(), tmp2 = Wire.read();
          v = -(float)tmp1 - (float)tmp2 * 0.01;
          busy[val_idxes[k]] = false;
          done[k] = true;
          if (beta <= v) {
            int cnt = 0;
            while (cnt < n_vals) {
              cnt = 0;
              for (int l = 0; l < n_vals; l++) {
                if (busy[val_idxes[l]]) {
                  Wire.requestFrom(slaves[val_idxes[l]], 3);
                  if (Wire.read()) {
                    busy[val_idxes[l]] = false;
                    ++cnt;
                  }
                  Wire.read();
                  Wire.read();
                } else
                  ++cnt;
              }
            }
            return v;
          }
          alpha = max(alpha, v);
          if (val < v)
            val = v;
        } else {
          Wire.read();
          Wire.read();
        }
      }
    }
  } else {
    for (int i = 0; i < hw; i++) {
      for (int j = 0; j < hw; j++) {
        if (1 & (mobility[i] >> j)) {
          pt[i] |= 1 << j;
          move_board(me, op, pt, n_me, n_op);
          pt[i] = 0;
          v = -nega_alpha(n_op, n_me, depth - 1, -beta, -alpha, 0, n_canput);
          if (beta <= v)
            return v;
          alpha = max(alpha, v);
          if (val < v)
            val = v;
        }
      }
    }
  }
  return val;
}

void ai(const int* me, const int* op, int* pt) {
  //int dammy[hw] = {0, 0, 0, 0, 0, 0, 0, 0};
  int mobility[hw];
  int rev[hw];
  check_mobility(me, op, mobility);
  int n_canput = pop_count(mobility);
  int n_me[hw], n_op[hw];
  float score, max_score = -65.0;
  int y, x;
  for (int i = 0; i < hw; i++)
    pt[i] = 0;
  for (int i = 0; i < hw; i++) {
    for (int j = 0; j < hw; j++) {
      if (1 & (mobility[i] >> j)) {
        pt[i] |= 1 << j;
        move_board(me, op, pt, n_me, n_op);
        score = -nega_alpha(n_op, n_me, max_depth - slave_depth - 2, max_score, 65.0, 0, n_canput);
        //Serial.print(" ");
        /*
          Serial.print((char)(hw - 1 - j + 'a'));
          Serial.print(i + 1);
          Serial.print(" ");
          Serial.println(score);
        */
        //print_board(n_me, n_op);
        if (max_score < score) {
          max_score = score;
          y = i;
          x = j;
        }
        pt[i] = 0;
      }
    }
  }
  //Serial.print((char)(hw - 1 - x + 'a'));
  //Serial.println(y + 1);
  pt[y] |= 1 << x;
}

void auto_play() {
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
  int stones = 5;
  print_board(black, white);
  while (skip_cnt < 2) {
    if (turn == 0)
      check_mobility(black, white, mobility);
    else
      check_mobility(white, black, mobility);
    if (pop_count(mobility))
      skip_cnt = 0;
    else {
      skip_cnt++;
      turn = 1 - turn;
      continue;
    }
    if (turn == 0) {
      ai(black, white, pt);
      move_board(black, white, pt, black, white);
    } else {
      ai(white, black, pt);
      move_board(white, black, pt, white, black);
    }
    Serial.println(stones);
    print_board(black, white);
    ++stones;
    turn = 1 - turn;
  }
  Serial.print("black(0): ");
  Serial.println(pop_count(black));
  Serial.print("white(1): ");
  Serial.println(pop_count(white));
}

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
    Serial.println(stones);
    print_board(black, white, mobility);
    if (turn == 0) {
      y = -1;
      x = 0;
      while (!(inside(y, x) && mobility[y] & (1 << (hw - 1 - x)))) {
        Serial.println("input a move");
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
      ai(white, black, pt);
      move_board(white, black, pt, white, black);
    }
    ++stones;
    turn = 1 - turn;
  }
  print_board(black, white);
  Serial.print("black(0): ");
  Serial.println(pop_count(black));
  Serial.print("white(1): ");
  Serial.println(pop_count(white));
}

void setup() {
  //long strt = millis();
  for (int i = 0; i < n_slaves; i++)
    busy[i] = false;
  Wire.begin();
  Wire.setClock(400000);
  Serial.begin(9600);
  button.begin(9600);
  pinMode(SDA, INPUT);
  pinMode(SCL, INPUT);
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  pinMode(5, OUTPUT);
  pinMode(DATAPIN1, OUTPUT);
  pinMode(LATCHPIN1, OUTPUT);
  pinMode(CLOCKPIN1, OUTPUT);
  pinMode(DATAPIN2, OUTPUT);
  pinMode(LATCHPIN2, OUTPUT);
  pinMode(CLOCKPIN2, OUTPUT);
  display.setBrightness(0x0f);
  play();
  //Serial.println("done");
  //Serial.println(millis() - strt);
}

void loop() {
  int zero_cnt = 0;
  while (zero_cnt < hw * 4) {
    if (Serial.available()) {
      if ((byte)Serial.read())
        zero_cnt = 0;
      else
        zero_cnt++;
    }
  }
  int pt[hw], me[hw], op[hw];
  for (int i = 0; i < hw; i++)
    me[i] = (int)Serial.read();
  for (int i = 0; i < hw; i++)
    op[i] = (int)Serial.read();
  digitalWrite(5, HIGH);
  ai(me, op, pt);
  digitalWrite(5, LOW);
  for (int i = 0; i < hw * 2; i++)
    Serial.write((byte)0);
  for (int i = 0; i < hw; i++)
    Serial.print(pt[i]);
}

#include <Wire.h>

#define hw 8
#define n_slaves 3

const int slaves[n_slaves] = {8, 9, 10};

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
}

float end_game(const int* me, const int* op) {
  return (float)(pop_count(me) - pop_count(op));
}

int send_slave(const int* me, const int* op, int i) {
  Wire.beginTransmission(slaves[i]);
  for (int j = 0; j < hw; j++)
    Wire.write(me[j]);
  for (int j = 0; j < hw; j++)
    Wire.write(op[j]);
  Wire.endTransmission();
  busy[i] = true;
}

//-------メモリ使用状況表示-------------------------------------------------
// ここから↓
int aRamStart = 0x0100;                   // RAM先頭アドレス（固定値）
int aGvalEnd;                             // グローバル変数領域末尾アドレス
int aHeapEnd;                             // ヒープ領域末尾アドレス(次のヒープ用アドレス）
int aSp;                                  // スタックポインタアドレス（次のスタック用アドレス）
char aBuff[6];
void printMem() {                         // RAM使用状況を表示
  Serial.println();
  Serial.println(F("RAM allocation table"));
  Serial.println(F("usage   start          end           size"));

  Serial.print(F("groval: "));            // 固定アドレス
  printHexDecimal(aRamStart);             // 開始
  Serial.print(F(" - "));
  printHexDecimal(aGvalEnd);              // 終了
  Serial.print("  ");
  printHexDecimal(aGvalEnd - aRamStart + 1); // サイズ
  Serial.println();

  Serial.print(F("heap  : "));            // ヒープ
  printHexDecimal(aGvalEnd + 1);          // 開始
  Serial.print(F(" - "));
  printHexDecimal(aHeapEnd - 1);          // 終了(aHeapEndは次のヒープ用のアドレスなので-1）
  Serial.print(F("  "));
  printHexDecimal(aHeapEnd - 1 - (aGvalEnd + 1) + 1); // サイズ
  Serial.println();

  Serial.print(F("free  : "));            // Free領域
  printHexDecimal(aHeapEnd);              // 開始
  Serial.print(F(" - "));
  printHexDecimal(aSp);                   // 終了
  Serial.print(F("  "));
  printHexDecimal(aSp - aHeapEnd + 1);  // サイズ
  Serial.println();

  Serial.print(F("stack : "));            // スタック領域
  printHexDecimal(aSp + 1);               // 開始（aSpは次のスタック用アドレスなので+1)
  Serial.print(F(" - "));
  printHexDecimal(RAMEND);                // 終了（RAMENDは0x8fffでシステム側で定義）
  Serial.print(F("  "));
  printHexDecimal(RAMEND - (aSp + 1) + 1);    // サイズ
  Serial.println();
  Serial.println();
}

void checkMem() {                         // RAM使用状況を記録
  // 意味不明なところもあるが、そのまま使用
  uint8_t *heapptr, *stackptr;
  stackptr = (uint8_t *)malloc(4);        // とりあえず4バイト確保
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);                         // 確保したメモリを返却
  stackptr =  (uint8_t *)(SP);            // SPの値を保存（SPには次のスタック用の値が入っている）
  aSp = (int)stackptr;                    // スタックポインタの値を記録
  aHeapEnd = (int)heapptr;                // ヒープポインタの値を記録
  aGvalEnd = (int)__malloc_heap_start - 1; // グローバル変数領域の末尾アドレスを記録
}

void printHexDecimal(int x) {             // 引数を16進と10進で表示 0xHHHH(dddd)
  sprintf(aBuff, "%04X", x);              // 16進4桁に変換
  Serial.print(F("0x")); Serial.print(aBuff);
  Serial.print(F("("));
  sprintf(aBuff, "%4d", x);               // 10進4桁に変換
  Serial.print(aBuff); Serial.print(F(")"));
}
//　ここまで↑　コピペ

float nega_alpha(const int* me, const int* op, const int* depth, float alpha, float beta, const int* skip_cnt, const int* canput) {
  if (skip_cnt == 2)
    return end_game(me, op);
  int mobility[hw];
  check_mobility(me, op, mobility);
  int n_canput = pop_count(mobility);
  if (n_canput == 0)
    return nega_alpha(op, me, depth, alpha, beta, skip_cnt + 1, 0);
  int n_me[hw], n_op[hw];
  int pt[hw] = {0, 0, 0, 0, 0, 0, 0, 0};
  float val = -65.0, v;
  int n_vals = 0;
  int val_idxes[32];
  bool done[32];
  if (depth == 0) {
    for (int i = 0; i < hw; i++) {
      for (int j = 0; j < hw; j++) {
        if (1 & (mobility[i] >> j)) {
          int use_slave = -1;
          while (use_slave == -1) {
            for (int k = 0; k < n_vals; k++) {
              if (done[k])
                continue;
              if (!busy[val_idxes[k]])
                continue;
              Wire.requestFrom(slaves[val_idxes[k]], 3);
              //checkMem();
              //printMem();
              if (Wire.read()) {
                v = -(float)Wire.read() - (float)Wire.read() * 0.01;
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
            for (int k = 0; k < n_slaves; k++)
              if (!busy[k])
                use_slave = k;
          }
          pt[i] |= 1 << j;
          move_board(me, op, pt, n_me, n_op);
          pt[i] = 0;
          send_slave(n_op, n_me, use_slave);
          val_idxes[n_vals] = use_slave;
          done[n_vals] = false;
          Serial.print(use_slave);
          //Serial.println(n_vals);
          ++n_vals;
        }
      }
    }
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
          //Serial.print(tmp1);
          //Serial.print(" ");
          //Serial.println(tmp2);
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
        score = -nega_alpha(n_op, n_me, 2, -65.0, 65.0, 0, n_canput);
        Serial.print((char)(hw - 1 - j + 'a'));
        Serial.print(i + 1);
        Serial.print(" ");
        Serial.println(score);
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
  Serial.print((char)(hw - 1 - x + 'a'));
  Serial.println(y + 1);
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
    print_board(black, white);
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
  while (skip_cnt < 2) {
    if (turn == 0)
      check_mobility(black, white, mobility);
    else
      check_mobility(white, black, mobility);
    print_board(black, white, mobility);
    if (pop_count(mobility))
      skip_cnt = 0;
    else {
      Serial.println("Skip!");
      skip_cnt++;
      turn = 1 - turn;
      continue;
    }
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
    turn = 1 - turn;
  }
  print_board(black, white);
  Serial.print("black(0): ");
  Serial.println(pop_count(black));
  Serial.print("white(1): ");
  Serial.println(pop_count(white));
}

void setup() {
  for (int i = 0; i < n_slaves; i++)
    busy[i] = false;
  Wire.begin();
  Wire.setClock(400000);
  pinMode(SDA, INPUT);
  pinMode(SCL, INPUT);
  Serial.begin(115200);
  auto_play();
  Serial.println("done");
}

void loop() {
  // put your main code here, to run repeatedly:
}

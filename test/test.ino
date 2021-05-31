#define hw 8
#define hw2 64

const float weight[hw2] = {
  3.2323232323232323, 0.23088023088023088, 1.3852813852813852, 1.0389610389610389, 1.0389610389610389, 1.3852813852813852, 0.23088023088023088, 3.2323232323232323,
  0.23088023088023088, 0.0, 0.9004329004329005, 0.9004329004329005, 0.9004329004329005, 0.9004329004329005, 0.0, 0.23088023088023088,
  1.3852813852813852, 0.9004329004329005, 1.0389610389610389, 0.9466089466089466, 0.9466089466089466, 1.0389610389610389, 0.9004329004329005, 1.3852813852813852,
  1.0389610389610389, 0.9004329004329005, 0.9466089466089466, 0.9235209235209235, 0.9235209235209235, 0.9466089466089466, 0.9004329004329005, 1.0389610389610389,
  1.0389610389610389, 0.9004329004329005, 0.9466089466089466, 0.9235209235209235, 0.9235209235209235, 0.9466089466089466, 0.9004329004329005, 1.0389610389610389,
  1.3852813852813852, 0.9004329004329005, 1.0389610389610389, 0.9466089466089466, 0.9466089466089466, 1.0389610389610389, 0.9004329004329005, 1.3852813852813852,
  0.23088023088023088, 0.0, 0.9004329004329005, 0.9004329004329005, 0.9004329004329005, 0.9004329004329005, 0.0, 0.23088023088023088,
  3.2323232323232323, 0.23088023088023088, 1.3852813852813852, 1.0389610389610389, 1.0389610389610389, 1.3852813852813852, 0.23088023088023088, 3.2323232323232323
};

float weight_weight = 0.5;
float canput_weight = 0.5;

void print_board(int p[hw], int o[hw]) {
  for (int i = 0; i < hw; i++) {
    for (int j = hw - 1; j >= 0; j--) {
      if (1 & (p[i] >> j))
        Serial.print(1);
      else if (1 & (o[i] >> j))
        Serial.print(2);
      else
        Serial.print(0);
    }
    Serial.println("");
  }
  Serial.println("");
}

int check_mobility_line(int p, int o) {
  int p1 = p << 1;
  return ~(p1 | o) & (p1 + o);
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
      p |= 1 & (me[j] >> (hw - 1 - i));
      o |= 1 & (op[j] >> (hw - 1 - i));
    }
    mobility_tmp = check_mobility_line(p, o);
    mobility_tmp |= reverse(check_mobility_line(reverse(p), reverse(o)));
    for (int j = 0; j < hw; j++)
      mobility[j] |= (1 & (mobility_tmp >> (hw - 1 - j))) << (hw - 1 - i);
  }
  for (int i = 2; i < hw * 2 - 2; i++) {
    p = 0;
    o = 0;
    for (int j = 0; j < hw; j++) {
      p <<= 1;
      o <<= 1;
      if (hw - i + j < 0)
        continue;
      if (hw - i + j >= hw)
        continue;
      p |= (1 & (me[hw - i + j] >> (hw - 1 - j)));
      o |= (1 & (op[hw - i + j] >> (hw - 1 - j)));
    }
    mobility_tmp = check_mobility_line(p, o);
    mobility_tmp |= reverse(check_mobility_line(reverse(p), reverse(o)));
    for (int j = 0; j < hw; j++) {
      if (hw - i + j < 0)
        continue;
      if (hw - i + j >= hw)
        continue;
      mobility[hw - i + j] |= (1 & (mobility_tmp >> (hw - 1 - j))) << (hw - 1 - j);
    }
  }
  for (int i = 2; i < hw * 2 - 2; i++) {
    p = 0;
    o = 0;
    for (int j = 0; j < hw; j++) {
      p <<= 1;
      o <<= 1;
      if (hw - i + j < 0)
        continue;
      if (hw - i + j >= hw)
        continue;
      p |= (1 & (me[j] >> (hw - i + j)));
      o |= (1 & (op[j] >> (hw - i + j)));
    }
    mobility_tmp = check_mobility_line(p, o);
    mobility_tmp |= reverse(check_mobility_line(reverse(p), reverse(o)));
    for (int j = 0; j < hw; j++) {
      if (hw - i + j < 0)
        continue;
      if (hw - i + j >= hw)
        continue;
      mobility[hw - i + j] |= (1 & (mobility_tmp >> (hw - 1 - j))) << (hw - i + j);
    }
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
        res[i] = pt[i - 1] >> 2;
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
        res[i] = pt[i + 1] >> 2;
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
          rev[i] = rev2[i];
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

int pop_count(const int* x){
  int res = 0;
  for (int i = 0; i < hw; i++)
    for (int j = 0; j < hw; j++)
      res += 1 & (x[i] >> j);
  return res;
}

float evaluate(const int* me, const int* op, int canput){
  int me_cnt = 0, op_cnt = 0;
  float weight_me = 0, weight_op = 0;
  int mobility[hw];
  int canput_all = canput;
  for (int i = 0; i < hw; i++){
    for (int j = 0; j < hw; j++){
      if (1 & (me[i] >> (hw - 1 - j))){
        weight_me += weight[i];
        me_cnt++;
      } else if (1 & (op[i] >> (hw - 1 - j))){
        weight_op += weight[i];
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

float end_game(const int* me, const int* op){
  return (float)(pop_count(me) - pop_count(op));
}

float nega_alpha(const int* me, const int* op, int depth, float alpha, float beta, int skip_cnt, int canput){
  if (skip_cnt == 2)
    return end_game(me, op);
  else if(depth == 0)
    return evaluate(me, op, canput);
  int mobility[hw];
  check_mobility(me, op, mobility);
}

void setup() {
  Serial.begin(115200);
  int me[hw] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00010000,
    0b00000000,
    0b00000000,
    0b00000000
  };
  int op[hw] = {
    0b00000000,
    0b00000000,
    0b00001000,
    0b00011000,
    0b00001000,
    0b00000000,
    0b00000000,
    0b00000000
  };
  int dammy[hw] = {0, 0, 0, 0, 0, 0, 0, 0};
  int mobility[hw];
  int rev[hw];
  check_mobility(me, op, mobility);

  int pt[hw] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
  };
  int n_me[hw], n_op[hw];
  int n_canput = pop_count(mobility);
  for (int i = 0; i < hw; i++){
    for (int j = 0; j < hw; j++){
      if (1 & (mobility[i] >> j)){
        pt[i] |= 1 << j;
        move_board(me, op, pt, n_me, n_op);
        Serial.println(-evaluate(n_op, n_me, n_canput));
        print_board(n_me, n_op);
        pt[i] = 0;
      }
    }
  }
  Serial.println("done");
}

void loop() {
  // put your main code here, to run repeatedly:
}

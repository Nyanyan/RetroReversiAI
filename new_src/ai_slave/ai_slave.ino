#include <Wire.h>

#define hw 8
#define hw2 64
#define slave_depth 2
#define score_max 6400

bool waiting = false;
uint64_t in_me, in_op;
int in_alpha, in_beta;
int val1, val2;

const int weight[hw2] = {
  120, -20,  20,   5,   5,  20, -20, 120,
  -20, -40,  -5,  -5,  -5,  -5, -40, -20,
  20,  -5,  15,   3,   3,  15,  -5,  20,
  5,  -5,   3,   3,   3,   3,  -5,   5,
  5,  -5,   3,   3,   3,   3,  -5,   5,
  20,  -5,  15,   3,   3,  15,  -5,  20,
  -20, -40,  -5,  -5,  -5,  -5, -40, -20,
  120, -20,  20,   5,   5,  20, -20, 120
};

bool inside(int y, int x) {
  return 0 <= y && y < hw && 0 <= x && x < hw;
}

inline uint64_t vertical_mirror(uint64_t x) {
  x = ((x >> 8) & 0x00FF00FF00FF00FFULL) | ((x << 8) & 0xFF00FF00FF00FF00ULL);
  x = ((x >> 16) & 0x0000FFFF0000FFFFULL) | ((x << 16) & 0xFFFF0000FFFF0000ULL);
  return ((x >> 32) & 0x00000000FFFFFFFFULL) | ((x << 32) & 0xFFFFFFFF00000000ULL);
}

inline uint64_t horizontal_mirror(uint64_t x) {
  x = ((x >> 1) & 0x5555555555555555ULL) | ((x << 1) & 0xAAAAAAAAAAAAAAAAULL);
  x = ((x >> 2) & 0x3333333333333333ULL) | ((x << 2) & 0xCCCCCCCCCCCCCCCCULL);
  return ((x >> 4) & 0x0F0F0F0F0F0F0F0FULL) | ((x << 4) & 0xF0F0F0F0F0F0F0F0ULL);
}

inline uint64_t black_line_mirror(uint64_t x) {
  uint64_t a = (x ^ (x >> 9)) & 0x0055005500550055ULL;
  x = x ^ a ^ (a << 9);
  a = (x ^ (x >> 18)) & 0x0000333300003333ULL;
  x = x ^ a ^ (a << 18);
  a = (x ^ (x >> 36)) & 0x000000000F0F0F0FULL;
  return x = x ^ a ^ (a << 36);
}

inline uint64_t white_line_mirror(uint64_t x) {
  uint64_t a = (x ^ (x >> 7)) & 0x00AA00AA00AA00AAULL;
  x = x ^ a ^ (a << 7);
  a = (x ^ (x >> 14)) & 0x0000CCCC0000CCCCULL;
  x = x ^ a ^ (a << 14);
  a = (x ^ (x >> 28)) & 0x00000000F0F0F0F0ULL;
  return x = x ^ a ^ (a << 28);
}

inline uint64_t rotate_90(uint64_t x) {
  return vertical_mirror(white_line_mirror(x));
}

inline uint64_t rotate_270(uint64_t x) {
  return vertical_mirror(black_line_mirror(x));
}

inline uint64_t rotate_45(uint64_t x) {
  uint64_t a = (x ^ (x >> 8)) & 0x0055005500550055ULL;
  x = x ^ a ^ (a << 8);
  a = (x ^ (x >> 16)) & 0x0000CC660000CC66ULL;
  x = x ^ a ^ (a << 16);
  a = (x ^ (x >> 32)) & 0x00000000C3E1F078ULL;
  return x ^ a ^ (a << 32);
}

inline uint64_t unrotate_45(uint64_t x) {
  uint64_t a = (x ^ (x >> 32)) & 0x00000000C3E1F078ULL;
  x = x ^ a ^ (a << 32);
  a = (x ^ (x >> 16)) & 0x0000CC660000CC66ULL;
  x = x ^ a ^ (a << 16);
  a = (x ^ (x >> 8)) & 0x0055005500550055ULL;
  return x ^ a ^ (a << 8);
}

inline uint64_t rotate_135(uint64_t x) {
  uint64_t a = (x ^ (x >> 8)) & 0x00AA00AA00AA00AAULL;
  x = x ^ a ^ (a << 8);
  a = (x ^ (x >> 16)) & 0x0000336600003366ULL;
  x = x ^ a ^ (a << 16);
  a = (x ^ (x >> 32)) & 0x00000000C3870F1EULL;
  return x ^ a ^ (a << 32);
}

inline uint64_t unrotate_135(uint64_t x) {
  uint64_t a = (x ^ (x >> 32)) & 0x00000000C3870F1EULL;
  x = x ^ a ^ (a << 32);
  a = (x ^ (x >> 16)) & 0x0000336600003366ULL;
  x = x ^ a ^ (a << 16);
  a = (x ^ (x >> 8)) & 0x00AA00AA00AA00AAULL;
  return x ^ a ^ (a << 8);
}

inline uint64_t calc_legal(const uint64_t me, const uint64_t op) {
  // horizontal
  uint64_t p1 = (me & 0x7F7F7F7F7F7F7F7FULL) << 1;
  uint64_t legal = ~(p1 | op) & (p1 + (op & 0x7F7F7F7F7F7F7F7FULL));
  uint64_t me_proc = horizontal_mirror(me);
  uint64_t op_proc = horizontal_mirror(op);
  p1 = (me_proc & 0x7F7F7F7F7F7F7F7FULL) << 1;
  legal |= horizontal_mirror(~(p1 | op_proc) & (p1 + (op_proc & 0x7F7F7F7F7F7F7F7FULL)));

  // vertical
  me_proc = black_line_mirror(me);
  op_proc = black_line_mirror(op);
  p1 = (me_proc & 0x7F7F7F7F7F7F7F7FULL) << 1;
  uint64_t legal_proc = ~(p1 | op_proc) & (p1 + (op_proc & 0x7F7F7F7F7F7F7F7FULL));
  me_proc = horizontal_mirror(me_proc);
  op_proc = horizontal_mirror(op_proc);
  p1 = (me_proc & 0x7F7F7F7F7F7F7F7FULL) << 1;
  legal_proc |= horizontal_mirror(~(p1 | op_proc) & (p1 + (op_proc & 0x7F7F7F7F7F7F7F7FULL)));
  legal |= black_line_mirror(legal_proc);

  // 45 deg
  me_proc = rotate_45(me);
  op_proc = rotate_45(op);
  p1 = (me_proc & 0x5F6F777B7D7E7F3FULL) << 1;
  legal_proc = ~(p1 | op_proc) & (p1 + (op_proc & 0x5F6F777B7D7E7F3FULL));
  me_proc = horizontal_mirror(me_proc);
  op_proc = horizontal_mirror(op_proc);
  p1 = (me_proc & 0x7D7B776F5F3F7F7EULL) << 1;
  legal_proc |= horizontal_mirror(~(p1 | op_proc) & (p1 + (op_proc & 0x7D7B776F5F3F7F7EULL)));
  legal |= unrotate_45(legal_proc);

  // 135 deg
  me_proc = rotate_135(me);
  op_proc = rotate_135(op);
  p1 = (me_proc & 0x7D7B776F5F3F7F7EULL) << 1;
  legal_proc = ~(p1 | op_proc) & (p1 + (op_proc & 0x7D7B776F5F3F7F7EULL));
  me_proc = horizontal_mirror(me_proc);
  op_proc = horizontal_mirror(op_proc);
  p1 = (me_proc & 0x5F6F777B7D7E7F3FULL) << 1;
  legal_proc |= horizontal_mirror(~(p1 | op_proc) & (p1 + (op_proc & 0x5F6F777B7D7E7F3FULL)));
  legal |= unrotate_135(legal_proc);

  return legal & ~(me | op);
}

inline uint64_t trans(const uint64_t pt, const int k) {
  uint64_t res = 0ULL;
  switch (k) {
    case 0:
      res = (pt << 8) & 0xFFFFFFFFFFFFFF00ULL;
      break;
    case 1:
      res = (pt << 7) & 0x7F7F7F7F7F7F7F00ULL;
      break;
    case 2:
      res = (pt >> 1) & 0x7F7F7F7F7F7F7F7FULL;
      break;
    case 3:
      res = (pt >> 9) & 0x007F7F7F7F7F7F7FULL;
      break;
    case 4:
      res = (pt >> 8) & 0x00FFFFFFFFFFFFFFULL;
      break;
    case 5:
      res = (pt >> 7) & 0x00FEFEFEFEFEFEFEULL;
      break;
    case 6:
      res = (pt << 1) & 0xFEFEFEFEFEFEFEFEULL;
      break;
    case 7:
      res = (pt << 9) & 0xFEFEFEFEFEFEFE00ULL;
      break;
    default:
      res = 0ULL;
  }
  return res;
}

uint64_t calc_flip(const uint64_t me, const uint64_t op, int cell) {
  uint64_t rev = 0ULL;
  uint64_t rev2, mask;
  uint64_t pt = 1ULL << cell;
  for (int k = 0; k < 8; ++k) {
    rev2 = 0ULL;
    mask = trans(pt, k);
    while (mask && (mask & op)) {
      rev2 |= mask;
      mask = trans(mask, k);
    }
    if ((mask & me) != 0) {
      rev |= rev2;
    }
  }
  return rev;
}

inline void flip_do(uint64_t *me, uint64_t *op, uint64_t flip, int pos) {
  *me ^= flip;
  *op ^= flip;
  *me ^= 1ULL << pos;
}

inline void flip_undo(uint64_t *me, uint64_t *op, uint64_t flip, int pos) {
  *me ^= 1ULL << pos;
  *me ^= flip;
  *op ^= flip;
}

inline int pop_count(uint64_t x) {
  x = x - ((x >> 1) & 0x5555555555555555ULL);
  x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
  x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
  x = (x * 0x0101010101010101ULL) >> 56;
  return x;
}

inline int evaluate(const uint64_t me, const uint64_t op) {
  int res = 0;
  for (uint8_t i = 0; i < hw2; ++i) {
    res += weight[i] * (1 & (me >> i));
    res -= weight[i] * (1 & (op >> i));
  }
  return max(-score_max, min(score_max, res));
}

inline int end_game(const uint64_t me, uint64_t op) {
  return 100 * (pop_count(me) - pop_count(op));
}

inline void move_ordering(int places[], int values[], const int siz) {
  int i, j, tmp;
  for (i = 0; i < siz; ++i) {
    for (j = i + 1; j < siz; ++j) {
      if (values[i] < values[j]) {
        tmp = values[i];
        values[i] = values[j];
        values[j] = tmp;
        tmp = places[i];
        places[i] = places[j];
        places[j] = tmp;
      }
    }
  }
}

inline int ntz(uint64_t *x) {
  return pop_count((*x & (-(*x))) - 1);
}

inline int first_bit(uint64_t *x) {
  return ntz(x);
}

inline int next_bit(uint64_t *x) {
  *x &= *x - 1;
  return ntz(x);
}

int nega_alpha(uint64_t me, uint64_t op, int depth, int alpha, int beta, bool skipped) {
  uint64_t legal = calc_legal(me, op);
  const int canput = pop_count(legal);
  if (canput == 0) {
    if (skipped)
      return end_game(me, op);
    return -nega_alpha(op, me, depth, -beta, -alpha, true);
  }
  if (depth == 0) {
    return evaluate(me, op);
  }
  int values[canput], places[canput];
  int i = 0;
  for (int cell = first_bit(&legal); legal; cell = next_bit(&legal)) {
    places[i] = cell;
    values[i] = weight[cell];
    ++i;
  }
  move_ordering(places, values, canput);
  int g;
  uint64_t flip;
  for (i = 0; i < canput; ++i) {
    flip = calc_flip(me, op, places[i]);
    flip_do(&me, &op, flip, places[i]);
    g = -nega_alpha(op, me, depth - 1, -beta, -alpha, false);
    flip_undo(&me, &op, flip, places[i]);
    alpha = max(alpha, g);
    if (beta <= alpha)
      return alpha;
  }
  return alpha;
}

void receive(int num) {
  waiting = false;
  int i;
  int tmp1, tmp2;
  in_me = 0ULL;
  in_op = 0ULL;
  while (Wire.available() < 20);
  for (i = 0; i < hw; ++i)
    in_me |= ((uint64_t)Wire.read()) << (hw * i);
  for (i = 0; i < hw; ++i)
    in_op |= ((uint64_t)Wire.read()) << (hw * i);
  tmp1 = Wire.read();
  tmp2 = Wire.read();
  in_alpha = tmp1 * 256 + tmp2 - score_max;
  tmp1 = Wire.read();
  tmp2 = Wire.read();
  in_beta = tmp1 * 256 + tmp2 - score_max;
  digitalWrite(5, HIGH);
}

void request() {
  Wire.write((int)waiting);
  Wire.write(val1);
  Wire.write(val2);
  if (waiting)
    digitalWrite(5, LOW);
}

void setup() {
  Wire.begin(15);
  Wire.setClock(400000);
  pinMode(SDA, INPUT);
  pinMode(SCL, INPUT);
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  pinMode(5, OUTPUT);
  Wire.onReceive(receive);
  Wire.onRequest(request);
  Serial.begin(115200);
  Serial.println("set up");
}

void loop() {
  if (!waiting) {
    int calculated_val = nega_alpha(in_me, in_op, slave_depth, in_alpha, in_beta, false);
    calculated_val += score_max;
    val1 = calculated_val / 256;
    val2 = calculated_val % 256;
    waiting = true;
  }
}

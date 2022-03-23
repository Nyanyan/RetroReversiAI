#include <Wire.h>
#include <TM1637Display.h>
#include <SoftwareSerial.h>

#define STRENGTH A3

#define lCLK 5
#define lDIO 6
TM1637Display display(lCLK, lDIO);

#define DATAPIN1 2
#define LATCHPIN1 3
#define CLOCKPIN1 4
#define DATAPIN2 9
#define LATCHPIN2 10
#define CLOCKPIN2 11

#define RED 7
#define GREEN 8

SoftwareSerial button(12, 13); // RX, TX

#define hw 8
#define hw2 64
#define n_slaves 8
#define slave_depth 1
#define max_depth 4
#define score_max 6400

const int led_arr_g[64] = {
  62, 61, 60, 59, 58, 57, 56, 63,
  54, 53, 52, 51, 50, 49, 48, 55,
  46, 45, 44, 43, 42, 41, 40, 47,
  38, 37, 36, 35, 34, 33, 32, 39,
  30, 29, 28, 27, 26, 25, 24, 31,
  22, 21, 20, 19, 18, 17, 16, 23,
  14, 13, 12, 11, 10, 9, 8, 15,
  6, 5, 4, 3, 2, 1, 0, 7
};

const int led_arr_r[64] = {
  47, 40, 41, 42, 43, 44, 45, 46,
  55, 48, 49, 50, 51, 52, 53, 54,
  63, 56, 57, 58, 59, 60, 61, 62,
  23, 16, 17, 18, 19, 20, 21, 22,
  31, 24, 25, 26, 27, 28, 29, 30,
  39, 32, 33, 34, 35, 36, 37, 38,
  7, 0, 1, 2, 3, 4, 5, 6,
  15, 8, 9, 10, 11, 12, 13, 14
};

const int slaves[n_slaves] = {8, 9, 10, 11, 12, 13, 14, 15};

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
const int canput_weight = 8;

inline void output_led(int lpin, int dpin, int cpin, bool* arr) {
  digitalWrite(lpin, LOW);
  for (int i = 0; i < hw2; i++) {
    digitalWrite(dpin, arr[i]);
    digitalWrite(cpin, HIGH);
    digitalWrite(cpin, LOW);
  }
  digitalWrite(lpin, HIGH);
}

inline void print_board(const uint64_t p, uint64_t o, const uint64_t m) {
  bool pp[hw2], oo[hw2];
  for (int i = 0; i < hw2; ++i) {
    pp[led_arr_r[i]] = 1 & (p >> (hw2 - 1 - i));
    pp[led_arr_r[i]] = 1 & (m >> (hw2 - 1 - i));
    oo[led_arr_g[i]] = 1 & (o >> (hw2 - 1 - i));
    oo[led_arr_g[i]] = 1 & (m >> (hw2 - 1 - i));
  }
  output_led(LATCHPIN1, DATAPIN1, CLOCKPIN1, pp);
  output_led(LATCHPIN2, DATAPIN2, CLOCKPIN2, oo);
  int num = pop_count(p) * 100 + pop_count(o);
  display.showNumberDecEx(num, 0x40, true);
}

inline void print_board(const uint64_t p, const uint64_t o) {
  bool pp[hw2], oo[hw2];
  for (int i = 0; i < hw2; ++i) {
    pp[led_arr_r[i]] = 1 & (p >> (hw2 - 1 - i));
    oo[led_arr_g[i]] = 1 & (o >> (hw2 - 1 - i));
  }
  output_led(LATCHPIN1, DATAPIN1, CLOCKPIN1, pp);
  output_led(LATCHPIN2, DATAPIN2, CLOCKPIN2, oo);
  int num = pop_count(p) * 100 + pop_count(o);
  display.showNumberDecEx(num, 0x40, true);
}

inline void print_serial_board(const uint64_t p, const uint64_t o, const uint64_t m) {
  Serial.println("  a b c d e f g h ");
  for (int i = 0; i < hw; ++i) {
    Serial.print(i + 1);
    Serial.print(" ");
    for (int j = 0; j < hw; ++j) {
      if (1 & (p >> (i * hw + j)))
        Serial.print("0 ");
      else if (1 & (o >> (i * hw + j)))
        Serial.print("1 ");
      else if (1 & (m >> (i * hw + j)))
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
  uint64_t legal = ~(p1 + op) & (p1 + (op & 0x7F7F7F7F7F7F7F7FULL));
  uint64_t me_proc = horizontal_mirror(me);
  uint64_t op_proc = horizontal_mirror(op);
  p1 = (me_proc & 0x7F7F7F7F7F7F7F7FULL) << 1;
  legal |= horizontal_mirror(~(p1 + op_proc) & (p1 + (op_proc & 0x7F7F7F7F7F7F7F7FULL)));

  // vertical
  me_proc = black_line_mirror(me);
  op_proc = black_line_mirror(op);
  p1 = (me_proc & 0x7F7F7F7F7F7F7F7FULL) << 1;
  uint64_t legal_proc = ~(p1 + op_proc) & (p1 + (op_proc & 0x7F7F7F7F7F7F7F7FULL));
  me_proc = horizontal_mirror(me_proc);
  op_proc = horizontal_mirror(op_proc);
  p1 = (me_proc & 0x7F7F7F7F7F7F7F7FULL) << 1;
  legal_proc |= horizontal_mirror(~(p1 + op_proc) & (p1 + (op_proc & 0x7F7F7F7F7F7F7F7FULL)));
  legal |= black_line_mirror(legal_proc);

  // 45 deg
  me_proc = rotate_45(me);
  op_proc = rotate_45(op);
  p1 = (me_proc & 0x5F6F777B7D7E7F3FULL) << 1;
  legal_proc = ~(p1 + op_proc) & (p1 + (op_proc & 0x5F6F777B7D7E7F3FULL));
  me_proc = horizontal_mirror(me_proc);
  op_proc = horizontal_mirror(op_proc);
  p1 = (me_proc & 0x7D7B776F5F3F7F7EULL) << 1;
  legal_proc |= horizontal_mirror(~(p1 + op_proc) & (p1 + (op_proc & 0x7D7B776F5F3F7F7EULL)));
  legal |= unrotate_45(legal_proc);

  // 135 deg
  me_proc = rotate_135(me);
  op_proc = rotate_135(op);
  p1 = (me_proc & 0x7D7B776F5F3F7F7EULL) << 1;
  legal_proc = ~(p1 + op_proc) & (p1 + (op_proc & 0x7D7B776F5F3F7F7EULL));
  me_proc = horizontal_mirror(me_proc);
  op_proc = horizontal_mirror(op_proc);
  p1 = (me_proc & 0x5F6F777B7D7E7F3FULL) << 1;
  legal_proc |= horizontal_mirror(~(p1 + op_proc) & (p1 + (op_proc & 0x5F6F777B7D7E7F3FULL)));
  legal |= unrotate_135(legal_proc);

  return legal & ~(me | op);
}

inline uint64_t trans(const uint64_t pt, const int k) {
  uint64_t res = 0ULL;
  switch (k) {
    case 0:
      res = (pt << 8) & 0xffffffffffffff00ULL;
    case 1:
      res = (pt << 7) & 0x7f7f7f7f7f7f7f00ULL;
    case 2:
      res = (pt >> 1) & 0x7f7f7f7f7f7f7f7fULL;
    case 3:
      res = (pt >> 9) & 0x007f7f7f7f7f7f7fULL;
    case 4:
      res = (pt >> 8) & 0x00ffffffffffffffULL;
    case 5:
      res = (pt >> 7) & 0x00fefefefefefefeULL;
    case 6:
      res = (pt << 1) & 0xfefefefefefefefeULL;
    case 7:
      res = (pt << 9) & 0xfefefefefefefe00ULL;
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
    while ((mask != 0) && (mask & op) != 0) {
      rev2 |= mask;
      mask = trans(mask, k);
    }
    if ((mask & me) != 0) {
      rev |= rev2;
    }
  }
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
  const int me_cnt = pop_count(me), op_cnt = pop_count(op);
  float weight_me = 0, weight_op = 0;
  uint64_t mobility;
  for (int i = 0; i < hw2; ++i) {
    weight_me += weight[i] * (1 & me >> i);
    weight_op += weight[i] * (1 & op >> i);
  }
  int weight_proc;
  weight_proc = weight_me / max(1, me_cnt) - weight_op / max(1, op_cnt);
  return max(-score_max, min(score_max, weight_proc));
}

inline int end_game(const uint64_t me, uint64_t op) {
  return 1000 * (pop_count(me) - pop_count(op));
}

inline void move_ordering(uint64_t flips[], int places[], int values[], const int siz) {
  int i, j, tmp;
  uint64_t tmp_flip;
  for (i = 0; i < siz; ++i) {
    for (j = i + 1; j < siz; ++j) {
      if (values[i] < values[j]) {
        tmp = values[i];
        values[i] = values[j];
        values[j] = tmp;
        tmp_flip = flips[i];
        flips[i] = flips[j];
        flips[j] = tmp_flip;
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

int nega_alpha(const uint64_t me, const uint64_t op, int depth, int alpha, int beta, bool skipped) {
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
  uint64_t flips[canput];
  int values[canput], places[canput];
  int i = 0;
  for (int cell = first_bit(&legal); legal; cell = next_bit(&legal)) {
    places[i] = cell;
    flips[i] = calc_flip(me, op, cell);
    flip_do(&me, &op, flips[i], cell);
    values[i] = evaluate(me, op);
    flip_undo(&me, &op, flips[i], cell);
    ++i;
  }
  move_ordering(flips, places, values, canput);
  int g;
  for (i = 0; i < canput; ++i) {
    flip_do(&me, &op, flips[i], places[i]);
    g = -nega_alpha(op, me, depth - 1, -beta, -alpha, false);
    flip_undo(&me, &op, flips[i], places[i]);
    alpha = max(alpha, g);
  }
  return alpha;
}

inline int ai(const uint64_t me, const uint64_t op) {
  uint64_t legal = calc_legal(me, op);
  const int canput = pop_count(legal);
  uint64_t flips[canput];
  int values[canput], places[canput];
  int i = 0;
  for (int cell = first_bit(&legal); legal; cell = next_bit(&legal)) {
    places[i] = cell;
    flips[i] = calc_flip(me, op, cell);
    flip_do(&me, &op, flips[i], cell);
    values[i] = evaluate(me, op);
    flip_undo(&me, &op, flips[i], cell);
    ++i;
  }
  move_ordering(flips, places, values, canput);
  int g, alpha = -score_max, policy_idx = 0;
  for (i = 0; i < canput; ++i) {
    flip_do(&me, &op, flips[i], places[i]);
    g = -nega_alpha(op, me, max_depth, -score_max, -alpha, false);
    flip_undo(&me, &op, flips[i], places[i]);
    if (alpha < g) {
      alpha = g;
      policy_idx = i;
    }
  }
  return places[policy_idx];
}

void play() {
  // black: red white: green
  uint64_t black = 0b0000000000000000000000000000100000010000000000000000000000000000;
  uint64_t white = 0b0000000000000000000000000001000000100000000000000000000000000000;
  bool skipped = false;
  uint64_t legal, flip;
  int turn = 0;
  int y, x, place;
  int stones = 5;
  while (true) {
    if (turn == 0)
      legal = calc_legal(black, white);
    else
      legal = calc_legal(white, black);
    if (pop_count(legal)) {
      skipped = false;
      Serial.println(stones);
      print_serial_board(black, white, legal);
      if (turn == 0) {
        digitalWrite(RED, HIGH);
        y = -1;
        x = 0;
        while (!(inside(y, x) && legal & (1 << (hw - 1 - y * hw - x)))) {
          button.listen();
          while (button.available())
            button.read();
          button.write((byte)0);
          while (button.available() < 2) {
            print_board(black, white, legal);
            delay(100);
            print_board(black, white);
            delay(100);
          }
          y = (int)button.read();
          x = (int)button.read();
          Serial.print((char)(x + 'a'));
          Serial.println(y + 1);
        }
        flip = calc_flip(black, white, hw - 1 - (y * hw + x));
        flip_do(&black, &white, flip, hw - 1 - (y * hw + x));
        digitalWrite(RED, LOW);
      } else {
        digitalWrite(GREEN, HIGH);
        print_board(black, white);
        place = ai(white, black);
        flip = calc_flip(white, black, place);
        flip_do(&white, &black, flip, place);
        digitalWrite(GREEN, LOW);
      }
      ++stones;
      turn = 1 - turn;
    } else {
      Serial.println("Skip!");
      if (skipped)
        break;
      skipped = true;
      turn = 1 - turn;
      continue;
    }
  }
  print_board(black, white);
  Serial.print("black(0): ");
  Serial.println(pop_count(black));
  Serial.print("white(1): ");
  Serial.println(pop_count(white));
}

void setup() {
  Serial.begin(115200);
  button.begin(1200);
  pinMode(SDA, INPUT);
  pinMode(SCL, INPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(DATAPIN1, OUTPUT);
  pinMode(LATCHPIN1, OUTPUT);
  pinMode(CLOCKPIN1, OUTPUT);
  pinMode(DATAPIN2, OUTPUT);
  pinMode(LATCHPIN2, OUTPUT);
  pinMode(CLOCKPIN2, OUTPUT);
  pinMode(STRENGTH, INPUT);
  display.setBrightness(0x0f);
  bool arr1[hw2], arr2[hw2];
  for (int i = 0; i < hw2; i++) {
    arr1[i] = true;
    arr2[i] = false;
  }
  output_led(LATCHPIN1, DATAPIN1, CLOCKPIN1, arr1);
  output_led(LATCHPIN2, DATAPIN2, CLOCKPIN2, arr2);
  delay(1000);
  output_led(LATCHPIN1, DATAPIN1, CLOCKPIN1, arr2);
  output_led(LATCHPIN2, DATAPIN2, CLOCKPIN2, arr1);
  delay(1000);
  output_led(LATCHPIN2, DATAPIN2, CLOCKPIN2, arr2);
  Serial.println("set up");
  play();
}

void loop() {
}

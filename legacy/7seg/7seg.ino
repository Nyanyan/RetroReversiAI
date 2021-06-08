/* *********************************************************
   Simple roll-your-own TM1637 operations without library
      Sample sketch V00  July 25, 2020 (c) Akira Tominaga 
 *  ********************************************************/
#define lDIO 6                // DIO for TM1637 LED
#define lCLK 5                // CLK for TM1637 LED
#define lBrt 0x07             // duty-r 1/16x 02:4,03:10,05:12,07:14
#define lTu 50                // time unit in micro-second
byte lChr;                    // single byte sent to LED
byte Data[] = { 0x00, 0x00, 0x00, 0x00 }; // LED initial value 0000

void setup() { // ***** Arduino setup *****
  pinMode(lCLK, OUTPUT);
  pinMode(lDIO, OUTPUT);
  digitalWrite(lCLK, HIGH);
  digitalWrite(lDIO, HIGH);
  lDispData();                // display 0000
  delay(1000);
}

void loop() { // ****** Arduino Loop *****
  // example 
  for (int k = 0; k < 4; k++) {
    uint8_t Rand = random(0, 15);
    Data[k] = Rand;
  }
  lDispData();
  delay(1000);
}

/**************************************
      User defined functions
* *********************************** */
// *** lDispData *** display data to 4dig LED
void lDispData(void) {
#define showDat 0x40            // show this is data
#define showAd0 0xC0            // LED data addr is zero
#define showDcB 0x88+lBrt       // show dCtl + brightness
  lStart();                     // start signal
  lChr = showDat;               // identifier for data
  lSend();                      // send it
  lStop();                      // stop signal
  lStart();                     // and restart
  lChr = showAd0;               // identifier for address
  lSend();                      // send it
  for (int j = 0; j < 4; j++) { // for Data[0] to Data[3]
    byte edChr = Data[j];       // set a byte to edChr for edit
    switch (edChr) { 
      case 0x00: lChr = 0x3F; break; // 0
      case 0x01: lChr = 0x06; break; // 1
      case 0x02: lChr = 0x5B; break; // 2
      case 0x03: lChr = 0x4F; break; // 3
      case 0x04: lChr = 0x66; break; // 4
      case 0x05: lChr = 0x6D; break; // 5
      case 0x06: lChr = 0x7D; break; // 6
      case 0x07: lChr = 0x07; break; // 7
      case 0x08: lChr = 0x7F; break; // 8
      case 0x09: lChr = 0x6F; break; // 9
      case 0x0A: lChr = 0x77; break; // A
      case 0x0B: lChr = 0x7C; break; // b
      case 0x0C: lChr = 0x39; break; // C
      case 0x0D: lChr = 0x5E; break; // d
      case 0x0E: lChr = 0x79; break; // E
      case 0x0F: lChr = 0x71; break; // F
      case 0x10: lChr = 0x00; break; // blank
      default:   lChr = 0x7B;        // e for error
    }
    lSend();                    // send each byte continuously
  }                             // end of four bytes
  lStop();                      // stop signal
  lStart();                     // restart
  lChr = showDcB;               // identifier for display brightness
  lSend();                      // send it
  lStop();                      // stop signal
}

// *** lSend *** send a charater in lChr
void lSend(void) {  
#define LSb B00000001           // Least Significant bit
  for (int i = 0; i < 8; i++) { // do the following for 8bits
    if ((lChr & LSb) == LSb) {  // if one then
      digitalWrite(lDIO, HIGH); // set lDIO high
    } else {                    // else
      digitalWrite(lDIO, LOW);  // set lDIO low
    }
    lCLKon();                   // clock on to show lDIO
    lChr = lChr >> 1;           // shift bits to right
  }
  digitalWrite(lDIO, LOW);      // pull down during Ack
  lCLKon();                     // clock on to simulate reading
}

// *** lStart *** send start signal 
void lStart(void) {
  digitalWrite(lDIO, LOW);
  delayMicroseconds(lTu);
  digitalWrite(lCLK, LOW);
  delayMicroseconds(lTu);
}

// *** lStop *** send stop signal
void lStop(void) {
  digitalWrite(lCLK, HIGH);
  delayMicroseconds(lTu);
  digitalWrite(lDIO, HIGH);
  delayMicroseconds(lTu);
}

// *** lCLKon ** clock on to show data
void lCLKon(void) {
  digitalWrite(lCLK, HIGH);
  delayMicroseconds(lTu);
  digitalWrite(lCLK, LOW);
  delayMicroseconds(lTu);
}
/* End of program */

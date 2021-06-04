#include <TM1637Display.h>
 
// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3
TM1637Display display(CLK, DIO);
 
void setup() {
    display.setBrightness(0x0f);
}
 
void loop() {
    display.showNumberDecEx(1234,0x40,true);
    delay(500);
    display.showNumberDecEx(5678,0x00,true);
    delay(500);
}

/*


  Karl Michel Koerich - 260870321
  
  Task: Design a simple digital timer, using either of the two timer functions mentioned above,
  that uses a counting algorithm to count down from
  10 minutes and display the remaining time on the serial monitor in an mm:ss format. 
  
  Based off example Blink without Delay.


*/

#include <Arduino.h>
#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
int seconds = 0;
int minutes = 10;

// constant won't change
const long interval = 1000;    // interval at which to change time

void setup() {
  u8x8.begin(); // to start using the screen
  u8x8.setFlipMode(1);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  Serial.begin(9600);
  u8x8.setCursor(0,0);
  Serial.println("Time: 10:00");
  u8x8.print("10:00");
}

void loop() {
  
  while (!(minutes == 0 && seconds == 0)) {

    u8x8.setCursor(0,0);
    currentMillis = millis(); // read current time

    // decrement time if the defined time interval has passed
    // since the last change time it decremented
    if (currentMillis - previousMillis >= interval) {

      // print zeros since minute values are smaller than 10
      Serial.print("Time: 0");
      u8x8.print("0");

      // checks if seconds is 0 to reset it and decrement minutes
      if (seconds == 0) {
        minutes = minutes - 1;
        seconds = 59;
      }
      else {
        seconds = seconds - 1;
      }

      
      // display in the format mm:ss
      Serial.print(minutes); Serial.print(":");
      u8x8.print(minutes); u8x8.print(":");
  
      if (seconds < 10) {
        Serial.print("0");
        u8x8.print("0");
      }
      Serial.println(seconds);
      u8x8.print(seconds);

      // update previousMillis with the current time elapsed
      previousMillis = currentMillis;
      
    }
  }
}

/*


  Karl Michel Koerich - 260870321

  Task: Design the alarm clock in Arduino using switch cases.
  We recommend creating functions to organize your code.

  Code from prelab_task.ino and Lab 1.


*/

#include <Arduino.h>
#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

// pins
int buttonPin = 3;   // pushbutton connected to D3
int potPin = A1;      // potentiometer connected to A1
int buzzerPin = 5;    // buzzer connected to D5

// button values: 0 not pressed, 1 while pressed
int buttonRead = 0;
int previousButtonRead = 0;
int buttonAction = 0;

// to store time last read and current time elapsed
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

// initial arbitrary time
int minutes = 57;
int hours = 22;
int seconds = 0;

// initial artibrary alarm time
int alarmMinutes = 34;
int alarmHours = 16;
int alarmSeconds = 0;

// time constants
const int ONE_SECOND = 1000;      // 1 second in milliseconds
const int HALF_SECOND = 500;      // 1/2 second in milliseconds
const int QUARTER_SECOND = 250;
const int FIVE_SECONDS = 5000;    // 5 seconds in milliseconds

// state constants
const int NORMAL = 0;
const int CHANGE_ALARM = 1;
const int CHANGE_CLOCK = 2;
const int ALARM = 3;
const int SNOOZE = 4;

// type of click constants
const int NO_CLICK = 0;
const int SINGLE_CLICK = 1;
const int DOUBLE_CLICK = 2;
const int HOLD = 3;
const int THREE_IN_FIVE = 4;

// type alarm
const int ON = 1;
const int OFF = 0;
const int High = 1;
const int Low = 0;

// variables that control clicks and releases
int nClicks = 0;
int nReleases = 0;
unsigned long timeFirstClick = 0;

// variables that control buzz and blink
int alarm = OFF; // controls if buzz should be ON
int blinking = OFF; // controls if blinking should be ON
int alarmBuzz = Low;
int alarmBlink = Low;
unsigned long timeLastAlarm = 0;

// current state
int state = NORMAL;

// setup before starting main loop
void setup() {

  // put your setup code here, to run once:
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(potPin, INPUT);
  u8x8.begin(); // to start using the screen
  u8x8.setFlipMode(1);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  Serial.begin(9600);
  display_normal();
}

// function that makes the alarm buzz and blink at 1 Hz, 50% duty cycle
void sound_alarm() {

  currentMillis = millis(); // get current time elapsed
  if (alarm == ON) {
    // turns on buzz if off for 1/2 second or more
    if (alarmBuzz == Low && currentMillis - timeLastAlarm >= HALF_SECOND) {
      tone(buzzerPin, 100); // set the voltage to high and makes a noise
      alarmBuzz = High;
    } // turns off buzz if on for 1/2 second or more
    else if (alarmBuzz == High && currentMillis - timeLastAlarm >= HALF_SECOND) {
      noTone(buzzerPin); // sets the voltage to low and makes no noise
      alarmBuzz = Low;
    }
  }
  else if (alarm == OFF) { // sets the voltage to low when snooze
    noTone(buzzerPin);
  }

  if (blinking == ON) {
    // turns on blink if off for 1/2 second or more
    if (alarmBlink == Low && currentMillis - timeLastAlarm >= HALF_SECOND) {
      u8x8.setCursor(0, 0); // lights up screen with 0s
      for (int i = 0; i < 8; i = i + 1) {
        u8x8.println("000000000000000");
      }
      alarmBlink = High;
      timeLastAlarm = currentMillis;
    } // turns off blink if on for 1/2 second or more
    else if (alarmBlink == High && currentMillis - timeLastAlarm >= HALF_SECOND) {
      u8x8.clear(); // clears screen
      alarmBlink = Low;
      timeLastAlarm = currentMillis;
    }
  }
  else if (blinking == OFF) { // resets blinking start time and clear screen
    u8x8.clear();
    unsigned long timeLastAlarm = 0;
  }

}

// change alarm time variables using rotary pot
void change_alarm() {

  int potValue = analogRead(potPin);
  potValue = potValue * 1.40625;               // (24*60)/(1024) => Each pot value represents 1.40625 minutes
  alarmMinutes = (int)potValue % 60;           // minutes is the mod 60
  alarmHours = (potValue - alarmMinutes) / 60; // hours is division by 60
}

// change clock time variables using rotary pot
void change_clock() {

  int potValue = analogRead(potPin);
  potValue = potValue * 1.40625;       // (24*60)/(1024) => Each pot value represents 1.40625 minutes
  minutes = (int)potValue % 60;        // minutes is the mod 60
  hours = (potValue - minutes) / 60;   // hours is division by 60
  seconds = 0;                         // reset seconds to 0

  // restart clock from the time defined in this function
  previousMillis = millis();
}

// displays screen with alarm time and current time
void display_normal() {

  u8x8.setCursor(0, 0); // alarm
  u8x8.print("Alarm:");
  display_time(alarmHours, alarmMinutes);
  u8x8.setCursor(0, 3); // clock
  u8x8.print("Clock:");
  display_time(hours, minutes);
}

void display_time(int h, int m) {

  if (h < 10) {
    u8x8.print("0");
  }
  u8x8.print(h); u8x8.print(":");
  if (m < 10) {
    u8x8.print("0");
  }
  u8x8.println(m);
}

// update clock every minute
void update_clock() {

  currentMillis = millis();

  if (currentMillis - previousMillis >= ONE_SECOND) {

    if (seconds == 59) {
      if (minutes == 59) {
        if (hours == 23) {
          hours = 0;
        }
        else {
          hours = hours + 1;
        }
        minutes = 0;
      }
      else {
        minutes = minutes + 1;
      }
      seconds = 0;
    }
    else {
      seconds = seconds + 1;
    }

    previousMillis = currentMillis;
  }
}

// checks for button action when the alarm is not sounding
int button_action_not_alarm() {

  // returns NO_CLICK if none of the conditions below are satisfied
  int returnOption = NO_CLICK;

  previousButtonRead = buttonRead;        // saves previous button value (0 or 1)
  buttonRead = digitalRead(buttonPin);    // read the button pin: if>0, then pressed

  // detect a click and save time if it's a first click
  if (buttonRead == 1 && previousButtonRead == 0) {
    if (nClicks == 0) {
      timeFirstClick = millis(); // get time elapsed at the moment of a first click
    }
    nClicks = nClicks + 1;
    Serial.print("nClicks: "); Serial.print(nClicks); Serial.print(" - nReleases: "); Serial.println(nReleases);
  }
  // detect if the button was released
  else if (buttonRead == 0 && previousButtonRead == 1) {
    nReleases = nReleases + 1;
    Serial.print("nClicks: "); Serial.print(nClicks); Serial.print(" - nReleases: "); Serial.println(nReleases);
  }

  currentMillis = millis(); // Get current time elapsed

  // if there is only 1 click in at least 1/2 second
  if (nClicks == 1 && nReleases == 1 && currentMillis - timeFirstClick >= HALF_SECOND) {
    returnOption = SINGLE_CLICK;
    nClicks = 0;
    nReleases = 0;
  }

  // if there are 2 clicks in at least 1/2 second
  else if (nClicks == 2 && nReleases == 2 && currentMillis - timeFirstClick >= HALF_SECOND) {
    returnOption = DOUBLE_CLICK;
    nClicks = 0;
    nReleases = 0;
  }

  // if there is 1 click and 0 releases in one second (hold for 1 second)
  else if (nReleases == 0 && nClicks == 1 && currentMillis - timeFirstClick >= ONE_SECOND) {
    returnOption = HOLD;
    nClicks = 0;
    nReleases = 0;
  }

  // reset if more releases than nClicks, or if its been too long since first click
  else if (nReleases > nClicks || currentMillis - timeFirstClick >= ONE_SECOND) {
    nClicks = 0;
    nReleases = 0;
  }

  return returnOption;
}


// checks for button action when the alarm is sounding
int button_action_alarm() {

  int returnOption = NO_CLICK;

  previousButtonRead = buttonRead;
  buttonRead = digitalRead(buttonPin);    // read the button pin: if>0, then pressed

  // detect a click and save time if it's a first click
  if (buttonRead == 1 && previousButtonRead == 0) {
    if (nClicks == 0) {
      timeFirstClick = millis(); // get time elapsed at the moment of a first click
    }
    nClicks = nClicks + 1;
    Serial.print("nClicks: "); Serial.print(nClicks); Serial.print(" - nReleases: "); Serial.println(nReleases);
  }
  // detect if the button was released
  else if (buttonRead == 0 && previousButtonRead == 1) {
    nReleases = nReleases + 1;
    Serial.print("nClicks: "); Serial.print(nClicks); Serial.print(" - nReleases: "); Serial.println(nReleases);
  }

  currentMillis = millis(); // Get current time elapsed

  // if there is only 1 click in less than 1/2 second
  // this check ensured that we alaways transition from ALARM to SNOOZE if the button is pressed
  if (nClicks == 1 && nReleases == 1 && (currentMillis - timeFirstClick <= QUARTER_SECOND)) { 
    returnOption = SINGLE_CLICK;
    // we do not update nClicks or nReleases because if the user presses 2 more times
    // before 5 seconds of the first click, we transition to NORMAl state.
  }

  // if there are 3 clicks in less than 5 seconds
  else if (nClicks == 3 && nReleases == 3 && (currentMillis - timeFirstClick <= FIVE_SECONDS)) {
    returnOption = THREE_IN_FIVE;
    nClicks = 0;
    nReleases = 0;
  }

  // reset if more releases than nClicks, or if its been 5 seconds since first click
  if (nReleases > nClicks || currentMillis - timeFirstClick >= FIVE_SECONDS) {
    nClicks = 0;
    nReleases = 0;
  }

  return returnOption;
}


// main program loop
void loop() {

  // controls and execute functions according to current state
  switch (state) {

    case NORMAL:
      buttonAction = button_action_not_alarm();
      // transition to ALARM when time is the same as alarm
      if (hours == alarmHours && minutes == alarmMinutes && seconds == alarmSeconds) {
        Serial.println("NORMAL --> ALARM");
        alarm = ON;
        blinking = ON;
        state = ALARM;
      }
      else if (buttonAction == SINGLE_CLICK) {
        Serial.println("NORMAL --> CHANGE_ALARM");
        state = CHANGE_ALARM;
      }
      else if (buttonAction == HOLD) {
        Serial.println("NORMAL --> CHANGE_CLOCK");
        state = CHANGE_CLOCK;
      }
      break;

    case CHANGE_ALARM:
      change_alarm();
      buttonAction = button_action_not_alarm();
      if (buttonAction == SINGLE_CLICK) {
        Serial.println("CHANGE_ALARM --> NORMAL");
        state = NORMAL;
      }
      break;

    case CHANGE_CLOCK:
      change_clock();
      buttonAction = button_action_not_alarm();
      if (buttonAction == DOUBLE_CLICK) {
        Serial.println("CHANGE_CLOCK --> NORMAL");
        state = NORMAL;
      }
      break;

    case ALARM:
      sound_alarm();
      buttonAction = button_action_alarm();
      if (buttonAction == SINGLE_CLICK) {
        Serial.println("ALARM --> SNOOZE");
        // call function one more time to make sure buzz is off
        alarm = OFF;
        sound_alarm();
        state = SNOOZE;
      }
      break;

    case SNOOZE:
      sound_alarm();
      buttonAction = button_action_alarm();
      if (buttonAction == THREE_IN_FIVE) {
        Serial.println("SNOOZE --> NORMAL");
        // call function one more time to make sure blinking is off
        blinking = OFF;
        sound_alarm();
        state = NORMAL;
      }
      break;

    default:
      Serial.println("Error: The program should not enter this case.");
      break;
  }

  update_clock(); // always update the clock, no matter what.
  display_normal(); // always display the current alarm and time

}

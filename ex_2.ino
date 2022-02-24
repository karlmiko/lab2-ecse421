/*

  Karl Michel Koerich - 260870321

  Task: Using your Arduino and available sensors, design the controller. As the
  train approaches, the approach sensor will detect the headlights from the train.
  When the train is leaving, it will honk a horn to signal that it is departing.

*/

#include <Arduino.h>
#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

// pins
int LEDPin = 2;      // LED connected to D2
int soundPin = A0;    // sound connected to A2
int lightPin = A3;    // light connected to A3

// time constants
const int ONE_SECOND = 1000;      // 1 second in milliseconds
const int two50_MS = 250;         // 250 milliseconds

// controller state constants
const int GATES_OPEN = 0;
const int GATES_LOWERING = 1;
const int GATES_DOWN = 2;
const int GATES_OPENING = 3;
// train state constants
const int FAR = 0;
const int CLOSE = 1;
// traffic light state constants
const int NOT_BLINKING = 0;
const int BLINKING = 1;
// gate state constants
const int UP = 0;
const int DOWN = 1;

// constants
const int APPROACH = 0;
const int EXIT = 1;
const int OFF = 0;
const int ON = 1;
const int NO = 0;
const int YES = 1;

// starting signals
int trainStatus = EXIT;
int blinking = OFF;
int fiveSeconds = NO;
int goDown = OFF;
int goUp = OFF;

// starting states
int controller_state = GATES_OPEN;
int train_state = FAR;
int trafficLight_state = NOT_BLINKING;
int gate_state = UP;
int isFar = 0;
int isClose = 0;

// time variables
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long timeLastBlink = 0;
int seconds = 5;

// sound variables
const int low_sound_boundary = 10;  // low value to decide when the detected sound is a train
const int high_sound_boundary = 90;  // high value to decide when the detected sound is a train
int soundReading = 0;      // variable to store the value read from the sonud sensor pin

// light variables
const int light_threshold = 745;  // threshold value to decide when gates should close
int lightReading = 0;      // variable to store the value read from the sensor pin

// LED variables
int blinkStatus = LOW;

// setup before starting main loop
void setup() {

  // put your setup code here, to run once:
  pinMode(LEDPin, OUTPUT);
  pinMode(soundPin, INPUT);
  pinMode(lightPin, INPUT);
  u8x8.begin(); // to start using the screen
  u8x8.setFlipMode(1);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  Serial.begin(9600);
  u8x8.setCursor(0,0);
  u8x8.println("CONTROL OPEN");
  u8x8.println("TRAIN IS FAR");
  u8x8.println("NOT BLINKING");
  u8x8.println("GATES OPEN");
}

// function that makes the light blink at 1 Hz, 50% duty cycle
void do_blinking() {

  currentMillis = millis(); // get current time elapsed

  if (blinking == ON) {
    // turns off blink if on for 1/4 second or more
    if (blinkStatus == HIGH && currentMillis - timeLastBlink >= two50_MS) {
      digitalWrite(LEDPin, LOW);
      blinkStatus = LOW;
      timeLastBlink = currentMillis;
    } // turns on blink if off for 1/4 second or more
    else if (blinkStatus == LOW && currentMillis - timeLastBlink >= two50_MS) {
      digitalWrite(LEDPin, HIGH);
      blinkStatus = HIGH;
      timeLastBlink = currentMillis;
    }
  }
  else if (blinking == OFF) { // resets blinking start time and clear LED
    digitalWrite(LEDPin, LOW);
    timeLastBlink = 0;
  }

}

// checks for button action when the alarm is sounding
int is_five_seconds() {

  int returnOption = NO;

  if (previousMillis != 0) {
    currentMillis = millis(); // Get current time elapsed
    // display current time left in 5 second timer
    if (currentMillis - previousMillis >= ONE_SECOND) {
      seconds = seconds - 1; // decrement 5 second counter
      // display counter
      u8x8.setCursor(7, 5);
      u8x8.print(seconds);
      previousMillis = currentMillis; // update preivous time with current time
      // return YES if 5 seconds have passed and reset variables
      if (seconds == 0) {
        returnOption = YES;
        previousMillis = 0;
        seconds = 5;       
      }
    }
  }
  // initiate 5 second counter by reading current time
  else if (previousMillis == 0) {
    u8x8.setCursor(7, 5);
    u8x8.print(seconds);
    previousMillis = millis(); 
  }

  return returnOption;
}

// function to detect if light after a threshold has been detected 
int is_light_detected() {

  int returnOption = NO;

  // read the sensor and store it in the variable
  lightReading = analogRead(lightPin);

  // After running the example, set threshold to be the sound seen.

  // if the sensor reading is greater than the threshold
  if (lightReading >= light_threshold) {
    returnOption = YES;
    Serial.print("---------------- Light detected: "); Serial.println(lightReading);
  }

  return returnOption;
}

// function to detect if sound within a range has been detected
int is_sound_detected() {

  int returnOption = NO;

  // read the sensor and store it in the variable
  soundReading = analogRead(soundPin);

  // if the sensor reading is greater than the threshold
  if (soundReading > low_sound_boundary && soundReading < high_sound_boundary) {
    returnOption = YES;
    Serial.print("---------------- Sound detected: "); Serial.println(soundReading);
  }

  return returnOption;

}

// main program loop
void loop() {

  u8x8.setCursor(0, 0);
  // controls and execute functions according to controller state
  switch (controller_state) {

    case GATES_OPEN:
      if (trainStatus == APPROACH) {
        Serial.println("Gates open --> Gates lowering");
        controller_state = GATES_LOWERING;
        u8x8.print("CONTROL LOWER");
        blinking = ON;
      }
      break;

    case GATES_LOWERING:
      fiveSeconds = is_five_seconds();
      u8x8.setCursor(0, 0);
      if (fiveSeconds == YES) {
        Serial.println("Gates lowering --> Gates down");
        controller_state = GATES_DOWN;
        u8x8.print("CONTROL DOWN ");
        goDown = ON;
      }
      break;

    case GATES_DOWN:
      if (trainStatus == EXIT) {
        Serial.println("Gates down --> Gates opening");
        controller_state = GATES_OPENING;
        u8x8.print("CONTROL RISE ");
      }
      break;

    case GATES_OPENING:
      fiveSeconds = is_five_seconds();
      u8x8.setCursor(0, 0);
      if (fiveSeconds == YES) {
        Serial.println("Gates opening --> Gates open");
        controller_state = GATES_OPEN;
        u8x8.print("CONTROL OPEN ");
        blinking = OFF;
        goUp = ON;
      }
      break;

    default:
      Serial.println("Error controller_state: The program should not enter this case.");
      break;
  }

  u8x8.setCursor(0, 1);
  // controls and execute functions according to train state
  switch (train_state) {

    case FAR:
      isClose = is_light_detected();
      if (isClose == YES && controller_state == GATES_OPEN) {
        Serial.println("Train is far --> Train is close");
        train_state = CLOSE;
        u8x8.print("TRAIN IS CLOSE");
        trainStatus = APPROACH;
      }
      break;

    case CLOSE:
      isFar = is_sound_detected();
      if (isFar == YES && controller_state == GATES_DOWN) {
        Serial.println("Train is close --> Train is far");
        train_state = FAR;
        u8x8.print("TRAIN IS FAR  ");
        trainStatus = EXIT;
      }
      break;

    default:
      Serial.println("Error train_state: The program should not enter this case.");
      break;
  }

  u8x8.setCursor(0, 2);
  // controls and execute functions according to traffic light state
  switch (trafficLight_state) {

    case NOT_BLINKING:
      if (blinking == ON) {
        Serial.println("Not blinking --> Blinking");
        trafficLight_state = BLINKING;
        u8x8.print("BLINKING    ");
      }
      break;

    case BLINKING:
      if (blinking == OFF) {
        Serial.println("Blinking --> Not blinking");
        trafficLight_state = NOT_BLINKING;
        u8x8.print("NOT BLINKING");
      }
      do_blinking(); // do blinking after for the case when blinking is OFF
      break;

    default:
      Serial.println("Error trafficLight_state: The program should not enter this case.");
      break;
  }

  u8x8.setCursor(0, 3);
  // controls and execute functions according to gate state
  switch (gate_state) {

    case UP:
      if (goDown == ON) {
        Serial.println("Up --> Down");
        gate_state = DOWN;
        u8x8.print("GATES CLOSED");
        goDown = OFF;
      }
      break;

    case DOWN:
      if (goUp == ON) {
        Serial.println("Down --> Up");
        gate_state = UP;
        u8x8.print("GATES OPEN  ");
        goUp = OFF;
      }
      break;

    default:
      Serial.println("Error gate_state: The program should not enter this case.");
      break;
  }

}

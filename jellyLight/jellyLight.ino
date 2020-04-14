// The digital pin that drives the white LEDs
int ledPin = 7;
// The brightness of the white LEDs
int brightness = 0;   
// The increment to fade the LED by
int fadeAmount = 5;    
// Whether or not the white LEDs are on
bool ledOn = false;
// The maximum brightness of the white LEDs
const int MAX_BRIGHTNESS = 255;

// Whether or not the jelly light is in idle mode
bool isIdle = false;
// Whether or not the jelly light was just in idle mode
bool wasJustIdle = true;

// The control pins for the RGB LEDs
int redPin = 9;
int greenPin = 10;
int bluePin = 11;
// The delay used for displaying RGB LEDs 
int displayTime = 10;
// The initial value for the RGB LEDs.  Ranges from 0 to 768
int ledValue = 0;
// The maximum value for the RGB LEDs
int ledValueMax = 768;

// The analog pin hooked up to the photocell
const int lightPin = A0;
// The analog pin that determines whether or not the jelly light is holding a bookmark
const int timerSensor = A1;
// The analog pin hooked up to the 15 minute heart sensor
const int fifteenSensor = A2;
// The analog pin hooked up to the 30 minute heart sensor
const int thirtySensor = A4;
// The analog pin hooked up to the 45 minute heart sensor
const int fortyFiveSensor = A5;
// The analog pin hooked up to the 60 minute heart sensor
const int sixtySensor = A6;

// The value used to determine how much time has elapsed
uint32_t initialTime = 0;
// The time the user has to read, set by the heart sensors
uint32_t timeToRead = 0;
// Whether or not the timer is running
bool timerRunning = false;

// The amount of time the user has to sleep.  Set to 40 seconds for demo purposes
uint32_t timeToSleep = 40000;
// Whether or not the sleep timer is running
bool sleepTimerRunning = false;

void setup() {
  Serial.begin(9600);

  // Setting up the pins used 
  pinMode(ledPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(lightPin, INPUT);
  pinMode(fifteenSensor, INPUT);
  pinMode(thirtySensor, INPUT);
  pinMode(fortyFiveSensor, INPUT);
  pinMode(sixtySensor, INPUT);
}

void loop() {
  // Read photocell and convert to voltage
  int lightSensor = analogRead(lightPin);
  float lightVoltage = lightSensor * 5 / 1023.0;

  // If the voltage is low, set the jelly light to idle mode 
  if (lightVoltage < 1) {
    isIdle = true;
  }
  // Otherwise, turn idle mode off
  else {
    isIdle = false;
  }

  // If the jelly light is not in idle mode...
  if (!isIdle) {
    // Turn off RGB LEDs
    analogWrite(redPin, LOW);
    analogWrite(bluePin, LOW);
    analogWrite(greenPin, LOW);

    // Read the input to the timer sensors, and convert it to voltage
    int timerSsensorValue = analogRead(timerSensor);
    float timerVoltage = timerSsensorValue * (5.0 / 1023.0);
    // Read the 15, 30,45, and 60 minute sensors, and convert to voltage
    int fifteen = analogRead(fifteenSensor);
    float fifteenVoltage = fifteen * (5.0 / 1023.0);
    int thirty = analogRead(thirtySensor);
    float thirtyVoltage = thirty * (5.0 / 1023.0);
    int fortyFive = analogRead(fortyFiveSensor);
    float fortyFiveVoltage = fortyFive * (5.0 / 1023.0);
    int sixty = analogRead(sixtySensor);
    float sixtyVoltage = sixty * (5.0 / 1023.0);

    // If the jelly light has just entered non-idle mode, and a time to read has been set...
    if (wasJustIdle && (fifteenVoltage > 4 || thirtyVoltage > 4 || fortyFiveVoltage > 4 || sixtyVoltage > 4)) {
      wasJustIdle = false;

      // Depending on which sensor is high, set the time to read
      // Note that times to read are set to 15, 30, 45, and 60 seconds for demo purposes
      if (fifteenVoltage > 4) {
        timeToRead = 15000;
      }
      else {
        if (thirtyVoltage > 4) {
          timeToRead = 30000;
        }
        else {
          if (fortyFiveVoltage > 4) {
            timeToRead = 45000;
          }
          else {
            if (sixtyVoltage > 4) {
              timeToRead = 60000;
            }
          }
        }
      }
    }

    // If the jelly light is holding a bookmark...
    if (timerVoltage > 2) {
      // If timer is not running, turn on the white LEDs and start the timer
      if (brightness == 0 && !timerRunning) {
        fadeLedOn();
        initialTime = millis();
        timerRunning = true;
      }

      // Calculate the elapsed time
      uint32_t currentTime = millis();
      uint32_t elapsedTime = currentTime - initialTime;

      // If the user still has time to read, update the LED value
      if (elapsedTime < timeToRead) {
        // Determine the LED intensity to write, depending on the time remaining
        float secondsLeft = (float)(timeToRead - elapsedTime) / (float)1000;
        float secondsToRead = (float)timeToRead / (float)1000;
        float newBrightness = secondsLeft / secondsToRead * MAX_BRIGHTNESS;

        analogWrite(ledPin, newBrightness);
      }
      // If time is up, turn off the LEDs and turn off the timer
      else {
        timerRunning = false;
        analogWrite(ledPin, LOW);
        return;
      }
    }
    // If the jelly is not holding a bookmark, turn off the white LEDs
    else {
      analogWrite(ledPin, LOW);
    }
  }
  // Otherwise, the jelly is in idle mode
  else {
    // If the jelly light was not just idle and the sleep timer is not running, turn it on
    if (!wasJustIdle && !sleepTimerRunning) {
      analogWrite(ledPin, LOW);
      brightness = 0;

      sleepTimerRunning = true;
      initialTime = millis();
      wasJustIdle = true;   
    }

    // If the sleep timer is running...
    if (sleepTimerRunning) {
      // Determine how much time the user has left to sleep
      uint32_t currentTime = millis();
      uint32_t elapsedTime = currentTime - initialTime;

      // If there is still time left, show the RGB spectrum
      if (elapsedTime < timeToSleep) {
        showRGB(ledValue); 
        delay(displayTime);
        ledValue++;

        if (ledValue == ledValueMax) {
          ledValue = 0;
        }
      }
      // If time is up, turn the timer off
      else {
        sleepTimerRunning = false;
      }
    }
    // If the sleep timer is off, turn off the RGB lights and fade on the white LEDs
    else {
      analogWrite(redPin, LOW);
      analogWrite(bluePin, LOW);
      analogWrite(greenPin, LOW);

      if (brightness < 255) {
        fadeLedOn();
        analogWrite(ledPin, 255);
        delay(10000);
      }

      wasJustIdle = true;
    }
  }
}

// Fades the white LED on, moving gradually from 0 brightness to 255
void fadeLedOn() {
  while (brightness < 255) {
    analogWrite(ledPin, brightness);
    brightness = brightness + fadeAmount;
    delay(50);
  }
}

// Cycles through the RGB spectrum, based on the parameter.
// Adapted from this RGB LED tutorial: https://learn.sparkfun.com/tutorials/sik-experiment-guide-for-arduino---v32/experiment-3-driving-an-rgb-led
void showRGB(int color)
{
  int redIntensity;
  int greenIntensity;
  int blueIntensity;

  // Here we'll use an "if / else" statement to determine which
  // of the three (R,G,B) zones x falls into. Each of these zones
  // spans 255 because analogWrite() wants a number from 0 to 255.

  // In each of these zones, we'll calculate the brightness
  // for each of the red, green, and blue LEDs within the RGB LED.

  if (color <= 255)          // zone 1
  {
    redIntensity = 255 - color;    // red goes from on to off
    greenIntensity = color;        // green goes from off to on
    blueIntensity = 0;             // blue is always off
  }
  else if (color <= 511)     // zone 2
  {
    redIntensity = 0;                     // red is always off
    greenIntensity = 255 - (color - 256); // green on to off
    blueIntensity = (color - 256);        // blue off to on
  }
  else // color >= 512       // zone 3
  {
    redIntensity = (color - 512);         // red off to on
    greenIntensity = 0;                   // green is always off
    blueIntensity = 255 - (color - 512);  // blue on to off
  }

  // Now that the brightness values have been set, command the LED
  // to those values

  analogWrite(redPin, redIntensity);
  analogWrite(bluePin, blueIntensity);
  analogWrite(greenPin, greenIntensity);
}

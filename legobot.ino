Servo leftServo;
Servo rightServo;

const int echoPin = D5;
const int trigPin = D4;

unsigned int sensorTime; //Variable to store how long it takes for the ultrasonic wave to come back
int distance; //Variable to store the distance calculated from the sensor
int triggerDistance = 30; //The distance we want the robot to look for a new path
int fDistance;  //Variable to store the distance in front of the robot
int lDistance;  //Variable to store the distance on the left side of the robot
int rDistance;  //Variable to store the distance on the right side of the robot

bool manual = false;
String manualCommand = "";

void setup() {
  // initialize serial communication to ultrasonic sensor
  Serial.begin(9600);

  // Register commands to control manually
  Spark.function("manual", manualControl);

  leftServo.attach(A0);        //Left servo connected to pin A1
  leftServo.write(90);        //Write the neutral position to that servo
  rightServo.attach(A1);       //Right servo connected to pin A0
  rightServo.write(90);       //Write the neutral position to that servo
}

void manualControl(String command) {

  if (command == 'reset') {
    manual = false;
  } else {
    manual = true;
  }
  manualCommand = command;
}

void resetManualOptions() {
  stop = false;
  left = false;
  right = false;
  forward = false;
  backward = false;
}

void loop() {


  if(manual) {

    if(manualCommand == "stop") {
      moveStop();
    } else if (manualCommand == 'left') {
      moveLeft();
    } else if (manualCommand == 'right') {
      moveRight();
    } else if (manualCommand == 'forward') {
      moveForward();
    } else if (manualCommand == 'backward') {
      moveBackward();
    }

  } else {

    scan(); // Scan for distance from an object
    fDistance = distance; //Set that distance to the front distance

    if(fDistance < triggerDistance) { //If there is something closer than 30cm in front of us
      moveBackward();
      delay(1000);
      moveRight();
      delay(500);
      moveStop();

      scan();

      rDistance = distance; //Store that to the distance on the right side
      moveLeft();
      delay(1000);
      moveStop();

      scan();

      lDistance = distance;
      if(lDistance < rDistance) { //If the distance on the left is smaller than that of the right
        moveRight(); // Turn Right for 200 milliseconds.
        delay(200);
        moveForward();
      } else {
        moveForward();
      }
    } else {
        moveForward();
    }

  }

}

void scan() {
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration;

  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  // convert the time into a distance
  //inches = microsecondsToInches(duration);
  distance = microsecondsToCentimeters(duration);
  if(distance == 0){ //If no ping was recieved
    distance = 100; //Set the distance to max
  }
  delay(10);
}

void moveBackward() {
  rightServo.write(180);
  leftServo.write(0);
}

void moveForward() {
  rightServo.write(0);
  leftServo.write(180);
}

void moveRight() {
  rightServo.write(0);
  leftServo.write(0);
}

void moveLeft() {
  rightServo.write(180);
  leftServo.write(180);
}

void moveStop() {
  rightServo.write(90);
  leftServo.write(95);
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

/*
 * pulseIn Function for the Spark Core - Version 0.1.1 (Beta)
 * Copyright (2014) Timothy Brown - See: LICENSE
 *
 * Due to the current timeout issues with Spark Cloud
 * this will return after 10 seconds, even if the
 * input pulse hasn't finished.
 *
 * Input: Trigger Pin, Trigger State
 * Output: Pulse Length in Microseconds (10uS to 10S)
 *
 */

unsigned long pulseIn(uint16_t pin, uint8_t state) {

    GPIO_TypeDef* portMask = (PIN_MAP[pin].gpio_peripheral); // Cache the target's peripheral mask to speed up the loops.
    uint16_t pinMask = (PIN_MAP[pin].gpio_pin); // Cache the target's GPIO pin mask to speed up the loops.
    unsigned long pulseCount = 0; // Initialize the pulseCount variable now to save time.
    unsigned long loopCount = 0; // Initialize the loopCount variable now to save time.
    unsigned long loopMax = 20000000; // Roughly just under 10 seconds timeout to maintain the Spark Cloud connection.

    // Wait for the pin to enter target state while keeping track of the timeout.
    while (GPIO_ReadInputDataBit(portMask, pinMask) != state) {
        if (loopCount++ == loopMax) {
            return 0;
        }
    }

    // Iterate the pulseCount variable each time through the loop to measure the pulse length; we also still keep track of the timeout.
    while (GPIO_ReadInputDataBit(portMask, pinMask) == state) {
        if (loopCount++ == loopMax) {
            return 0;
        }
        pulseCount++;
    }

    // Return the pulse time in microseconds by multiplying the pulseCount variable with the time it takes to run once through the loop.
    return pulseCount * 0.405; // Calculated the pulseCount++ loop to be about 0.405uS in length.
}

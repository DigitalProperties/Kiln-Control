#include <Metro.h>
#include <PID_v1.h>
#include <max6675.h>
#include <SoftwareSerial.h>

//Declare the pins used for thermocouple input and relay out
int ktcSO = 2;
int ktcCS = 3;
int ktcCLK = 4;
byte readbyte;

#define RelayPin 13

//Define Variables we'll be connecting to
double setpoint, Input, Output, pidActualP, pidActualI, pidActualD;
double pidP = 1;
double pidI = 0.05;
double pidD = 0.25;
long setTimer, timeRemaining, readTimer;

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);

//SoftwareSerial BTserial(9, 10); // RX | TX

//S pecify the links and initial tuning parameters
PID myPID(&Input, &Output, &setpoint, pidP, pidI, pidD, DIRECT);

// PWM window things
int WindowSize = 5000;  
unsigned long windowStartTime;
unsigned long previousMillis;

// Metro timer things
Metro holdTimer = Metro(); // Initialize
bool doesTimerExist = false;
bool isFinishedCycle = false;


/*
 * Arduino Default Functions
 */
 
void setup() {
  // give the MAX a little time to settle
  windowStartTime = millis();
   
  // pin out
  pinMode(RelayPin, OUTPUT);

  //initialize the variables we're linked to
  setpoint = 1050;

  //tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, WindowSize);

  //turn the PID on
  myPID.SetMode(AUTOMATIC);

  Serial.begin(9600);
}

void loop() {
  // Capture serial input
  doSerial();
  
  if (!isFinishedCycle) {    
    // If up to temp, hold (aka continue as normal, until timer is hit, then shut it down)
    if (Input >= setpoint ) {
        holdAtTemp();
    }
    
    // Turn the pins on/off
    executePinouts();
  }
  else {
     digitalWrite(RelayPin,LOW);
  }
}

/*
 * Custom Functions
 */
// Do computations and turn the heater on/off
void executePinouts() {
    // Run every 500ms
  if (millis() >= (previousMillis + 250)){
      // Display a readout to the serial
      getReadout();
      
      // Update PID
      Input = ktc.readFahrenheit();
      myPID.Compute();

      // Update time
      previousMillis = millis();
  }

  /* turn the output pin on/off based on pid output */
  if(millis() - windowStartTime>WindowSize){ //time to shift the Relay Window
    windowStartTime += WindowSize;
  }if(Output < millis() - windowStartTime) {
    digitalWrite(RelayPin,HIGH);
  } else{
    digitalWrite(RelayPin,LOW);
  }
}

// Holds for a specific amount of time (holdTimer)
void holdAtTemp () {
    // Start a timer IF we are up to temp, and no timer yet exists to prevent overwriting/reinstantiating
    if (!doesTimerExist) {
      holdTimer.interval(setTimer);
      holdTimer.reset();
      readTimer = millis();
      doesTimerExist = true;
    }

    // If we HIT our hold time, we set temp to 32 (0 for device)
    if (holdTimer.check() == true) {
      Serial.println("Timer Complete");
      // initialize the variables we're linked to
      setpoint = 32;
      isFinishedCycle = true;
    }
}

// Listens for input
void doSerial() {
  // Readout / Set Tunings
  while (Serial.available()) {
    readbyte = Serial.read(); /* read the most recent byte */

    // Reset the flags so you can update the serial
    isFinishedCycle = false;
    doesTimerExist = false;
    
    switch (readbyte) {
        case 'p':
          pidP = Serial.parseFloat();
          break;
        case 'i':
          pidI = Serial.parseFloat();
          break;
        case 'd': 
          pidD = Serial.parseFloat();
          break;
        case 't': 
          setpoint = Serial.parseInt();
          break;
        case 'm':
          setTimer = Serial.parseInt();
          setTimer = setTimer * 6000;   //minutes to millis
          break;
    }

    myPID.SetTunings(pidP, pidI, pidD);
  }
}

// Display to user
void getReadout() {
  // Basic readout test
  Serial.print("\n Target = ");
  Serial.print(setpoint);
  Serial.print("\t Temp *F = ");
  Serial.print(Input);   
  Serial.print("\t Output = ");
  Serial.print(Output);   
//  Serial.print("\t PID = ");
//  Serial.print(myPID.GetKp());
//  Serial.print(" | ");
//  Serial.print(myPID.GetKi());
//  Serial.print(" | ");
//  Serial.print(myPID.GetKd());
  if (setTimer >= 1){
    Serial.print(" timer ");
    Serial.print(holdTimer.remaining());
  }
}

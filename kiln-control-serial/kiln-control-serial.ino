#include <PID_v1.h>
#include <max6675.h>
#include <SoftwareSerial.h>
#include <Metro.h>

//Declare the pins used for thermocouple input and relay out
int ktcSO = 2;
int ktcCS = 3;
int ktcCLK = 4;
byte readbyte;

#define RelayPin 15

//Define Variables we'll be connecting to
double setpoint, Input, Output, pidActualP, pidActualI, pidActualD;
double pidP = 1;
double pidI = 0.05;
double pidD = 0.25;

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);

//SoftwareSerial BTserial(9, 10); // RX | TX

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &setpoint, pidP, pidI, pidD, DIRECT);

int WindowSize = 5000;  
unsigned long windowStartTime;
unsigned long previousMillis;
Metro holdTimer;
bool doesTimerExist = false;
 

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
  // Get current time in loop
  unsigned long currentMillis = millis();
  
  // Readout / Set Tunings
  while (Serial.available()) {
    readbyte = Serial.read(); /* read the most recent byte */
    
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
    }

    myPID.SetTunings(pidP, pidI, pidD);
  }

  // HOLD AT TEMP (aka continue as normal, until timer is hit, then shut it down)
  if (Input >= setpoint) {
    // Start a timer IF we are up to temp, and no timer yet exists to prevent overwriting/reinstantiating
    if (!doesTimerExist) { // Not 100% sure this will work
      Serial.println("Created timer");
      holdTimer = Metro(600); // Initialize
      doesTimerExist = true;
    }
    
    // If we HIT our hold time, we set temp to 32 (0 for device)
    if (holdTimer.check()) {
      Serial.println("Created done");
      // initialize the variables we're linked to
      setpoint = 32;
    }
  }
 
  // Run every 500ms
  if (currentMillis >= (previousMillis + 500)){
      // Basic readout test
      Serial.print("\n Target = ");
      Serial.print(setpoint);
      Serial.print("\t Temp *F = ");
      Serial.print(Input);   
      Serial.print("\t PID = ");
      Serial.print(myPID.GetKp());
      Serial.print(" | ");
      Serial.print(myPID.GetKi());
      Serial.print(" | ");
      Serial.print(myPID.GetKd());

      // Update values
      Input = ktc.readFahrenheit();
      previousMillis = currentMillis;
      myPID.Compute();
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

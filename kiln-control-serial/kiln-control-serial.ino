#include <PID_v1.h>
#include <max6675.h>

//Declare the pins used for thermocouple input and relay out
int ktcSO = 2;
int ktcCS = 3;
int ktcCLK = 4;
byte readbyte;

#define RelayPin 5

//Define Variables we'll be connecting to
double setpoint, Input, Output, pidActualP, pidActualI, pidActualD;
double pidP = 1;
double pidI = 0.05;
double pidD = 0.25;

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &setpoint, pidP, pidI, pidD, DIRECT);

int WindowSize = 5000;
unsigned long windowStartTime;


void setup() {
  // give the MAX a little time to settle
   windowStartTime = millis();
   
   //pin out
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
  while (Serial.available()) {
    
    /* read the most recent byte */
    readbyte = Serial.read();
    
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

  pidActualP = myPID.GetKp(); 
  pidActualI = myPID.GetKi(); 
  pidActualD = myPID.GetKd();
  
  // basic readout test
  Serial.print("\n Target = ");
  Serial.print(setpoint);
  Serial.print("\t Temp *F = ");
  Serial.print(Input);   
  Serial.print("\t PID = ");
  Serial.print(pidActualP);
  Serial.print(" | ");
  Serial.print(pidActualI);
  Serial.print(" | ");
  Serial.print(pidActualD);

  Input = ktc.readFahrenheit();

  myPID.Compute();

  /************************************************
   * turn the output pin on/off based on pid output
   ************************************************/
  if(millis() - windowStartTime>WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  
  if(Output < millis() - windowStartTime) {
    digitalWrite(RelayPin,HIGH);
  }
  else{
    digitalWrite(RelayPin,LOW);
  }
}



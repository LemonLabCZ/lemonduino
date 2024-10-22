/*
  Name:		neuroduino.ino
  Created:	1/1/2024 6:43:19 PM
  Author:	Lukáš 'hejtmy' Hejtmánek
*/

#define ARRAY_SIZE(array) ((sizeof(array))/(sizeof(array[0])))

// Class state
String serialInput;
int timeout = 25;

// CONECTION STATUS
bool connected = false;
int lettingKnowTime = 1000; //duration of how long will we keep the connection trying

// Output Pins
int outputPins[] = {8, 9};
int N_OUTPUT_PINS = 2;
bool pulsing = false;
int pulsingStartTime = 0;
int pulseDuration = 50; //speed for the delay factor
int diodePin = 10;

// General values
unsigned long msAtStart;
unsigned long msFromStart;

// THRESHOLD
int thresholdPin = A1;
int universalThreshold = 500;

// LINE IN
int lineInPin = A0;
bool lineInentData = false;
bool lineInUse = false;
int lineInThreshold = 500;
int lineInOuput = outputPins[0];

// MICROPHONE
bool microphoneUse = false;
int microphonePin = A2;
bool microphoneSendData = false;

// PHOTORESISTOR
int photoresistorPin = 0;
int photoresistorThreshold = 500;
int photoTresholdMargin = 100;
int photoCalibrationTime = 200;
bool photoActionSent = false;
bool photoSendData = false;
int photoTimeSent = 0;
int photoSendingDelay = 200;

char untilChar = '\!';

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(timeout);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  for(int i = 0; i < N_OUTPUT_PINS; i++){
    pinMode(outputPins[i], OUTPUT);
  }
  pinMode(lineInPin, INPUT);
  pinMode(thresholdPin, INPUT);
}

// the loop function runs over and over again until power down or reset
void loop() {
  serialInput = Serial.readStringUntil(untilChar);
  if (serialInput != "") ReactToSerialInput(serialInput);
  // CancellingPulse
  if(pulsing && millis() - pulseStartTime > pulseLength) CancelPulse();

  if (!connected) return;
  UpdateThreshold();
  if(lineInUse) LineInEvaluate();
}

void ReactToSerialInput(String serialInput) {
  if (serialInput == "") return; //no input
  if (serialInput == "WHO") LettingKnow(); // This blocks the code until the connection is established
  if (serialInput == "DISCONNECT") Disconnect();
  if (!connected) return; //will be false for both connection and disconnection times
  ListenForOrders(serialInput);
}

void UpdateThreshold() {
  lineInThreshold = AnalogRead(thresholdPin);
}

// The function blocks rest of the code until the connection is established or Timeout reached
void LettingKnow() {
  unsigned long t = millis();
  while (true) {
    serialInput = Serial.readStringUntil(untilChar);
    if (serialInput == "DONE") {
      Connect();
      break;
    }
    Serial.println("NEURODUINO");
    if (millis() - t > lettingKnowTime) {
      Serial.println("TIME IS UP");
      break;
    }
    delay(speed);
  }
}

void Connect() {
  msAtStart = millis();
  connected = true;
}

void Disconnect() {
  Restart();
  connected = false;
}

void Restart() {
  CancelPulse();
}

void LineInEvaluate() {
  int lineIn = AnalogRead(lineInPin);
  if(lineIn > lineInThreshold){
    if(pulsing) return;
    LineInPulse();
  } else {
    if(pulsing) CancelPulse();
  }
}

void ListenForOrders(String serialInput) {
  if(serialInput.substring(0,6) == "PULSE+"){ 
    StartPulse(serialInput);
    SendDone();
  }
  if (serialInput == "PULSE-") {
    CancelPulse();
    SendDone();
  }
  if (serialInput == "BLINK") {
    Blink();
    SendDone();
  }
  if (serialInput == "LINEIN-START") {
    lineInUse = true;
    SendDone();
  }
  if (serialInput == "LINEIN-END") {
    lineInUse = false;
    SendDone();
  }
  if (serialInput == "RESTART") {
    Restart();
  }
}

unsigned long GetTime() {
  unsigned long msSinceStart = millis() - msAtStart;
  return msSinceStart;
}

void SendDone() {
  unsigned long msSinceStart = GetTime();
  char buf[14];
  sprintf(buf,"DONE%lu", msSinceStart);
  Serial.println(buf);
}

void Blink() {
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
}

void LineInPulse() {
  pulsing = true;
  pulseStartTime = millis();
  digitalWrite(lineInPulse, HIGH);
}

void StartPulse(String inputString){
  pulsing = true; // no fuctionality yet
  // The signal comes as PULSE+01 where the 
  // 0 an 1 determine pins at given positions to be on or off
  String activePins = inputString.substring(6,10);
  //Serial.println(activePins);
  char buf[N_OUTPUT_PINS+1];
  activePins.toCharArray(buf, N_OUTPUT_PINS + 1);
  //Serial.println(buf);
  for(int i = 0; i < N_OUTPUT_PINS; i++){
    int val = buf[i] - '0';
    //Serial.println(val);
    if(val == 1){
      digitalWrite(outputPins[i], HIGH);
      //Serial.println("true");
    } else {
      digitalWrite(outputPins[i], LOW);
      //Serial.println("false");
    }
  }
}

void CancelPulse(){
  pulsing = false; // no fuctionality yet
  for(int i = 0; i < N_OUTPUT_PINS; i++){
    digitalWrite(outputPins[i], LOW);
  }
}

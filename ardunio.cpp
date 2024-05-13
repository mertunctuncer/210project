#include <Servo.h>




// SONAR
const int TRIGGER_PIN = 7;
const int ECHO_PIN = TRIGGER_PIN;
const float MILLIS_TO_CM = 0.01723;

float fetchDistance()
{
  pinMode(TRIGGER_PIN, OUTPUT);
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  pinMode(ECHO_PIN, INPUT);
  return MILLIS_TO_CM * pulseIn(ECHO_PIN, HIGH);
}

// BUZZER
const int BUZZER_PIN = 8;
const float MAX_HZ = 10000;
const float MIN_HZ = 1000;
float hz;


void buzz(float distance, float maxDistance) {
  hz =((distance * -1) * (MAX_HZ - MIN_HZ) / maxDistance) + MAX_HZ;
  tone(BUZZER_PIN, hz);
}


// LED
const int GREEN_PIN = 4;
const int BLUE_PIN = 5;
const int RED_PIN = 6;

const int COLOR_PINS[3] = {BLUE_PIN, RED_PIN, GREEN_PIN};

int activePin = -1;

void setColor(int color) {
  if(activePin == color) return;
  activePin = color;
  
  for(int pin : COLOR_PINS) {
    if(pin == color) digitalWrite(color, HIGH);
    else digitalWrite(pin, LOW);
  }
}

// SERVO
const int SERVO_PIN = 3;
Servo servo;
const int MAX_ANGLE = 180;
const int STEP = 1;

int angle = 0;
int reverse = 0;

void servoMoveNext() {
  if(reverse)angle -= STEP;
  else angle += STEP;
  
  if(angle > MAX_ANGLE) reverse = 1;
  else if(angle < 0) reverse = 0;
    
  servo.write(angle);
}

void servoReset() {
  angle = 0;
  servo.write(angle);
}

// PROCESS INFO
const float CLOSE = 15;
const float MEDIUM = 30;
void processDistance(float distance) {
  if(distance < CLOSE) {
    setColor(RED_PIN);
    buzz(distance, MEDIUM);
  } else if(distance < MEDIUM) {
    setColor(GREEN_PIN);
    buzz(distance, MEDIUM);
  } else {
    setColor(BLUE_PIN);
    noTone(BUZZER_PIN);
  }
}

char * serialInput;
char * bluetoothInput;

//INPUT
void getSerialInput() {
  while(Serial.available() > 0) {
    
  }
}
void getBluetoothInput() {
  
}

// OUTPUT



// SETUP
void setup()
{
  for(int pin : COLOR_PINS) pinMode(pin, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  servo.attach(SERVO_PIN);
  Serial.begin(9600);
}


// LOOP
float cm = 0;
int sonarActive = 1;

void loop()
{
  getSerialInput();
  getBluetoothInput();
  if(sonarActive) {
    servoMoveNext();
    cm = fetchDistance();
  }
  
  processDistance(cm);
  
  delay(10);
}

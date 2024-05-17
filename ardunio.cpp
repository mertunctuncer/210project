#include <Servo.h>
#include <SoftwareSerial.h>


// << PINS >>
// BUZZER
const int BUZZER_PIN = 4;

// SONAR
const int TRIGGER_PIN = 9;
const int ECHO_PIN = 8;

// SERVO
const int SERVO_PIN = 10;

// LED
const int GREEN_PIN = 6;
const int BLUE_PIN = 5;
const int RED_PIN = 7;

// BLUETOOTH
const int BLUETOOTH_TX_PIN = 0;
const int BLUETOOTH_RX_PIN = 1;

void broadcastData(float, float);


// SONAR
const float MILLIS_TO_CM = 0.01723;
long duration;

float fetchDistance()
{
    pinMode(TRIGGER_PIN, OUTPUT);

    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);

    pinMode(ECHO_PIN, INPUT);
    duration = pulseIn(ECHO_PIN, HIGH);
    return duration * MILLIS_TO_CM;

}

// BUZZER

void buzz(const float distance) {

  if(distance < 5) tone(BUZZER_PIN, 6000);
  else if(distance < 10) tone(BUZZER_PIN, 4000);
  else if(distance < 15) tone(BUZZER_PIN, 2000);
}


// LED

const int COLOR_PINS[3] = {BLUE_PIN, RED_PIN, GREEN_PIN};

int activePin = -1;

void setColor(const int color) {
    if(activePin == color) return;
    activePin = color;

    for(int pin : COLOR_PINS) {
        if(pin == color) digitalWrite(color, HIGH);
        else digitalWrite(pin, LOW);
    }
}

// SERVO

Servo servo;
const int MAX_ANGLE = 175;
const int MIN_ANGLE = 5;
const int STEP = 3;

int angle = 0;
int reverse = 0;

void servoMoveNext() {
    if(reverse)angle -= STEP;
    else angle += STEP;

    if(angle >= MAX_ANGLE) reverse = 1;
    else if(angle <= MIN_ANGLE) reverse = 0;

    servo.write(angle);
}

void servoSetAngle(int newAngle) {
  angle = newAngle;
  servo.write(angle);
}

void servoReset() {
    angle = MIN_ANGLE;
    servo.write(angle);
}

// PROCESS INFO
const float CLOSE = 15;
const float MEDIUM = 30;

void processDistance(const float distance) {

    if(distance < CLOSE) {
        setColor(RED_PIN);
        buzz(distance);

    } else if(distance < MEDIUM) {
        setColor(GREEN_PIN);
        buzz(distance);

    } else {
        setColor(BLUE_PIN);
        noTone(BUZZER_PIN);
    }
}

// SETUP

SoftwareSerial btSerial(0, 1);

void setup()
{
    for(int pin : COLOR_PINS) pinMode(pin, OUTPUT);

    pinMode(BUZZER_PIN, OUTPUT);

    servo.attach(SERVO_PIN);
    servoSetAngle(90);

    Serial.begin(9600);
    btSerial.begin(9600);
}


// LOOP


int sonarActive = 1;
int autoTurn = 1;
int manualControl = 0;

float distanceCentis = 0;

void loop()
{
  getInput();
  if(autoTurn) {
    servoMoveNext();
  }

  if(sonarActive) {
    distanceCentis = fetchDistance();
    broadcastData(angle, distanceCentis);
    processDistance(distanceCentis);
  }

  delay(150);
  
}

// INPUT
char input;

int parseCommand(byte * buffer, int lastIndex) {
  Serial.print("m#Received message: ");
  input = buffer[0];
  Serial.print(input);
  input = buffer[1];
  Serial.println(input);
  switch(buffer[0]) {
    case 'a': // Auto on - off
      if(buffer[1] == '1') {
        Serial.println("m#Auto rotate mode on!");
        manualControl = 0;
        autoTurn = 1;
      } else {
        Serial.println("m#Auto rotate mode off!");
        autoTurn = 0;
      }
      break;
    case 'r': // Reset to 0
      servoReset();
      break;
    case 's': // Sonar on - off
      if(buffer[1] == '1') {
        Serial.println("m#Sonar turned on!");
        sonarActive = 1;
      } else {
        Serial.println("m#Sonar turned off!");
        sonarActive = 0;
      }
      break;
    case 'c': // Manual control mode
      if(buffer[1] == '1') {
        Serial.println("m#Manual control turned on!");
        autoTurn = 0;
        manualControl = 1;
      } else {
        Serial.println("m#Manual control turned off!");
        manualControl = 0;
      }
      break;
    case 'm': // Set angle
      if(manualControl != 1) {
        Serial.println("m#Manual control is not enabled!");
        return;
      }

      servoSetAngle(buffer[1]);
      break;
  }
}


int badInput = 0;
int index = 0;
byte buffer[2] = {};


void cleanBuffer(char * buffer) {
  for(int i = 0; i < 2; i++) {
    buffer[i] = NULL;
  }
}

void getInput() {
  while(btSerial.available()) {
    if(index == 2) {
      badInput = 1;
        index = 0;
      }

        input = btSerial.read();
        Serial.print("got:");
        Serial.print(input);

        if(input == '\n') {
          if(!badInput) parseCommand(buffer, index);

          cleanBuffer(buffer);
          badInput = 0;
          index = 0;
        } else {
          buffer[index] = input;
          index++;
        }
  }

    while(Serial.available()) {
        if(index == 2) {
          badInput = 1;
          index = 0;
        }

        input = Serial.read();

        if(input == '\n') {
          if(!badInput) parseCommand(buffer, index);

          cleanBuffer(buffer);
          badInput = 0;
          index = 0;
        } else {
          buffer[index] = input;
          index++;
        }
    }
}

void broadcastData(const float angle, const float distanceCms) {
  Serial.print("d#");
  Serial.print(angle);
  Serial.print(" ");
  Serial.println(distanceCms);
}

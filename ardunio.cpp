#include <Servo.h>
#include <SoftwareSerial.h>

void getSerialInput();
void getBluetoothInput();

void sendProcessingData(float, float);
void sendBluetoothData(float, float);



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


void buzz(const float distance, const float maxDistance) {
    hz =((distance * -1) * (MAX_HZ - MIN_HZ) / maxDistance) + MAX_HZ;
    tone(BUZZER_PIN, hz);
}


// LED
const int GREEN_PIN = 4;
const int BLUE_PIN = 5;
const int RED_PIN = 6;

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

// BLUETOOTH
const int BLUETOOTH_TX_PIN = 9;
const int BLUETOOTH_RX_PIN = 10;

SoftwareSerial blSerial(BLUETOOTH_TX_PIN, BLUETOOTH_RX_PIN);




// PROCESS INFO
const float CLOSE = 15;
const float MEDIUM = 30;

void processDistance(const float distance) {

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



// SETUP
void setup()
{
    for(int pin : COLOR_PINS) pinMode(pin, OUTPUT);

    pinMode(BUZZER_PIN, OUTPUT);

    servo.attach(SERVO_PIN);
    servoReset();

    Serial.begin(9600);
    blSerial.begin(9600);
}


// LOOP
float cm = 0;

int sonarActive = 0;
int processingConnected = 0;
int bluetoothConnected = 0;

void loop()
{

    getSerialInput();
    getBluetoothInput();

    if(sonarActive) {

        servoMoveNext();
        cm = fetchDistance();
        processDistance(cm);

        if(processingConnected) sendProcessingData(angle, cm);
        if(bluetoothConnected) sendBluetoothData(angle, cm);

        delay(10);
    }
}

// INPUT

int serialInput;
int bluetoothInput;


int parseCommand(const int input) {
    switch(input) {
        case 0:
            processingConnected = 1;
            break;
        case 1:
            processingConnected = 0;
            break
        case 2:
            bluetoothConnected = 1;
            break;
        case 3:
            bluetoothConnected = 0;
            break;
        case 48: // START '0'
            sonarActive = 1;
            break;
        case 49: // STOP '1'
            sonarActive = 0;
            servoReset();
            break;
        case 50: // RESET '2'
            servoReset();
            break;
        default:
            return 0;
            break;
    }

    return 1;
}


void getSerialInput() {
    while(Serial.available()) {
        serialInput = Serial.read();
        parseCommand(serialInput);
    }
}

void getBluetoothInput() {
    while(blSerial.available()) {
        bluetoothInput = blSerial.read();
        parseCommand(bluetoothInput);
    }
}

// OUTPUT 
void sendProcessingData(const float currentAngle, const float distance) {
    Serial.print(currentAngle);
    Serial.print(" ");
    Serial.println(distance);
}


void sendBluetoothData(const float currentAngle, const float distance) {
    blSerial.print(currentAngle);
    blSerial.print(" ");
    blSerial.println(distance);
}

#include <NewPing.h>
//---Variable definitions---
//Linesensor
bool sensorValues[] = {false, false, false, false, false};
const int sensorPins[] = {5, A2, A3, A4, A5};

const bool straightPath[] = {false, false, true, false, false};
const bool leftTurn[] = {true, true, true, false, false};
const bool rightTurn[] = {false, false, true, true, true};
const bool crossing[] = {true, true, true, true, true};
const bool white[] = {false, false, false, false, false};


//Motors
int pwmSpeedA = 50; //75 appears to be minimum? Kinda need it to be slower though (46 causes steering bugs?)
int pwmSpeedB = 50; //Dev value: 50
// const float pwmCorrectionMod = 1.15; // niet meer nodig denk ik
//A - RIGHT
const int directionPinA = 12;
const int pwmSpeedPinA = 3;
//B - LEFT
const int directionPinB = 13;
const int pwmSpeedPinB = 11;
//A+B
const int mainDrivePwmTurnSpeed = 65; //60
const int subDrivePwmTurnSpeed = 20; //20
bool directionState;

//Navigation algorithm
bool solving = true;
enum robotStatus {idle, forward, leftDetected, rightDetected, stopped, offRoad, whiteDetected, obstacleDetected, intersection, turnDetected};
robotStatus currentStatus = idle;
const bool followLeft = true;
bool turning = false;
bool reversing = false;
//long modeLockout;
int miep = 0; //nonsensical name because the original name was 'i' which may have caused issues
long start = 0;
long timer = 0;
long timer2 = 0;
bool pulseAllowed = true;
bool lastPulse[] = {false, false, false, false, false};

// segmenten display 
#define U1 2
#define U2 4
#define g 0
#define f 1
#define e 8
#define d 9
#define c 10
#define b A1
#define a A0
bool segment = true;
bool finish = true;
long startTimer = 0;
bool FI = true;
bool eindTijdKnipperen = true;
bool tijdSwitchen = false;

// ultrasonic sensor
#define echoPin 6
#define trigPin 7
long distance = 11;

bool timingAllowed = true;

NewPing sonar(trigPin, echoPin, 5);

//---Runtime logic---
void setup() 
{
    // TCCR2B = TCCR2B & B11111000 | B00000110; // for PWM frequency of 122.55 Hz
    TCCR2B = TCCR2B & B11111000 | B00000111;  // for PWM frequency of 30.64 Hz
    
    // Serial.begin(115200);
    pinMode(echoPin, INPUT);
    pinMode(trigPin, OUTPUT);

    //define motor pins
    pinMode(directionPinA, OUTPUT); 
    pinMode(pwmSpeedPinA, OUTPUT);

    pinMode(directionPinB, OUTPUT);
    pinMode(pwmSpeedPinB, OUTPUT);
    
    // segmenten display
    pinMode(U1, OUTPUT);
    pinMode(U2, OUTPUT);
    pinMode(g, OUTPUT);
    pinMode(f, OUTPUT);
    pinMode(e, OUTPUT);
    pinMode(d, OUTPUT);
    pinMode(c, OUTPUT);
    pinMode(b, OUTPUT);
    pinMode(a, OUTPUT);

    begin();

    solving = true;
    
}
void loop() {
    if(solving){
        readSensors();
        solve();
    }
}

void pulseLineSensor()
{
    if(pulseAllowed)
    {
        for (int i = 0; i < 5; i++)
        {
        lastPulse[i] = !digitalRead(sensorPins[i]);
        }
        pulseAllowed = false;
    }
}

void solve(){
    switch(currentStatus){
        case obstacleDetected:
            turnAround();
            timer = millis();
            while(millis() - timer < 1100){
                readSensors();
            }
            while(!(currentStatus == forward || currentStatus == offRoad)){
                readSensors();
            }
            break;      
        case whiteDetected:
            driveBackward();
            while(equal(sensorValues, white)){ // search for the line
                readSensors();
            }
            readSensors();
            if(currentStatus == forward){ // line found and turn around
                turnAround();
                timer = millis();
                while(millis() - timer < 1200){
                    readSensors();
                }
                while(!(currentStatus == forward || currentStatus == offRoad)){ // risky :3, might get stuck on driving backwards. Drive back for 3sec, then read sensors, if white >> go forward until line?
                    readSensors();
                }
            }
            stopMotors();
            break;
        case forward:
            driveForward();
            break;
        case offRoad:
            if(sensorValues[0] || sensorValues[1] && !sensorValues[2]){
                offRoadRight();
            } else if (sensorValues[3] || sensorValues[4] && !sensorValues[2]){
                offRoadLeft();
            }
            break;
        case turnDetected:
            bool kruising = false;
            bool path = false;
            driveForward();
            while(!(equal(sensorValues, straightPath) || currentStatus == offRoad || currentStatus == whiteDetected)){// go forward till we find a path, nothing or a crossing
                readSensors();
                if(equal(sensorValues, crossing)){
                    stopMotors();
                    kruising = true;
                    break;
                }
            }
            if(kruising){ // if we found a crossing, we need to check for a finish
                driveForward();
                timer = millis();
                while(millis() - timer < 500){
                    readSensors();
                }
                stopMotors();
                readSensors();
                if(equal(sensorValues, crossing)){
                    solving = false;
                    nummer(-1);
                    einde();
                } else { // else we turn left because we are hugging to the left wall
                    driveBackward();
                    timer = millis();
                    while(millis() - timer < 500){
                        readSensors();
                    }
                    turnLeft();
                    timer = millis();   
                    while(millis() - timer < 750){
                        readSensors();
                    }
                    while(!(equal(sensorValues, straightPath) || currentStatus == offRoad)){
                        readSensors();
                    }
                }
            } else { // if we didn't find a crossing, we need to check if we can go left, if not go forward and if thats not possible go right
                driveBackward();
                while(!(sensorValues[0] || sensorValues[4])){ // risky, might get stuck on driving backwards
                    readSensors();
                }
                if(sensorValues[4]){
                    driveForward();
                    timer = millis();
                    while(millis() - timer < 500){
                        readSensors();
                    }
                    if(equal(sensorValues, straightPath) || currentStatus == offRoad){
                        path = true;
                    } else {
                        path = false;
                    }
                    driveBackward();
                    while(!sensorValues[2]){
                        readSensors();
                    }
                    stopMotors();
                    if(path){
                        driveForward();
                        while(!(equal(sensorValues, straightPath) || currentStatus == offRoad)){
                            readSensors();
                        }
                    } else {
                        turnRight();
                        timer = millis();
                        while(millis() - timer < 700){
                            readSensors();
                        }
                        while(!(equal(sensorValues, straightPath) || currentStatus == offRoad)){
                            readSensors();
                        }
                    }
                } else { // turn left
                    turnLeft();
                    if(timingAllowed)
                    {
                        timer = millis();
                        timingAllowed = false;
                    }
                    while(millis() - timer < 700){ //delay
                        readSensors();
                    } 
                    while(!(equal(sensorValues, straightPath) || currentStatus == offRoad)){// turn while there is no straight path or offroad
                        readSensors();
                    }
                    timingAllowed = true;
                    stopMotors();
                }
            }
            break;
        default:
            stopMotors();
            break;
    }
        
}
//---Function definitions---
void readSensors(){
    for (int i = 0; i < 5; i++){
        sensorValues[i] = !digitalRead(sensorPins[i]);
    }
    
    tijd();
    distance = sonar.ping_cm();
    
    if(distance > 0 && distance < 5){
        currentStatus = obstacleDetected;
    } else if(equal(sensorValues, straightPath)){ //else
        currentStatus = forward;
    } else if(equal(sensorValues, white)){
        currentStatus = whiteDetected; 
    } else if((sensorValues[0] && sensorValues[1]) || ((sensorValues[3] && sensorValues[4]))){
        currentStatus = turnDetected;
    } else {
        currentStatus = offRoad;
    }


    
    
    
}
bool equal(bool x[], bool y[]){
    for(int i = 0; i < 5; i++)
    {
        if(x[i] != y[i])
        {
            return false;
        }
    }
    return true;
}

// --Drive functions--
void offRoadRight(){
    digitalWrite(directionPinA, HIGH);
    digitalWrite(directionPinB, LOW);
    analogWrite(pwmSpeedPinA, pwmSpeedA - 25);
    analogWrite(pwmSpeedPinB, pwmSpeedB + 15);
}
void offRoadLeft(){
    digitalWrite(directionPinA, HIGH);
    digitalWrite(directionPinB, LOW);
    analogWrite(pwmSpeedPinA, pwmSpeedA + 15);
    analogWrite(pwmSpeedPinB, pwmSpeedB - 25);
}
void turnRight(){
    digitalWrite(directionPinA, HIGH);
    digitalWrite(directionPinB, HIGH);
    analogWrite(pwmSpeedPinA, subDrivePwmTurnSpeed); //20
    analogWrite(pwmSpeedPinB, mainDrivePwmTurnSpeed); //60
}
void turnLeft(){
    digitalWrite(directionPinA, LOW);
    digitalWrite(directionPinB, LOW);
    analogWrite(pwmSpeedPinA, mainDrivePwmTurnSpeed); //60
    analogWrite(pwmSpeedPinB, 20); //20

}
void turnAround(){
    digitalWrite(directionPinA, HIGH);
    digitalWrite(directionPinB, HIGH);
    analogWrite(pwmSpeedPinA, pwmSpeedA);
    analogWrite(pwmSpeedPinB, pwmSpeedB);
}

void stopMotors(){
    analogWrite(pwmSpeedPinA, 0);
    analogWrite(pwmSpeedPinB, 0);
}

void driveForward(){
    digitalWrite(directionPinA, LOW);
    digitalWrite(directionPinB, HIGH);
    analogWrite(pwmSpeedPinA, pwmSpeedA - 10);
    analogWrite(pwmSpeedPinB, pwmSpeedB - 10);
}

void driveBackward(){
    digitalWrite(directionPinA, HIGH);
    digitalWrite(directionPinB, LOW);
    analogWrite(pwmSpeedPinA, pwmSpeedA - 10);
    analogWrite(pwmSpeedPinB, pwmSpeedB - 10);
}

void tijd(){
    if(millis() - timer2 > 5){
        nummer(-1);
        tijdSwitchen = !tijdSwitchen;
        digitalWrite(U1, !tijdSwitchen);
        digitalWrite(U2, tijdSwitchen);
        timer2 = millis();
    }
    miep = (millis() - startTimer) / 1000;
    if(miep >= 100){
            startTimer = millis();
    }
    if(tijdSwitchen){
        nummer(miep / 10);
    } else {
        nummer(miep % 10);
    }
}
void begin(){
    bool u1 = true;
    digitalWrite(U1, LOW);
    digitalWrite(U2, LOW);
    start = millis();
    while(millis() - start < 1000){
        if(u1){
            digitalWrite(U2, HIGH);
            nummer(1);
            delay(5);
            nummer(-1);
            digitalWrite(U2, LOW);
            u1 = false;
        } else {
            digitalWrite(U1, HIGH);
            nummer(0);
            delay(5);
            nummer(-1);
            digitalWrite(U1, LOW);
            u1 = true;
        }   
    }
        digitalWrite(U1, HIGH);
        for(int i = 9; i > 0; i--){
            nummer(i);
            delay(1000);
            nummer(-1);
        }

    
    start = millis();
    while(millis() - start < 1000){
        if(u1){
            digitalWrite(U2, LOW);
            nummer(-2);
            delay(5);
            nummer(-1);
            digitalWrite(U2, HIGH);
            u1 = false;
        } else {
            digitalWrite(U1, LOW);
            nummer(5);
            delay(5);
            nummer(-1);
            digitalWrite(U1, HIGH);
            u1 = true;
        }   
    }
    solving = true;
    startTimer = millis();
}
void einde() {
    stopMotors();
    if (finish) { // als hij bij het einde is
        miep = (millis() - startTimer) / 1000;
        for(int x = 0; x <= 3; x++){
            timer = millis();
            while(millis() - timer < 1000){ 
                if (segment) { // laat 10 tallen zien
                    digitalWrite(U1, LOW);
                    digitalWrite(U2, HIGH);
                    nummer(miep / 10);
                    delay(5);
                    nummer(-1);
                    segment = !segment;
                } else { // laat 123456789 zien
                    digitalWrite(U2, LOW);
                    digitalWrite(U1, HIGH);
                    nummer(miep % 10);
                    delay(5);
                    nummer(-1);
                    segment = !segment;
                }
                digitalWrite(U1, LOW);
                digitalWrite(U2, LOW);
            }
            nummer(-1);
            delay(1000);
        }
        
        while (true) {
            if(FI){
                digitalWrite(U1, LOW);
                nummer(-3);
                delay(5);
                nummer(-1);
                digitalWrite(U1, HIGH);
                FI = false;
            } else {
                digitalWrite(U2, LOW);
                nummer(1);
                delay(5);
                nummer(-1);
                digitalWrite(U2, HIGH);
                FI = true;
                
            }   
        }
    }
}
void nummer(int num){
    if(num == 1){
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
    } else if(num == 2){
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(g, HIGH);
        digitalWrite(e, HIGH);
        digitalWrite(d, HIGH);
    } else if(num == 3){
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(g, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
    } else if(num == 4){
        digitalWrite(f, HIGH);
        digitalWrite(g, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
    } else if(num == 5){
        digitalWrite(a, HIGH);
        digitalWrite(f, HIGH);
        digitalWrite(g, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
    } else if(num == 6){
        digitalWrite(a, HIGH);
        digitalWrite(f, HIGH);
        digitalWrite(g, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
        digitalWrite(e, HIGH);
    } else if(num == 7){
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
    } else if(num == 8){
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
        digitalWrite(e, HIGH);
        digitalWrite(f, HIGH);
        digitalWrite(g, HIGH);
    } else if(num == 9){
        digitalWrite(a, HIGH);
        digitalWrite(f, HIGH);
        digitalWrite(g, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
    } else if(num == 0){
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
        digitalWrite(e, HIGH);
        digitalWrite(f, HIGH);
    } else if (num == -1){ // ledjes uit
        digitalWrite(g, LOW);
        digitalWrite(f, LOW);
        digitalWrite(e, LOW);
        digitalWrite(d, LOW);
        digitalWrite(c, LOW);
        digitalWrite(b, LOW);
        digitalWrite(a, LOW);
    } else if (num == -2){ // letter T
        digitalWrite(f, HIGH);
        digitalWrite(e, HIGH);
        digitalWrite(d, HIGH);
        digitalWrite(g, HIGH);
    } else if (num == -3) { // letter F
        digitalWrite(f, HIGH);
        digitalWrite(a, HIGH);
        digitalWrite(g, HIGH);
        digitalWrite(e, HIGH);
    }
}




//---Variable definitions---
//Linesensor
bool sensorValues[] = {false, false, false, false, false};
int sensorPins[] = {5, A2, A3, A4, A5};

//Motors
int pwmSpeed = 75;
//A
int directionPinA = 12;
int pwmSpeedPinA = 3;
int brakePinA = 9;
//B
int directionPinB = 13;
int pwmSpeedPinB = 11;
int brakePinB = 8;
bool directionState;

#define TCCR2B & B11111000 | B0000111; // for PWM frequency of 30.64 Hz
 
//---Runtime logic---
void setup() {
    Serial.begin(9600);
    pinMode(5, INPUT);


//define motor pins
    pinMode(directionPinA, OUTPUT);
    pinMode(pwmSpeedPinA, OUTPUT);
    pinMode(brakePinA, OUTPUT);

    pinMode(directionPinB, OUTPUT);
    pinMode(pwmSpeedPinB, OUTPUT);
    pinMode(brakePinB, OUTPUT);
    
}
void loop() {
    readSensor();
    for (int i = 0; i < 5; i++) {
        Serial.print(sensorValues[i]);
    }
    Serial.println();
    turn(true);
    delay(1000);
}

//---Function definitions---
//Sensors
void readSensor(){
    for(int i = 0; i < 5; i++){
        sensorValues[i] = !digitalRead(sensorPins[i]);
    }
}

//Movement
void driveForward(){
    //Vroem
    digitalWrite(directionPinA, HIGH);
    digitalWrite(pwmSpeedPinA, HIGH);
    
    digitalWrite(pwmSpeedPinB, HIGH);
}

void turn(bool left) {
    if(left) {
        // Draai naar links
        digitalWrite(directionPinA, LOW);  
        digitalWrite(brakePinA, LOW);     
        analogWrite(pwmSpeedPinA, 75);    

        digitalWrite(directionPinB, HIGH); 
        digitalWrite(brakePinB, LOW);       
        analogWrite(pwmSpeedPinB, 75);     
    } else {
        // Draai naar rechts
        digitalWrite(directionPinA, HIGH);  
        digitalWrite(brakePinA, LOW);       
        analogWrite(pwmSpeedPinA, 75);     

        digitalWrite(directionPinB, LOW);   
        digitalWrite(brakePinB, LOW);       
        analogWrite(pwmSpeedPinB, 75);     
    }
}
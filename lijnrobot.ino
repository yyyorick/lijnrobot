//---Variable definitions---
//Linesensor
bool sensorValues[] = {false, false, false, false, false};
int sensorPins[] = {5, A2, A3, A4, A5};

bool straightPath[] = {false, false, true, false, false};
bool leftTurn[] = {true, true, true, false, false};
bool rightTurn[] = {false, false, true, true, true};
bool crossing[] = {true, true, true, true, true};
//bool offRoad[] = {false, false, false, false, false}; might be redundant?

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

//Navigation algorithm
enum robotStatus {idle, forward, leftDetected, rightDetected, crossingDetected, stopped, obstacleDetected};
robotStatus currentStatus = idle;
bool followLeft = true;
 
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
   debugSensorOutput();
   translateSensor();
   Navigate();

    delay(1000);

}

//---Function definitions---
//Sensors
void readSensor(){
    for(int i = 0; i < 5; i++){
        sensorValues[i] = !digitalRead(sensorPins[i]);
    }
}

void translateSensor()
{
    if(isBoolArrayEqual(sensorValues, straightPath, 5, 5))
    {
        SetRobotState(forward);
    }
    else if(isBoolArrayEqual(sensorValues, leftTurn, 5, 5))
    {
        SetRobotState(leftDetected);
    }
    else if(isBoolArrayEqual(sensorValues, rightTurn, 5, 5))
    {
        SetRobotState(rightDetected);
    }
    else if(isBoolArrayEqual(sensorValues, crossing, 5, 5))
    {
        SetRobotState(crossingDetected);
    }
    // else
    // {
        //fullStop();
        //SetRobotState(stopped);
    //}

}

void debugSensorOutput()
{
    readSensor();
    for (int i = 0; i < 5; i++) {
        Serial.print(sensorValues[i]);
    }
    Serial.println();
    // turn(true);
    // delay(1000); temporary code, disabled to test robot states
}

//Movement
void fullStop()
{
    digitalWrite(pwmSpeedPinA, LOW);
    digitalWrite(pwmSpeedPinB, LOW);

    digitalWrite(brakePinA, HIGH);
    digitalWrite(brakePinB, HIGH);

}
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

//Navigation
void Navigate()
{
    if(currentStatus != obstacleDetected)
    {
        switch (currentStatus)
        {
        case idle:
            Serial.println("I am idle!");
            break;
        case forward:
            Serial.println("I am going forward!");
            break;
        case leftDetected:
            Serial.println("Left turn detected!");
            break;
        case rightDetected:
            Serial.println("Right turn detected!");
            break;
        case crossingDetected:
            Serial.println("Crossing (Or endpoint) detected!");
            break;
        case stopped:
            Serial.println("I have stopped!");
            //Probably need to turn around?
            break;
        default:
            Serial.println("De robot is stukkie wukkie :3");
            //fullStop();
            break;
        }
    }
    else
    {
        //Navigate around the obstacle
    }
        
}

void SetRobotState(robotStatus newState)
{
    currentStatus = newState;
}

//---Helper functions---
bool isBoolArrayEqual(bool* a1, bool* a2, int a1Count, int a2Count)
{
   if(a1Count != a2Count)
   {
    return false; //Arrays cannot be equal if their counts are different.
   }

   for (int i = 0; i < a2Count; i++)
   {
        if(a1[i] != a2[i])
        {
            return false; //Return false if a difference is detected.
        }
   }
   
   return true; //Arrays are the same.
    
}
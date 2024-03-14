//---Variable definitions---
//Linesensor
bool sensorValues[] = {false, false, false, false, false};
const int sensorPins[] = {5, A2, A3, A4, A5};

const bool straightPath[] = {false, false, true, false, false};
const bool leftTurn[] = {true, true, true, false, false};
const bool turningLeftA[] = {false, true, true, false, false};
const bool turningLeftB[] = {true, true, true, true, false};
const bool rightTurn[] = {false, false, true, true, true};
const bool turningRightA[] = {false, false, true, true, true};
const bool turningRightB[] = {false, true, true, true, true};
const bool crossing[] = {true, true, true, true, true};
const bool offRoad[] = {false, false, false, false, false};


//Motors
int pwmSpeed = 46; //75 appears to be minimum? Kinda need it to be slower though (46 causes steering bugs?)
const float pwmCorrectionMod = 1.15;
//A - RIGHT
const int directionPinA = 12;
const int pwmSpeedPinA = 3;
const int brakePinA = 9;
//B - LEFT
const int directionPinB = 13;
const int pwmSpeedPinB = 11;
const int brakePinB = 8;
bool directionState;

//Navigation algorithm
bool solving = true;
enum robotStatus {idle, forward, leftDetected, rightDetected, crossingDetected, stopped, obstacleDetected};
robotStatus currentStatus = idle;
bool followLeft = true;
bool turning = false;
bool reversing = false;
//long modeLockout;

String lastSensorDebug = "";


//---Runtime logic---
void setup() 
{
    TCCR2B = TCCR2B & B11111000 | B00000110; // for PWM frequency of 122.55 Hz
    Serial.begin(9600);
    pinMode(5, INPUT);


    //define motor pins
    pinMode(directionPinA, OUTPUT); //Might want to put these in an array and just loop to do this
    pinMode(pwmSpeedPinA, OUTPUT);
    pinMode(brakePinA, OUTPUT);

    pinMode(directionPinB, OUTPUT);
    pinMode(pwmSpeedPinB, OUTPUT);
    pinMode(brakePinB, OUTPUT);
    
}
void loop() {
    switch (currentStatus)
    {
    case idle:
        Serial.println("STATUS: IDLE");
        break;
    default:
        break;
    }
    if(solving)
    {
        debugSensorOutput();
        //translateSensor();
        ReworkedTranslateSensor();
        ReworkedNavigate();
        //Navigate();
    }
}

//---Function definitions---












//Sensors
void readSensor(){
    for(int i = 0; i < 5; i++){
        sensorValues[i] = !digitalRead(sensorPins[i]);
    }
}
/*
void translateSensor() //Translate SensorValues array to Robot 
{
    //modeLockout = millis();
    if(onStraight && currentStatus != forward && !turning)
    {
        SetRobotState(forward);
    }
    if(!onCross())
    {
            if(onLeft() && currentStatus != leftDetected && !turning)
            {
                SetRobotState(leftDetected);
                turning = true;
            }
            else if(onRight() && currentStatus != rightDetected && !turning)
            {
                SetRobotState(rightDetected);
                turning = true;
            }
    }
    else
    {
            if(currentStatus != crossingDetected && !turning)
            {
                SetRobotState(crossingDetected);
                turning = true;
            }
    }
    
    if(onWhite() && currentStatus != stopped && !turning)
    {
        SetRobotState(stopped);
    }

}
*/
void ReworkedTranslateSensor() //Just making it again to see if I can do anything differently
{
    if(isBoolArrayEqual(sensorValues, straightPath, 5, 5) && !turning)
    {
        SetRobotState(forward);
    } 
    else if(isBoolArrayEqual(sensorValues, leftTurn, 5, 5) && !turning)
    {
        SetRobotState(leftDetected);
        turning = true;
    } 
    else if(isBoolArrayEqual(sensorValues, rightTurn, 5, 5) && !turning)
    {
        SetRobotState(rightDetected);
        turning = true;
    }
    else if(isBoolArrayEqual(sensorValues, offRoad, 5, 5) && !turning)
    {
        SetRobotState(stopped);
        //fullStop();
        //Serial.println("Stop");
    }
    else if(isBoolArrayEqual(sensorValues, crossing, 5, 5) && !turning)
    {
        // if(followLeft)
        // {
        //     turnLeft();
        // }
        // else
        // {
        //     turnRight();
        // }
        fullStop();
        Serial.println("TESTSTOP");
    }
}

void ReworkedNavigate()
{
    switch (currentStatus)
    {
    case idle:
        Serial.println("zzzzzz");
        break;
    case forward:
        driveForward();
        Serial.println("FW");
        break;
    case leftDetected:
        turnLeft();
        Serial.println("LT");
        break;
    case rightDetected:
        turnRight();
        Serial.println("RT");
        break;
    case stopped:
        turnAround();
        Serial.println("STOPPED");
        break;
    default:
        Serial.println("De robot is stukkie wukkie :3");
        break;
    }
}

void debugSensorOutput()
{
    readSensor();
    // for (int i = 0; i < 5; i++) {
    //     Serial.print(sensorValues[i]);
    
    // }
    // Serial.println();
}

//Movement
void fullStop()
{
    digitalWrite(pwmSpeedPinA, 0);
    digitalWrite(pwmSpeedPinB, 0);

    digitalWrite(brakePinA, HIGH);
    digitalWrite(brakePinB, HIGH);

    turning = false;
}
void driveForward(){
    //Vroem
    digitalWrite(brakePinA, LOW);
    digitalWrite(brakePinB, LOW);

    digitalWrite(directionPinA, LOW); //todo: figure out what the directionpin actually does, robot seems to turn right at random while going straight.
    analogWrite(pwmSpeedPinA, pwmSpeed * pwmCorrectionMod);
    
    digitalWrite(directionPinB, HIGH);
    analogWrite(pwmSpeedPinB, pwmSpeed);

   
}

void driveBackward()
{
    digitalWrite(brakePinA, LOW);
    digitalWrite(brakePinB, LOW);

    digitalWrite(directionPinA, HIGH);
    analogWrite(pwmSpeedPinA, pwmSpeed * pwmCorrectionMod);
    
    digitalWrite(directionPinB, LOW);
    analogWrite(pwmSpeedPinB, pwmSpeed);
}
void turnAround(){
    
    digitalWrite(brakePinA, LOW);
    digitalWrite(brakePinB, LOW);

    digitalWrite(directionPinA, HIGH);
    analogWrite(pwmSpeedPinA, pwmSpeed * pwmCorrectionMod);
    
    digitalWrite(directionPinB, HIGH);
    analogWrite(pwmSpeedPinB, pwmSpeed);
    delay(1000);
}

//Navigation
/*
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
            if(!reversing)
            {
                Serial.println("I am going forward!");
                driveForward();
            }
            else
            {
                Serial.println("I am reversing!");
                driveBackward();
            }
            break;
        case leftDetected:
            Serial.println("Left turn detected!");
                turn(true);
            break;
        case rightDetected:
            Serial.println("Right turn detected!");
                turn(false);
            break;
        case crossingDetected:
            Serial.println("Crossing (Or endpoint) detected!");
            turn(followLeft);
            break;
        case stopped:
            Serial.println("I have stopped!");
            fullStop();
            // reversing = true;
            // driveBackward();
            //delay(45); //busywait might not be allowed but works for now
            //fullStop();
            if(onCross())
            {
                //Either at crossing or endpoint
                //if crossing > turn in followdir
                //else > stop SolveMaze(); and go idle

                //SolveMaze(); //temp
            }
            else if(isBoolArrayEqual(sensorValues, straightPath, 5, 5))
            {
                //Dead end, turn around
                //SolveMaze(); //temp
            } 
            else if (isBoolArrayEqual(sensorValues, offRoad, 5, 5))
            {
                turnAround();
            }
            
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
*/
void SetRobotState(robotStatus newState)
{
    //fullStop();
    currentStatus = newState;
}

bool WallDetected(bool left)
{
    if(left)
    {
        if(!sensorValues[0] && !sensorValues[1]) //Linesensor OUT1 and OUT2 return false (White)
        {
            return true;
        }
    }
    else
    { 
        if(!sensorValues[3] && !sensorValues[4]) //Linesensor OUT4 and OUT5 return false (White)
        {
            return true;
        }
    }

    return false;
}

void SolveMaze() //Call this when at the endpoint to stop the robot
{
    SetRobotState(idle);
    solving = false;
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

void turnRight()
{
    while(turning)
    {
        digitalWrite(directionPinB, HIGH);  
        analogWrite(pwmSpeedPinB, pwmSpeed);   
        analogWrite(pwmSpeedPinA, 0);    
        debugSensorOutput();
        if(isBoolArrayEqual(sensorValues, straightPath, 5, 5))
        {
            turning = false;
        }
    }

}

void turnLeft()
{
    while(turning)
    {
        digitalWrite(directionPinA, LOW);  
        analogWrite(pwmSpeedPinA, pwmSpeed * pwmCorrectionMod);   
        analogWrite(pwmSpeedPinB, 0);
        debugSensorOutput();
        if(isBoolArrayEqual(sensorValues, straightPath, 5, 5))
        {
            turning = false;
            break;
        }
    }
}
 
void begin() {
    
}
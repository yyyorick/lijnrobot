//---Variable definitions---
//Linesensor
bool sensorValues[] = {false, false, false, false, false};
int sensorPins[] = {5, A2, A3, A4, A5};

bool straightPath[] = {false, false, true, false, false};
bool leftTurn[] = {true, true, true, false, false};
bool turningLeftA[] = {false, true, true, false, false};
bool turningLeftB[] = {true, true, true, true, false};
bool rightTurn[] = {false, false, true, true, true};
bool turningRightA[] = {false, false, true, true, true};
bool turningRightB[] = {false, true, true, true, true};
bool crossing[] = {true, true, true, true, true};
bool offRoad[] = {false, false, false, false, false};

bool onStraight()
{
    return isBoolArrayEqual(sensorValues, straightPath, 5, 5);
}
bool onLeft()
{
    return isBoolArrayEqual(sensorValues, leftTurn, 5, 5);
}
bool onRight()
{
    return isBoolArrayEqual(sensorValues, rightTurn, 5, 5);
}
bool onCross()
{
    return isBoolArrayEqual(sensorValues, crossing, 5, 5);
}
bool onWhite()
{
    return isBoolArrayEqual(sensorValues, offRoad, 5, 5);
}
//Motors
int pwmSpeed = 75; //75 appears to be minimum?
//A - RIGHT
int directionPinA = 12;
int pwmSpeedPinA = 3;
int brakePinA = 9;
//B - LEFT
int directionPinB = 13;
int pwmSpeedPinB = 11;
int brakePinB = 8;
bool directionState;

#define TCCR2B & B11111000 | B0000111; // for PWM frequency of 30.64 Hz

//Navigation algorithm
bool solving = true;
enum robotStatus {idle, forward, leftDetected, rightDetected, crossingDetected, stopped, obstacleDetected};
robotStatus currentStatus = idle;
bool followLeft = true;
bool turning = false;
//long modeLockout;
 
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
//    if((millis() - modeLockout) >= 2000)
//    {
        //translateSensor();
//    }

    if(solving)
    {
        debugSensorOutput();
        translateSensor();
        Navigate();
    }
   

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
    //modeLockout = millis();
    if(onStraight && currentStatus != forward && !turning)
    {
        SetRobotState(forward);
    }
    else if(onLeft() && currentStatus != leftDetected && !turning)
    {
        SetRobotState(leftDetected);
        turning = true;
    }
    else if(onRight() && currentStatus != rightDetected && !turning)
    {
        SetRobotState(rightDetected);
        //turning = true;
    }
    else if(onCross() && currentStatus != crossingDetected && !turning)
    {
        SetRobotState(crossingDetected);
        turning = true;
    }
    else if(onWhite() && currentStatus != stopped)
    {
        SetRobotState(stopped);
    }

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
    analogWrite(pwmSpeedPinA, pwmSpeed);
    
    digitalWrite(directionPinB, HIGH);
    analogWrite(pwmSpeedPinB, pwmSpeed);

   
}

void driveBackward()
{
    digitalWrite(brakePinA, LOW);
    digitalWrite(brakePinB, LOW);

    digitalWrite(directionPinA, HIGH);
    analogWrite(pwmSpeedPinA, pwmSpeed);
    
    digitalWrite(directionPinB, LOW);
    analogWrite(pwmSpeedPinB, pwmSpeed);
}

void turn(bool left) { //This function needs to turn the robot 90 degrees
    turning = true;
    if(left) {
        // Draai naar links
        //digitalWrite(brakePinB, HIGH);     
        //digitalWrite(brakePinA, HIGH);     

        digitalWrite(directionPinA, LOW);  
        analogWrite(pwmSpeedPinA, pwmSpeed);   

        //digitalWrite(directionPinB, LOW);  
        analogWrite(pwmSpeedPinB, 0);      

        if(onStraight() && currentStatus != forward)
        {
            SetRobotState(forward);
        }


    } else {
        // // Draai naar rechts
        // digitalWrite(directionPinA, LOW);  
        // digitalWrite(brakePinA, LOW);       
        // analogWrite(pwmSpeedPinA, pwmSpeed);     

        // digitalWrite(directionPinB, LOW);   
        // digitalWrite(brakePinB, LOW);       
        // analogWrite(pwmSpeedPinB, pwmSpeed);     
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
            if(WallDetected(followLeft) )
            {
                driveForward();
            }
            else
            {
                fullStop();
            }
            break;
        case leftDetected:
            Serial.println("Left turn detected!");
                turn(true);
            break;
        case rightDetected:
            Serial.println("Right turn detected!");
            // if(WallDetected(followLeft) && !WallDetected(!followLeft)) //If there is a wall to the left but not the right, turn right.
            //     {
            //         fullStop();
            //         turn(!followLeft);
            //     }
            break;
        case crossingDetected:
            Serial.println("Crossing (Or endpoint) detected!");
            turn(followLeft);
            break;
        case stopped:
            Serial.println("I have stopped!");
            // turn(followLeft);
            // turn(followLeft); //Call turn twice to rotate 180 degrees
            driveBackward();
            delay(25); //busywait might not be allowed but works for now
            fullStop();
            if(onCross())
            {
                //Either at crossing or endpoint
                //if crossing > turn in followdir
                //else > stop SolveMaze(); and go idle

                SolveMaze(); //temp
            }
            else if(onStraight())
            {
                //Dead end, turn around
                SolveMaze(); //temp
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

void SetRobotState(robotStatus newState)
{
    fullStop();
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
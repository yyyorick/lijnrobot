
int start = 0;

void setup() {
    Serial.begin(9600);
    
}
void loop() {
    
}
bool readSensor(){
    bool sensorValues[5] = {false, false, false, false, false};
    for(int i = 0; i < 5; i++){
        sensorValues[i] = digitalRead(i);
    }
    return sensorValues;
}
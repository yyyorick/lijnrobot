bool sensorValues[] = {false, false, false, false, false};
int start = 0;

void setup() {
    Serial.begin(9600);

}
void loop() {
    readSensor(sensorValues);
    for (int i = 0; i < 5; i++) {
        Serial.print(sensorValues[i]);
    }
    Serial.println();
    delay(1000);
}
void readSensor(bool sensorValues[]){
    for(int i = 0; i < 5; i++){
        if (analogRead(i) > 500){
            sensorValues[i] = false;
        } else {
            sensorValues[i] = true;
        }
    }
}
int start = 0;

void setup() {
    Serial.begin(9600);
}
void loop() {
    analogRead(A0);
    Serial.print("A0: ");
    Serial.println(analogRead(A0));
    analogRead(A1);
    Serial.print("A1: ");
    Serial.println(analogRead(A1));
    analogRead(A2);
    Serial.print("A2: ");
    Serial.println(analogRead(A2));
    analogRead(A3);
    Serial.print("A3: ");
    Serial.println(analogRead(A3));
    analogRead(A4);
    Serial.print("A4: ");
    Serial.println(analogRead(A4));
    delay(1000);

}
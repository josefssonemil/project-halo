int irPin = 7;  // This is our input pin (IR LED at pin 7)
int sensorOut = HIGH;  // HIGH at No Obstacle
 
void setup() {
 
  pinMode(LED, OUTPUT);
 
  pinMode(irPin, INPUT);
 
  Serial.begin(9600);
 
}
 
void loop() {
 
  sensorOut = digitalRead(irPin);
 
  if (sensorOut == LOW)
 
  {
 
    Serial.println("What is this Obstacle?");
 
    digitalWrite(LED, HIGH);
 
  }
 
  else
 
  {
 
    Serial.println("No Obstacle");
 
    digitalWrite(LED, LOW);
 
  }
 
  delay(200);
 
}

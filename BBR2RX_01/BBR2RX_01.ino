#include <Servo.h> //ESP8266 and arduino only, not ESP32! 

Servo LM;  
Servo RM;  

void setup() 
{
  Serial.begin(9600);
  Serial.println("Servo tester - D3");
  LM.attach(D3); 
  LM.write(90); 
  RM.attach(D4); 
  RM.write(90); 
}

void loop() 
{  
  Serial.println("In Loop");
  delay(1000);
  RM.write(0); 
  LM.write(0); 
  delay(1000);
  RM.write(90); 
  LM.write(90); 
  delay(1000);
  RM.write(180); 
  LM.write(180); 
  delay(1000);
  
  
}

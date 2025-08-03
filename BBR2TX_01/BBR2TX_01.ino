//BBR2 TX 

#define LF D6
#define LB D7
#define RF D3
#define RB D4

void setup() {
  Serial.begin(9600);
  Serial.println("TX Swicth Test");
  // put your setup code here, to run once:
  pinMode(LF, INPUT_PULLUP);
  pinMode(LB, INPUT_PULLUP);
  pinMode(RF, INPUT_PULLUP);
  pinMode(RB, INPUT_PULLUP);
}

void loop() 
{
  // put your main code here, to run repeatedly:
  if(digitalRead(LF)==LOW){Serial.println("LF");}
  if(digitalRead(LB)==LOW){Serial.println("LB");}
  if(digitalRead(RF)==LOW){Serial.println("RF");}
  if(digitalRead(RB)==LOW){Serial.println("RB");}
  delay(500);
}

//BBR2 RX - with Wifi

//Servo Stuff
#include <Servo.h> //ESP8266 and arduino only, not ESP32! 
Servo LS;  //NOTICE CHANGE OF NAME
Servo RS;  

//WiFi Stuff
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>


//Safety Cutout - like the big droids, Safety First! 
//time since last message - cut motors if too long. 
unsigned long lastMillis=0;  //some global variables available anywhere in the program
unsigned long currentMillis=0;
unsigned long maxLag=1000;

//incoming wifi message - Control signals
typedef struct struct_message {
    char a[32];
    int LM;
    int RM;
    int DM;
} struct_message;
struct_message droidCommands;


void setup() 
{
  Serial.begin(9600);
  Serial.println("BBR2 RX... Starting...");
  
  setupWifi();
    
  LS.attach(D3); 
  LS.write(90); 
  RS.attach(D4); 
  RS.write(90); 
}


void setupWifi()
{  
  WiFi.disconnect();
  ESP.eraseConfig();
 
  // Wifi STA Mode
  WiFi.mode(WIFI_STA);
  // Get Mac Add
  Serial.print("Mac Address: ");
  Serial.print(WiFi.macAddress());
  Serial.println("\nESP-Now Receiver");

  // Initializing the ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Problem during ESP-NOW init");
    return;
  }
  Serial.println("\nESP-Now Receiver initialised...");
  
  //esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  // We can register the receiver callback function
  esp_now_register_recv_cb(onDataReceiver);
  
}


void loop() 
{  
  Serial.println("In Loop");
  
  int LM = droidCommands.LM;  
  int RM = droidCommands.RM;
  int DM = droidCommands.DM;

  if(millis() - lastMillis>maxLag)//too long since last message... shut down! enter idle mode! 
  {
    Serial.println("Lagging!");    
    LM = 0;
    RM = 0;
  }

  //Go Droid!!!
  if(LM == -1){LS.write(0);}  
  if(LM == 0){LS.write(90);}
  if(LM == 1){LS.write(180);}

  if(RM == -1){RS.write(180);}   
  if(RM == 0){RS.write(90);} 
  if(RM == 1){RS.write(0);}

}


void onDataReceiver(uint8_t * mac, uint8_t *incomingData, uint8_t len) 
{
    lastMillis = millis(); //safety Check
    //Serial.println("Message received.");
    //We don't use mac to verify the sender
    //read incomingData into instruction structure
    memcpy(&droidCommands, incomingData, sizeof(droidCommands));
}

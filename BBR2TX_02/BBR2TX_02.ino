//BBR2 TX - with Wifi

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

#define LF D6
#define LB D7
#define RF D3
#define RB D4


bool rxConnection = false; 
//Mac Address: C4:5B:BE:4B:11:16 - find by running the RX code and check the serial monitor
//then copy into broadcase address

uint8_t broadcastAddress[] = {0xC4, 0x5B, 0xBE, 0x4B, 0x11, 0x16};


// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char a[32]; //for testing, ignore
  int LM;
  int RM;
  int DM;
} struct_message;

// Create a struct_message called droidCommands to hold sensor readings
struct_message droidCommands;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

// Define variables to store incoming readings
float message1;
float message2;

void setup() {
  Serial.begin(9600);
  Serial.println("TX Switch Test");

  Serial.println("Starting ESP...");
  delay(250);
  Serial.println("Starting Wifi...");  
  setupWifi();
  delay(250);
  
  // put your setup code here, to run once:
  pinMode(LF, INPUT_PULLUP);
  pinMode(LB, INPUT_PULLUP);
  pinMode(RF, INPUT_PULLUP);
  pinMode(RB, INPUT_PULLUP);
}

void setupWifi()
{
  Serial.println("Connecting...");
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

}

void loop() 
{
  int dir = 0;
  int LM =0; //Left Motor
  int RM = 0;
  int DM = 0;
  
  // put your main code here, to run repeatedly:
  if(digitalRead(LF)==LOW){Serial.println("LF"); LM = 1;}
  if(digitalRead(LB)==LOW){Serial.println("LB"); LM = -1;}
  if(digitalRead(RF)==LOW){Serial.println("RF"); RM = 1;}
  if(digitalRead(RB)==LOW){Serial.println("RB");RM = -1;}
  delay(500);

  //Build Data Instruction
  String command = "Instruction";
  command.toCharArray(droidCommands.a, 32);
  droidCommands.LM = LM;
  droidCommands.RM = RM;
  droidCommands.DM = DM;

  // Send message via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *) &droidCommands, sizeof(droidCommands));
  delay(50);
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) 
{
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
    rxConnection = true; 
  }
  else{
    Serial.println("Delivery fail");
    rxConnection = false;
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  //extract incoming readings here
}

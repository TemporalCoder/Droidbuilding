//BBR2 RX - with Wifi
//with Mp3

//DFPLayer - New type, with dodgy drivers. 
#include <SoftwareSerial.h>
#include <DFPlayer.h>


//Servo Stuff
#include <Servo.h> //ESP8266 and arduino only, not ESP32! 
Servo LS;  //NOTICE CHANGE OF NAME
Servo RS;  
Servo DS;  

//WiFi Stuff
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

#define MP3_RX_PIN              D6     //GPIO4/D2 to DFPlayer Mini TX
#define MP3_TX_PIN              D7     //GPIO5/D1 to DFPlayer Mini RX
#define MP3_SERIAL_SPEED        9600  //DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_BUFFER_SIZE  32    //software serial buffer size in bytes, to send 8-bytes you need 11-bytes buffer (start byte+8-data bytes+parity-byte+stop-byte=11-bytes)
#define MP3_SERIAL_TIMEOUT      350   //average DFPlayer response timeout 200msec..300msec for YX5200/AAxxxx chip & 350msec..500msec for GD3200B/MH2024K chip

SoftwareSerial mp3Serial;
DFPlayer       mp3;
int numOfTracks = -1;

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
    int SND;
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
  DS.attach(D2); 
  DS.write(90); 

  initDFPlayer();
  randomPlay();
}


void initDFPlayer()
{
  Serial.println("MP3 Test.. initialising");
  
  mp3Serial.begin(MP3_SERIAL_SPEED, SWSERIAL_8N1, MP3_RX_PIN, MP3_TX_PIN, false, MP3_SERIAL_BUFFER_SIZE, 0); //false=signal not inverted, 0=ISR/RX buffer size (shared with serial TX buffer)

  mp3.begin(mp3Serial, MP3_SERIAL_TIMEOUT, DFPLAYER_MINI, false); //"DFPLAYER_MINI" see NOTE, false=no response from module after the command
  mp3.stop();                             //if player was runing during ESP8266 reboot
  mp3.reset();                            //reset all setting to default
  mp3.setSource(2);                       //1=USB-Disk, 2=TF-Card, 3=Aux, 4=Sleep, 5=NOR Flash
  mp3.setEQ(0);                           //0=Off, 1=Pop, 2=Rock, 3=Jazz, 4=Classic, 5=Bass
  mp3.setVolume(25);                      //0..30, module persists volume on power failure
  mp3.sleep();                            //inter sleep mode, 24mA
  mp3.wakeup(2);                          //exit sleep mode & initialize source 1=USB-Disk, 2=TF-Card, 3=Aux, 5=NOR Flash
  mp3Serial.enableRx(true);               //enable interrupts on RX-pin for better response detection, less overhead than mp3Serial.listen()

  Serial.println(mp3.getStatus());        //0=stop, 1=playing, 2=pause, 3=sleep or standby, 4=communication error, 5=unknown state
  Serial.println(mp3.getVolume());        //0..30
  Serial.println(mp3.getCommandStatus()); //1=module busy, 2=module sleep, 3=request not fully received, 4=checksum not match
                                          //5=requested folder/track out of range, 6=requested folder/track not found
                                          //7=advert available while track is playing, 8=SD card not found, 9=???, 10=module sleep
                                          //11=OK command accepted, 12=OK playback completed, 13=OK module ready after reboot
  
  numOfTracks = (int)mp3.getTotalTracksSD();
  
  mp3Serial.enableRx(false);              //disable interrupts on RX-pin, less overhead than mp3Serial.listen()
  Serial.print("NumOfTracks: ");
  Serial.println(numOfTracks);
 
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

void randomPlay()
{
  int randTrack = random(1, 10);
  Serial.println("Track: ");
  Serial.println(randTrack); 
  mp3.playTrack(randTrack);     //play track #1, don’t copy 0003.mp3 and then 0001.mp3, because 0003.mp3 will be played firts
  //delay(3000);         //play for 3 seconds
 }
 


void loop() 
{  
  Serial.println("In Loop");
  
  int LM = droidCommands.LM;  
  int RM = droidCommands.RM;
  int DM = droidCommands.DM;
  int SND = droidCommands.SND;



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

  if(DM == 0){DS.write(90);} 
  if(DM == 1){DS.write(0);}

  if(SND == 1){randomPlay();}

}


void onDataReceiver(uint8_t * mac, uint8_t *incomingData, uint8_t len) 
{
    lastMillis = millis(); //safety Check
    //Serial.println("Message received.");
    //We don't use mac to verify the sender
    //read incomingData into instruction structure
    memcpy(&droidCommands, incomingData, sizeof(droidCommands));
}

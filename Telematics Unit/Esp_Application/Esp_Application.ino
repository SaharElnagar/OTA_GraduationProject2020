
/***************************************************************************/
/*                          Include libraries                              */
/***************************************************************************/
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include<SoftwareSerial.h>

/***************************************************************************/
/*                        Enum Type definitions                            */
/***************************************************************************/
typedef uint8_t ModuleState_Type ;
#define  IDLE_STATE               ((ModuleState_Type)0U)
#define  DOWNLOAD_FILE            ((ModuleState_Type)1U)
#define  SEND_UPDATE_REQ          ((ModuleState_Type)2U)
#define  SEND_FILE_SIZE           ((ModuleState_Type)3U)
#define  WAIT_GATEWAY_RESPONSE    ((ModuleState_Type)4U)
#define  SEND_PACKET              ((ModuleState_Type)5U)
#define  RETRANSMIT_PACKET        ((ModuleState_Type)6U)
#define  END_STATE                ((ModuleState_Type)7U)

/***************************************************************************/
/*                          Macro definitions                              */
/***************************************************************************/
/*Hager*/
#define FIREBASE_HOST "esp-ota-c3cfb.firebaseio.com"             // the project name address from firebase id
#define FIREBASE_AUTH "aFVy9k0r7hdFZIG1TvmvSKqAg8tdEW8LPD1CHJY4"       // the secret key generated from firebase

/*Belal*/
//#define FIREBASE_HOST "my-first-project-15e18.firebaseio.com"
//#define FIREBASE_AUTH "7kncqw0MoNuSQ0n8J1QwVt30bKZGCFVfjehcnYw1"

/*Packet size in bytes*/
#define PACKET_SIZE                           1024
#define FIRST_PACKET_SIZE                     36

/*Received messages IDs*/
#define DISCARD_UPDATE                        (0x01U)
#define READY_FOR_UPDATE                      (0x02U)
#define READY_TO_RECEIVE_NEXT_PACKET          (0x03U)
#define RETRANSMIT_LAST_PACKET                (0x04U)

/*Transmitted messages IDs*/
#define UPDATE_REQ                            (0x05U)

/***************************************************************************/
/*                           Global variables                              */
/***************************************************************************/
/*
* Started SoftwareSerial at RX and TX pin of ESP8266/NodeMCU
* Tx -> GPIO pin 1
* RX -> GPIO pin 3
*/
SoftwareSerial s(3,1) ;

String FIRMWARE_URL = "" ;
String Fingerprint = "" ;
const char* ssid = "TE-Data-mok";
const char* password = "0102217147m";
String new_update = "false";
int led = 2;
int ReadOnce = 0 ;

ModuleState_Type ModuleState = IDLE_STATE ;
uint8_t Rec_Msg ;
uint32_t PacketNum = 0 ;
uint16_t Extra_Bytes = 0 ;
uint8_t ByteBuffer ;
uint32_t FileSize ;
uint32_t NumOfPackets = 0 ;
uint8_t thousands = 0, hundreds = 0, tens = 0, c = 0 ;
uint8_t FirstPacket = 1 ;

/***************************************************************************/
/*                        Functions Implementation                         */
/***************************************************************************/

void setup() {

  s.begin(9600) ;
  pinMode(led, OUTPUT) ;                 
  digitalWrite(led, HIGH) ;
  
  SPIFFS.begin();
  // connect to firebase
  wifiConnect() ;  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH) ;
}

void loop() {
  
  switch(ModuleState)
  {
/*********************IDLE STATE*****************************/
    case IDLE_STATE :
       digitalWrite(led, LOW) ;
       /* Wait for new update */
       new_update = Firebase.getString("new_update") ;
       if (new_update == "true" && ReadOnce == 0)
       {
          FirstPacket = 1 ;
          ReadOnce = 1 ;
          ModuleState = DOWNLOAD_FILE ;
          digitalWrite(led, HIGH) ; 
       }
       else{}
       
       break ;
/*******************DOWNLOAD_FILE STATE**********************/
    case DOWNLOAD_FILE :

       //Firebase.setString("new_update", "false");
       FIRMWARE_URL = Firebase.getString("FIRMWARE_URL");
       Fingerprint = Firebase.getString("Fingerprint");
       Download_Firmware();
       ModuleState = SEND_UPDATE_REQ ;
       
       break ;
/****************SEND_UPDATE_REQ STATE**********************/      
    case SEND_UPDATE_REQ :
    
       /* Send update request to Gateway node */
       s.write(UPDATE_REQ) ;
       ModuleState = WAIT_GATEWAY_RESPONSE ;
       break ;
/*************WAIT_GATEWAY_RESPONSE STATE*******************/    
    case WAIT_GATEWAY_RESPONSE :
       
       Rec_Msg = s.read() ;
       if(Rec_Msg == DISCARD_UPDATE)
       {
          ModuleState = IDLE_STATE ;
          s.flush();
       }
       else if(Rec_Msg == READY_FOR_UPDATE)
       {
          ModuleState = SEND_FILE_SIZE ;
          s.flush();
       }
       else if(Rec_Msg == READY_TO_RECEIVE_NEXT_PACKET)
       {
          ModuleState = SEND_PACKET ;
          s.flush();
       }
       else if(Rec_Msg == RETRANSMIT_LAST_PACKET)
       {
          ModuleState = RETRANSMIT_PACKET ;
          s.flush();
       }
       else
       {/*Do nothing*/
       }
       break ;
/******************SEND_FILE_SIZE STATE*********************/
    case SEND_FILE_SIZE :
    
       thousands = FileSize/1000;
       hundreds = (FileSize % 1000) / 100 ;
       tens = (FileSize % 1000) % 100 ;

       if(c == 0 && s.available() == 0)
       {
         s.write(thousands) ;
         c = 1 ;
       }
       else if(c == 1 && s.available() == 0)
       {
         s.write(hundreds) ;
         c = 2 ;
       }
       else if(c == 2 && s.available() == 0)
       {
         s.write(tens) ;
         ModuleState = WAIT_GATEWAY_RESPONSE ;
         c = 0 ;
       }
       break ;
/*******************SEND_PACKET STATE***********************/       
    case SEND_PACKET :

       if(FirstPacket == 1)
       {
          Send_First_Packet() ;
          FirstPacket = 0 ;
       }
       else
       { 
         Send_Packet() ;
       }
       if(FileSize == 0)
       {
          ModuleState = END_STATE ;
       }
       ModuleState = WAIT_GATEWAY_RESPONSE ;
       
       break ;
/*****************RETRANSMIT_PACKET STATE********************/       
    case RETRANSMIT_PACKET :
    
       PacketNum-- ;
       NumOfPackets++;
       if(FileSize == 0 && Extra_Bytes != 0)
       {
          FileSize += Extra_Bytes ;
       }
       else
       {
          FileSize += PACKET_SIZE ;
          PacketNum-- ;
          NumOfPackets++ ;
          
       }
       ModuleState = SEND_PACKET ;
       break ;
/***********************END_STATE****************************/     
    case END_STATE :
    
       digitalWrite(led, LOW) ;
       ModuleState = IDLE_STATE ;
       break ;
  }
}

/***************************************************************************/
/*                            Function Declarations                        */
/***************************************************************************/
/****************************************************************************/
/*    Function Name           : wifiConnect                                 */
/*    Function Description    : Connect to wifi                             */
/*    Parameter in            : none                                        */
/*    Parameter inout         : none                                        */
/*    Parameter out           : none                                        */
/*    Return value            : none                                        */
/****************************************************************************/
void wifiConnect() {
 //Serial.print("Connecting to "); Serial.print(ssid);
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   //Serial.print(".");
 }
 //Serial.print("nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

/****************************************************************************/
/*    Function Name           : Download_Firmware                           */
/*    Function Description    : Download firmware and store it into flash   */
/*    Parameter in            : none                                        */
/*    Parameter inout         : none                                        */
/*    Parameter out           : none                                        */
/*    Return value            : none                                        */
/****************************************************************************/
void Download_Firmware() {
  HTTPClient http;
  const char* filename = "/samplefile.bin";
  SPIFFS.format();
  File f = SPIFFS.open(filename, "w");
  if(f)
  {
    http.begin(FIRMWARE_URL ,Fingerprint);
    int httpCode = http.GET();
    if (httpCode <= 0)
    {
        //Serial.printf("HTTP failed, error: %s\n", 
        //http.errorToString(httpCode).c_str());
    return;
    }
    if (httpCode == HTTP_CODE_OK) 
    {
        http.writeToStream(&f);
    }
    //Serial.printf("Content-Length: %d\n", (http.getSize()));
    //Serial.printf("file-size: %d\n", (f.size()));
    FileSize = f.size() - FIRST_PACKET_SIZE ;
    NumOfPackets = f.size() / PACKET_SIZE ;
    Extra_Bytes = f.size() % PACKET_SIZE ;
    
    f.close();
    http.end();
   }else
   {
    //Serial.println("file open failed");
    return;
   }
   //Serial.println();
   //Serial.println("Done");
}

/****************************************************************************/
/*    Function Name           : Send_Packet                                 */
/*    Function Description    : sends 1024 byte into serial pin TX (pin1)   */
/*    Parameter in            : none                                        */
/*    Parameter inout         : none                                        */
/*    Parameter out           : none                                        */
/*    Return value            : none                                        */
/****************************************************************************/
void Send_Packet()
{
   const char* filename = "/samplefile.bin";
   uint16_t counter = 0 ;
   File f = SPIFFS.open(filename, "r");
   if (!f) 
   {
      /* File Open Failed */
      return;
   }
   else
   {
       if(NumOfPackets > 0)
       { 
         //seek cursor inside file from begining of file into start address of packet
         f.seek((PacketNum * PACKET_SIZE + FIRST_PACKET_SIZE) , SeekSet);
         while(counter < PACKET_SIZE)
         {
             digitalWrite(led, HIGH);
             delay(5);
             digitalWrite(led, LOW);
             delay(5);
            if(s.available() == 0)
            {
               /*read 1 byte from file into ByteBuffer variable*/
               f.read(&ByteBuffer, 1) ;
               /*write value of ByteBuffer into serial pin TX*/
               s.write(ByteBuffer);
               counter++;
            }
         }
         PacketNum++ ;
         NumOfPackets-- ;
         FileSize -= PACKET_SIZE ;
       }
       else
       { 
         //seek cursor inside file from begining of file into start address of packet
         f.seek((PacketNum * PACKET_SIZE + FIRST_PACKET_SIZE) , SeekSet);
         while(counter < Extra_Bytes)
         {

             digitalWrite(led, HIGH);
             delay(5);
             digitalWrite(led, LOW);
             delay(5);
            if(s.available() == 0)
            {
               /*read 1 byte from file into ByteBuffer variable*/
               f.read(&ByteBuffer, 1) ;
               /*write value of ByteBuffer into serial pin TX*/
               s.write(ByteBuffer);
               counter++;
               
            }
         }
         FileSize -= Extra_Bytes ;
       }
       f.close();  //Close file
   }
}

/****************************************************************************/
/*    Function Name           : Send_First_Packet                           */
/*    Function Description    : sends first packet into serial pin TX (pin1)*/
/*    Parameter in            : none                                        */
/*    Parameter inout         : none                                        */
/*    Parameter out           : none                                        */
/*    Return value            : none                                        */
/****************************************************************************/
void Send_First_Packet()
{
   const char* filename = "/samplefile.bin";
   uint16_t counter = 0 ;
   
   File f = SPIFFS.open(filename, "r");
   if (!f) 
   {
      /* File Open Failed */
      return;
   }
   else
   {
      while(counter < FIRST_PACKET_SIZE)
      {
         digitalWrite(led, HIGH);
         delay(5);
         digitalWrite(led, LOW);
         delay(5);
         if(s.available() == 0)
         {
            /*read 1 byte from file into ByteBuffer variable*/
            f.read(&ByteBuffer, 1) ;
            /*write value of ByteBuffer into serial pin TX*/
            s.write(ByteBuffer);
            counter++ ;
         }
      }
   }
}

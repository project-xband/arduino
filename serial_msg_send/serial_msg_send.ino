#include <SPI.h>
#include <RH_RF22.h>

#define CLEAR 0
#define STARTED 1
#define SYNC_CHARACTER '{'
#define TERMINATOR_CHARACTER '}'

unsigned char incomingByte;   // for incoming serial data
unsigned char reliableLength;
unsigned char commandIndex = 0;
unsigned char commandState = CLEAR;
unsigned char reliableMode = false;

#define INPUT_LENGTH 250
unsigned char commandBuffer[INPUT_LENGTH + 2];

// Singleton instance of the radio driver
RH_RF22 rf22;

void setup() 
{
  Serial.begin(9600);
  if (!rf22.init())
    Serial.println("init failed");
}

void loop()
{

  // Write to Serial
  // Send a message to a rf22_mesh_server
  // A route to the destination will be automatically discovered.
  while(Serial.available() > 0) {
     incomingByte = Serial.read();

     if (CLEAR == commandState) {
       if (SYNC_CHARACTER == incomingByte) {  // look for start of command sync character
         commandState = STARTED;
         continue;
       }
     }

     if (STARTED == commandState) {      
       if (TERMINATOR_CHARACTER == incomingByte) {  // look for end of command terminator character
         commandBuffer[commandIndex] = 0;  // null terminate the command buffer
         Serial.println("[found end]");
         rf22.send(commandBuffer, commandIndex);
         commandIndex = 0;
         commandState = CLEAR;
         return;
       }
       else {
         commandBuffer[commandIndex] = incomingByte;  // store a command character in buffer
         commandIndex++;
         if (INPUT_LENGTH < commandIndex) {
           Serial.println("{overflow}");
           commandIndex = 0;
           commandState = CLEAR;
           return;
         }
       }
     }      
  }
  
//  Serial.println("Sending to rf22_server");
//  // generate max msg len data
//  uint8_t data[RH_RF22_MAX_MESSAGE_LEN];
//  for (int i = 0; i < sizeof(data); i++)
//  {
//      data[i] = 'a';
//  }
//  
//  rf22.send(data, sizeof(data));
  
//  rf22.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[INPUT_LENGTH + 2];
  uint8_t len = sizeof(buf);

  // Should be a reply message for us now   
  if (rf22.recv(buf, &len))
  {
      buf[len] = '\0';
      Serial.print("{");
      Serial.print((char*)buf);
      Serial.println("}");
  }
}


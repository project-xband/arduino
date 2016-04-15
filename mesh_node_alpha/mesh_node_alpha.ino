// rf22_mesh_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, routed reliable messaging client
// with the RHMesh class.
// It is designed to work with the other examples rf22_mesh_server*
// Hint: you can simulate other network topologies by setting the 
// RH_TEST_NETWORK define in RHRouter.h

// Mesh has much greater memory requirements, and you may need to limit the
// max message length to prevent wierd crashes
#define RH_MESH_MAX_MESSAGE_LEN 50

#include <RHMesh.h>
#include <RH_RF22.h>
#include <SPI.h>

// In this small artifical network of 4 nodes,
#define ALPHA_ADDRESS 1
#define BETA_ADDRESS 2
#define CHARLIE_ADDRESS 3
#define DELTA_ADDRESS 4

#define CLEAR 0
#define STARTED 1
#define SYNC_CHARACTER '{'
#define TERMINATOR_CHARACTER '}'

unsigned char incomingByte;   // for incoming serial data
unsigned char reliableLength;
unsigned char commandIndex = 0;
unsigned char commandState = CLEAR;
unsigned char reliableMode = false;

#define INPUT_LENGTH 50
unsigned char commandBuffer[INPUT_LENGTH + 2];

// Singleton instance of the radio driver
RH_RF22 driver;

// Class to manage message delivery and receipt, using the driver declared above
RHMesh manager(driver, ALPHA_ADDRESS);

void setup() 
{
  Serial.begin(9600);
  if (!manager.init())
    Serial.println("init failed");
}

// set up for serial write message
//uint8_t data[INPUT_LENGTH+1];

// Dont put this on the stack:
uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];


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
         if (manager.sendtoWait(commandBuffer, commandIndex, 255) == RH_ROUTER_ERROR_NONE)
          {
//            Serial.println("[sent]");
          }
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

  // read incomming message print to serial
  uint8_t len = sizeof(buf);
  uint8_t from; 
  if (manager.recvfromAck(buf, &len, &from))
  {
    Serial.print("{");
    Serial.print((char*)buf);
    Serial.println("}");
  }

  
}


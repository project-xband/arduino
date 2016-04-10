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
#define CLIENT1_ADDRESS 1
#define CLIENT2_ADDRESS 2
#define SERVER3_ADDRESS 3
#define CLIENT4_ADDRESS 4

// Singleton instance of the radio driver
RH_RF22 driver;

// Class to manage message delivery and receipt, using the driver declared above
RHMesh manager(driver, SERVER3_ADDRESS);

void setup() 
{
  Serial.begin(9600);
  if (!manager.init())
    Serial.println("init failed");
}

// Dont put this on the stack:
uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];


void loop()
{ 

  // Send a message to a rf22_mesh_server
  // A route to the destination will be automatically discovered.
  
  uint8_t len = sizeof(buf);
  uint8_t from;
  if (manager.recvfromAck(buf, &len, &from))
  {
    Serial.print("[0x");
    Serial.print(from, HEX);
    Serial.print("]: ");
    Serial.println((char*)buf);
  }
  
}


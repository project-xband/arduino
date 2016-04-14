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

// packet size limit from serial
#define INPUT_SIZE      30

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
String  serial_read;
int     address;
uint8_t data[INPUT_SIZE+1];

// Dont put this on the stack:
uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];


void loop()
{ 
  // Write to Serial
  // Send a message to a rf22_mesh_server
  // A route to the destination will be automatically discovered.
  while(Serial.available()) {
    address = Serial.parseInt();
    Serial.readBytes(data,30);
    Serial.println();
    Serial.println("[read]");

    if (manager.sendtoWait(data, sizeof(data), 255) == RH_ROUTER_ERROR_NONE)
    {
        Serial.println("[sent]");
    }
    
  }

  // read incomming message print to serial
  uint8_t len = sizeof(buf);
  uint8_t from; 
  uint8_t broadcast;
  broadcast = 255;
  if (manager.recvfromAck(buf, &len, &from))
  {
    Serial.print("[");
    Serial.print(from, HEX);
    Serial.print("]: ");
    Serial.println((char*)buf);
  }

  
}


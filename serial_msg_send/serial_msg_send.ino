#include <SPI.h>
#include <RH_RF22.h>

//#define DEBUG_MODE

#define CLEAR 0
#define STARTED 1
#define SYNC_CHARACTER '{'
#define TERMINATOR_CHARACTER '}'
#define INPUT_LENGTH 250

unsigned char incomingByte;   // read serial input
unsigned char commandIndex = 0;
unsigned char commandState = CLEAR;
//unsigned char reliableLength;
//unsigned char reliableMode = false;
unsigned char commandBuffer[INPUT_LENGTH + 2];

// instance of the radio driver
RH_RF22 rf22;

void setup() 
{
  Serial.begin(9600);
  if (!rf22.init())
    Serial.println("init failed");
}

void framePacket (byte * * ppFramedData, word * pFramedLength, byte * pPacketData, byte packetLength)
{
  word index;
  byte * pFramedData;
  byte * pData;
  word calculatedLength;

  for (index = 0, calculatedLength = 0, pData = pPacketData; index < packetLength; index++, pData++)
  {
    switch (*pData)
    {
        case '%':
        case '{':
        case '}':
            calculatedLength += 2;
            break;
        default:
            calculatedLength++;
    }
  }
  
  calculatedLength += 2;  // add length of opening and closing curly braces
  
  pFramedData = (byte *) malloc (calculatedLength + 2);
  
  *ppFramedData = pFramedData;
  
  *pFramedData++ = '{';
  
  for (index = 0, pData = pPacketData; index < packetLength; index++, pData++)
  {
      switch (*pData)
      {
          case '%':
              *pFramedData++ = '%';
              *pFramedData++ = '%';
              break;
          case '{':
              *pFramedData++ = '%';
              *pFramedData++ = '[';
              break;
          case '}':
              *pFramedData++ = '%';
              *pFramedData++ = ']';
              break;
          default:
              *pFramedData++ = *pData;
              break;
      }
  }
  *pFramedData++ = '}';
  *pFramedLength = calculatedLength;
}

void extractPacket (byte * * ppPacketData, byte * pPacketLength, byte * pFramedPacketData, word framePacketLength, word * pConsumedLength)
{
  word index;
  byte * pFramedData;
  byte * pEndOfFramedData;
  word calculatedLength = 0;
  byte * pUnframedData;
  
  *ppPacketData = NULL;
  
  pEndOfFramedData = pFramedPacketData + framePacketLength;
  pFramedData = pFramedPacketData;

  if ('{' == *pFramedData)
  {
      pFramedData++;

      for ( ; pFramedData < pEndOfFramedData; pFramedData++)
      {
          if ('%' == *pFramedData)
          {
              pFramedData++;
          }
          else
          {
              if ('}' == *pFramedData)
              {
                  *pConsumedLength = (pFramedData - pFramedPacketData) - 1; // calculate how many bytes we consumed
                  goto FoundClosingFrame;
              }
          }
          calculatedLength++;
      }
  // if we exit the for loop without finding the closing brace (frame) this is an error
      return;
        

FoundClosingFrame:
        
// NTR check for alloc failure
      pUnframedData = (byte *) malloc (calculatedLength + 2);
      *ppPacketData = pUnframedData;
      *pPacketLength = calculatedLength;
      
      pFramedData   = pFramedPacketData + 1;  // start after opening brace
  
      for (index = 0; index < calculatedLength; index++)
        {
          if ('%' == *pFramedData)
          {
              pFramedData++;
              
              switch (*pFramedData)
              {
                case '[':
                  *pUnframedData++ = '{'; 
                  break;
                case ']':
                  *pUnframedData++ = '}';
                  break;
                default:
                  *pUnframedData++ = *pFramedData;
                  break;
              }
              pFramedData++;
          }
          else
          {
            *pUnframedData++ = *pFramedData++;
          }
        }
    }
}

void loop()
{

  int index;
  byte *pPacketData;
  byte packetLength;
  word consumedBytes;
  
  byte * pFramedPacketData;
  word framedPacketLength;
  
  // read from serial
  // send a message to a AP/server
  if (Serial.available() > 0)
  {
    
    incomingByte = Serial.read();
    // framePacket(byte incomingByte, byte sizeof(incomingByte));
    
    if (commandState == CLEAR)
    {
       if (SYNC_CHARACTER == incomingByte) // look for start of command sync character
       {
         commandState = STARTED;
       }
     }

     if (commandState == STARTED)
     {      
       if (TERMINATOR_CHARACTER == incomingByte) // look for end of command terminator character
       {
        
         commandBuffer[commandIndex] = incomingByte;  // store a command character in buffer
         commandIndex++;
         commandBuffer[commandIndex] = 0;  // null terminate the command buffer

#ifdef DEBUG_MODE
         Serial.print("sending: ");
         Serial.print((char*)commandBuffer);
         Serial.println(" ");
#endif

         extractPacket(& pPacketData, & packetLength, commandBuffer, commandIndex, & consumedBytes);
         rf22.send(pPacketData, packetLength);
         rf22.waitPacketSent();
         free (pPacketData);

#ifdef DEBUG_MODE
         *(pPacketData + packetLength) = 0;
         Serial.print("send complete: ");
         Serial.print((char*)commandBuffer);
         Serial.println(" ");
#endif

//         rf22.send(commandBuffer, commandIndex +1);
//         rf22.waitPacketSent();
         
         commandIndex = 0;
         commandState = CLEAR;
       }
       else
       {
         commandBuffer[commandIndex] = incomingByte;  // store a command character in buffer
         commandIndex++;
         if (INPUT_LENGTH < commandIndex)
         {
           Serial.println("{overflow}");
           commandIndex = 0;
           commandState = CLEAR;
         }
       }
     }
  }

  if (rf22.available())
  {
    // Now wait for a reply
    uint8_t buf[INPUT_LENGTH +2];
    uint8_t len = INPUT_LENGTH;
  
    // Should be a reply message for us now
    if (rf22.recv(buf, &len))
    {

#ifdef DEBUG_MODE
      buf[len] = '\0';
      Serial.print("=");
      Serial.print((char*)buf);
      Serial.println("=");
#endif

      framePacket(& pFramedPacketData, & framedPacketLength, buf, len);

#ifdef DEBUG_MODE
      Serial.print("==");
      *(pFramedPacketData + framedPacketLength) = 0;
      Serial.print((const char*)pFramedPacketData);
      Serial.println("==");
#else
      for (index =0; index < framedPacketLength; index++)
      {
        Serial.write(pFramedPacketData[index]);
      }
#endif

      free (pFramedPacketData);
    }
  }
}


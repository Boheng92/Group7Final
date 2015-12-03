#include <Servo.h>
#include <XBee.h>
#include <SoftwareSerial.h>
#include "stdlib.h"
 
Servo wheels; // servo for turning the wheels
Servo esc; // not actually a servo, but controlled like one!
bool startup = true; // used to ensure startup only happens once
int startupDelay = 1000; // time to pause at each calibration step
double maxSpeedOffset = 45; // maximum speed magnitude, in servo 'degrees'
double maxWheelOffset = 85; // maximum wheel turn magnitude, in servo 'degrees'
double wheelOffset = 0.0; // For Adjusting the wheel

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();

// create reusable response objects for responses we expect to handle 
ZBRxResponse rx = ZBRxResponse();

SoftwareSerial xbeeSerial(2,3);

char grabOperation()
{
  xbee.readPacket();
  
  char result = 'N';
  
  if (xbee.getResponse().isAvailable()) 
  {
    // got something
           
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) 
    {
      // got a zb rx packet
        
      // now fill our zb rx class
      xbee.getResponse().getZBRxResponse(rx);
      
      Serial.println("Got an rx packet!");
            
      if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) 
      {
        // the sender got an ACK
        Serial.println("packet acknowledged");
      } 
      else 
      {
        Serial.println("packet not acknowledged");
      }
        
      Serial.print("checksum is ");
      Serial.println(rx.getChecksum(), HEX);

      Serial.print("packet length is ");
      Serial.println(rx.getPacketLength(), DEC);
        
      for (int i = 0; i < rx.getDataLength(); i++) 
      {
        Serial.print("payload [");
        Serial.print(i, DEC);
        Serial.print("] is ");
        Serial.println(rx.getData()[i], HEX);
      }
        
      for (int i = 0; i < xbee.getResponse().getFrameDataLength(); i++) 
      {
        Serial.print("frame data [");
        Serial.print(i, DEC);
        Serial.print("] is ");

        // Here is the data part loaded in server.js(from [11])
        // Only For testing
        if(i >= 11)
        {
          Serial.print("DATA [");
          Serial.print((i-11), DEC);
          Serial.print("]:");
          Serial.println( (char)(xbee.getResponse().getFrameData()[i]));
        }
        else
        {
          Serial.println(xbee.getResponse().getFrameData()[i], HEX);
        }

        // Only the First Character is needed
        if(xbee.getResponse().getFrameDataLength() > 11)
        {
          result = xbee.getResponse().getFrameData()[11];
        }
      }
 
      Serial.println("");
        
      }
    } 
    else if (xbee.getResponse().isError()) 
    {
      Serial.print("error code:");
      Serial.println(xbee.getResponse().getErrorCode());
    }
}

void setup()
{
  wheels.attach(8); // initialize wheel servo to Digital IO Pin #8
  esc.attach(9); // initialize ESC to Digital IO Pin #9
  /*  If you're re-uploading code via USB while leaving the ESC powered on, 
   *  you don't need to re-calibrate each time, and you can comment this part out.
   */
  Serial.begin(9600);

  xbeeSerial.begin(9600);
  xbee.setSerial(xbeeSerial);
  Serial.println("Initializing Crawler...");
   
   calibrateESC();
}

/* Convert degree value to radians */
double degToRad(double degrees){
  return (degrees * 71) / 4068;
}

/* Convert radian value to degrees */
double radToDeg(double radians){
  return (radians * 4068) / 71;
}

/* Calibrate the ESC by sending a high signal, then a low, then middle.*/
void calibrateESC(){
    esc.write(180); // full backwards
    delay(startupDelay);
    esc.write(0); // full forwards
    delay(startupDelay);
    esc.write(90); // neutral
    delay(startupDelay);
    esc.write(90); // reset the ESC to neutral (non-moving) value
}

/* Oscillate between various servo/ESC states, using a sine wave to gradually 
 *  change speed and turn values.
 */
void oscillate(){
  for (int i =0; i < 360; i++){
    double rad = degToRad(i);
    double speedOffset = sin(rad) * maxSpeedOffset;
    double wheelOffset = sin(rad) * maxWheelOffset;
//    Serial.println(90 + speedOffset);
    Serial.println(90 + wheelOffset);
//    esc.write(90 + speedOffset);
//    wheels.write(90 + wheelOffset);
    delay(50);
  }
}

void steerRight(double d)
{
//  Serial.write("Steer Right:");
//  Serial.write("\n");
  
  if( (d >= 0.0 ) && (d <= 1.0))
  {
    double temp = min( (d * maxWheelOffset + wheelOffset), maxWheelOffset);
//    Serial.println("temp :  "+ (String)temp);
    
    wheels.write(90 - temp);
  }
}

void steerLeft(double d)
{ 
//  Serial.write("Steer Left:");
//  Serial.write("\n");
  
  if( (d >= 0.0 ) && (d <= 1.0))
  {
    double temp = min( (d * maxWheelOffset + wheelOffset), maxWheelOffset);
    
    wheels.write(90 + temp);
  }
}

void setVelocity(double s)
{
  if( (s >= -1.0 ) && (s <= 1.0))
  {
    esc.write(90 - (s * maxSpeedOffset));
  }
}
 
void loop()
{
  Serial.println("=======loop start=======");
  delay(500);

  boolean rec = false;

  // This is for the API mode
  // TODO: Receive ZB_RX, grab the payload and determine the operations
  char command = grabOperation();

  if(command = 'F')
  {
    setVelocity(0.4);

    rec = true;
  }
  else if(command = 'B')
  {
    setVelocity(-0.2);

    rec = true;
  }
  else if(command = 'L')
  {
    steerLeft(0.8);

    rec = true;
  }
  else if(command = 'R')
  {
    steerRight(0.8);

    rec = true;
  }
  
  if(rec = false)
  {
    setVelocity(0.0);
  }
}



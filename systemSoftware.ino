#include <stdint.h>
#include <Wire.h>
#include "LIDARLite_v4LED.h"
#include <Adafruit_DotStar.h>
#include <SPI.h>
#include "Adafruit_VL53L0X.h"
#define NUMPIXELS 15 // Number of LEDs in strip
#define DATAPIN 4 //Pin on the Arduino to connect the SDA
#define CLOCKPIN 5 //Pin on Arduino to connect the SCL
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG); //defines DotStar Strip
LIDARLite_v4LED sleeve1; //Define the Sleeve Sensor 1
LIDARLite_v4LED sleeve2; //Define the second sleeve sensor
LIDARLite_v4LED rack1; //Define the first rack sensor
LIDARLite_v4LED rack2; //Define the second Rack sensor

enum rangeType_T
{
    RANGE_NONE,
    RANGE_SINGLE,
    RANGE_CONTINUOUS,
    RANGE_SINGLE_GPIO,
    RANGE_CONTINUOUS_GPIO
};

uint16_t distanceRack1; //Defines measurement variables for the rack sensors
uint16_t distanceRack2;
uint8_t  newDistanceRack1;
uint8_t  newDistanceRack2;

uint16_t distanceSleeve1; //Defines measurement variables for the sleeve sensors
uint16_t distanceSleeve2;
uint8_t  newDistanceSleeve1;
uint8_t  newDistanceSleeve2;

double coilDist;
double coilOffsetReso = 3; //Change this in terms of cm for the resolution of the LightBar
int coilOffset;
uint32_t ledColor;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  digitalWrite(SCL, LOW);
  digitalWrite(SDA, LOW);
  rack1.configure(0); //Connects the first rack sensor
  digitalWrite(SCL, LOW);
  digitalWrite(SDA, LOW);
  rack2.configure(0, 0x50); //Connects the second rack sensor
  digitalWrite(SCL, LOW);
  digitalWrite(SDA, LOW);
  sleeve1.configure(0, 0x49); //Connects the first sleeve sensor
  digitalWrite(SCL, LOW);
  digitalWrite(SDA, LOW);
  sleeve2.configure(0, 0x48);//Connects the second sleeve sensor
  Serial.println("Rack Sensor Connected");
  Serial.println("Sleeve Sensor Connected");
  strip.begin(); //Begins port communication with LED LightBar
  strip.show(); //Initializes all LEDs to "off"
  strip.clear(); //Turns all LEDs "off"
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("loop start");
  //Rack Sensing
   newDistanceRack1 = distanceContinuous(&distanceRack1, rack1);
   newDistanceRack2 = distanceContinuous(&distanceRack2, rack2);
   if (newDistanceRack1 | newDistanceRack2)
       {
           Serial.print("Rack Distance: ");
           distanceRack = min(distanceRack1, distanceRack2);
           Serial.println(distanceRack-4);
       }
   //Sleeve Sensing
   newDistanceSleeve1 = distanceContinuous(&distanceSleeve1, sleeve1);
   newDistanceSleeve2 = distanceContinuous(&distanceSleeve2, sleeve2);
   if (newDistanceSleeve1 | newDistanceSleeve2)
       {
           Serial.print("Sleeve Distance: ");
           distanceSleeve = min(distanceSleeve1, distanceSleeve2);
           Serial.println(distanceSleeve-4);
       }
    
   //Calculating using the measurements from above
   coilOffset = (coilDist/coilOffsetReso); //Finding the offset from the perfect placement position
   if((coilDist/coilOffsetReso) > 0){
      coilOffset = floor((coilDist/coilOffsetReso)) + 8;
   }
   else {
    coilOffset = ceil((coilDist/coilOffsetReso)) + 8;
   }
   //Finding the offset in units of resolution defined above
   if(coilOffset > 15){
    coilOffset = 15;
   }
   if(coilOffset < 1){
    coilOffset = 1;
   }
   //Defining which LEDs will be what color
   Serial.print("LED Index: ");
   Serial.println(coilOffset);
   switch(coilOffset) {
        case 1:
        case 2:
        case 14:
        case 15: 
          ledColor = strip.Color(0,255,0); //LEDs 1,2,14,15 are Red
          break;
        case 3:
        case 4:
        case 12:
        case 13: 
          ledColor = strip.Color(255,255,0); //LEDs 3,4,12,13 are Orange
          break;
        case 5:
        case 6:
        case 7:
        case 9:
        case 11:
        case 10: 
          ledColor = strip.Color(255,165,0); //LEDs 5,6,7,9,10,11 are Yellow
          break;
        case 8:
          ledColor = strip.Color(255,0,0); //LED 8 is Green
          break;         
      }
      
      strip.clear(); //Turns all LEDs "off"
      strip.setPixelColor(coilOffset-1, ledColor); //Sets LED[coilOffset] to the specified color
      strip.show(); //Pushes set colors onto LED strip
      Serial.println("------------------"); //New Loop
}
  
//The function that records the measurements from the sensors
uint8_t distanceContinuous(uint16_t * distance, LIDARLite_v4LED rack)
{
    uint8_t newDistance = 0;

    if (rack.getBusyFlag() == 0)
    {
        // Trigger the next range measurement
        rack.takeRange();

        // Read new distance data from device registers
        *distance = rack.readDistance();

        // Report to calling function that we have new data
        newDistance = 1;
    }

    return newDistance;
}

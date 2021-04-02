#include <stdint.h>
#include <Wire.h>
#include "LIDARLite_v4LED.h"
#include <Adafruit_DotStar.h>
#include <SPI.h>
#include "Adafruit_VL53L0X.h"
#define NUMPIXELS 15 // Number of LEDs in strip
#define DATAPIN 4
#define CLOCKPIN 5
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG); //defines DotStar Strip
Adafruit_VL53L0X sleeve = Adafruit_VL53L0X();
LIDARLite_v4LED rack;

enum rangeType_T
{
    RANGE_NONE,
    RANGE_SINGLE,
    RANGE_CONTINUOUS,
    RANGE_SINGLE_GPIO,
    RANGE_CONTINUOUS_GPIO
};

uint16_t distanceRack;
uint8_t  newDistanceRack;

VL53L0X_RangingMeasurementData_t measurementSleeve;

double coilDist;
double coilOffsetReso = 3;
int coilOffset;
uint32_t ledColor;
int initTime;
int endTime;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  digitalWrite(SCL, LOW);
  digitalWrite(SDA, LOW);
  rack.configure(0);
  Serial.println("Rack Sensor Connected");
  if (!sleeve.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  } 
  Serial.println("Sleeve Sensor Connected");
  strip.begin(); //Begins port communication with LED LightBar
  strip.show(); //Initializes all LEDs to "off"
  strip.clear(); //Turns all LEDs "off"
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("loop start");
  initTime = millis();
  //Rack Sensing
   newDistanceRack = distanceContinuous(&distanceRack);
   if (newDistanceRack)
       {
           Serial.print("Rack Distance: ");
           Serial.println(distanceRack-4);
       }
   //Sleeve Sensing
   sleeve.rangingTest(&measurementSleeve, false);
   if (measurementSleeve.RangeStatus != 4) {
    Serial.print("Sleeve Distance: ");
    Serial.println((measurementSleeve.RangeMilliMeter - 55)/10.0);
   }
   coilDist = (distanceRack-4) - ((measurementSleeve.RangeMilliMeter - 55)/10.0);
   Serial.print("Coil Distance: ");
   Serial.println(coilDist);
   coilOffset = (coilDist/coilOffsetReso);
   if((coilDist/coilOffsetReso) > 0){
      coilOffset = floor((coilDist/coilOffsetReso)) + 8;
   }
   else {
    coilOffset = ceil((coilDist/coilOffsetReso)) + 8;
   }

   if(coilOffset > 15){
    coilOffset = 15;
   }
   if(coilOffset < 1){
    coilOffset = 1;
   }
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

      endTime = millis();
      Serial.print("Measured Time: ");
      Serial.println(endTime - initTime);
      Serial.println("------------------");
}
  

uint8_t distanceContinuous(uint16_t * distance)
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

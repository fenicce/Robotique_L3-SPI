#include <Arduino.h>
#include <MeMCore.h> // Remplace par MeAuriga.h ou MeOrion.h selon ta carte
#include "MeRGBLineFollower.h" 

MeRGBLineFollower LightSensorRGB_1(PORT_3); 

void setup() {
  Serial.begin(115200); 
  
  // Balise 1 : Le programme démarre-t-il ?
  Serial.println("--- Demarrage du programme ---"); 
  
  LightSensorRGB_1.begin();
  
  // Balise 2 : Le capteur a-t-il repondu ?
  Serial.println("--- Capteur initialise ---"); 
  
  LightSensorRGB_1.updataAllSensorValue(); 
  LightSensorRGB_1.setKp(1);
}

void loop() {
  LightSensorRGB_1.updataAllSensorValue(); 

  Serial.print("Position : ");
  Serial.println(LightSensorRGB_1.getPositionState());
  
  delay(100); 
}
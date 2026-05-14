#include <Arduino.h>
#include <MeOrion.h> // Assure-toi que c'est bien la bonne carte
#include "MeRGBLineFollower.h" 

// On passe sur le PORT_3 qui gère l'I2C
MeRGBLineFollower LightSensorRGB_1(PORT_3); 

void setup() {
  Serial.begin(115200); 
  LightSensorRGB_1.begin();
  Serial.println("Initialisation terminee");
}

void loop() {
  LightSensorRGB_1.updataAllSensorValue(); 

  Serial.print("Position : ");
  Serial.println(LightSensorRGB_1.getPositionState());
  
  delay(200); // Un délai un peu plus long pour lire confortablement
}
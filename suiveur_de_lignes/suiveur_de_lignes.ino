#include <Arduino.h>
#include <MeOrion.h>
#include "MeRGBLineFollower.h"
#include <Wire.h>
//addresses des moteurs de propulsion
#define MOTEUR_GAUCHE 0x66 //moteur de gauche sur la carte I2C
#define MOTEUR_DROIT 0x68 //moteur de droite sur la carte I2C
//commandes de direction pour les moteurs
#define ARRET 0x00
#define AVANT 0x01
#define ARRIERE 0x02
#define FREIN 0x03

// On passe sur le PORT_3 qui gère l'I2C
MeRGBLineFollower LightSensorRGB_1(PORT_3);
void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(100);
  Serial.println("Test Mini I2C Motor Driver (DRV8830)");
  LightSensorRGB_1.begin();
  delay(100);
  Serial.println("Test line follower");
}

// Fonction pour piloter un moteur
void piloterMoteur(byte adresse, byte direction, byte vitesse) {
  // La vitesse sur ce module va de 0 à 63 (6 bits)
  if (vitesse > 63) vitesse = 63;
  
  // Le registre de contrôle est 0x00
  // On combine la vitesse et la direction dans un seul octet
  byte commande = (vitesse << 2) | direction;
  
  Wire.beginTransmission(adresse);
  Wire.write(0x00);      // Registre de contrôle
  Wire.write(commande);  // Valeur vitesse + direction
  Wire.endTransmission();
}

//fonctions pour commandes de propulsion
void droiteLeger()
{
  piloterMoteur(MOTEUR_GAUCHE, AVANT, 40); // Vitesse 40/63
  piloterMoteur(MOTEUR_DROIT, ARRIERE, 20);
}
void droiteFort()
{
  piloterMoteur(MOTEUR_GAUCHE, AVANT, 50); // Vitesse 40/63
  piloterMoteur(MOTEUR_DROIT, AVANT, 10);
}
void gaucheLeger()
{
  piloterMoteur(MOTEUR_GAUCHE, AVANT, 20); // Vitesse 40/63
  piloterMoteur(MOTEUR_DROIT, ARRIERE, 40);
}
void gaucheFort()
{
  piloterMoteur(MOTEUR_GAUCHE, ARRIERE, 10); // Vitesse 40/63
  piloterMoteur(MOTEUR_DROIT, ARRIERE, 50);
}
void avancer()
{
  piloterMoteur(MOTEUR_GAUCHE, AVANT, 40); // Vitesse 40/63
  piloterMoteur(MOTEUR_DROIT, ARRIERE, 40);
}
void stop()
{
  piloterMoteur(MOTEUR_GAUCHE, AVANT, 0); // Vitesse 40/63
  piloterMoteur(MOTEUR_DROIT, ARRIERE, 0);
}

void loop() {
  // mise a jour des led pour savoir où est la ligne
  LightSensorRGB_1.updataAllSensorValue();
  int position=LightSensorRGB_1.getPositionState();
  Serial.print("position=");
  Serial.println(position);
  //
  if(position==9)
  //          1001
  {
  avancer();
  }
  else if(position==3 || position==1 || position ==11)
  //              0011           0001            1011
  {
    gaucheLeger();
  }
  else if(position==8)
  //              1000
  {
    gaucheFort();
  }
  else if(position==12 || position==8 || position==13)
  //            1100           1000         1101
  {
    droiteLeger();
  }
  else if(position==14)
  //              1110
{
  droiteFort();
}

  else if(position==15)
  //              1111
  {
  stop();
  }
  delay(100);
}

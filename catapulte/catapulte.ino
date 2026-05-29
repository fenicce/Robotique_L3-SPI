#include <Arduino.h>
#include <MeOrion.h>
#include "MeRGBLineFollower.h"
#include <Wire.h>
#include "rgb_lcd.h" // La nouvelle bibliothèque spécifique pour ton écran Grove

// Création de l'objet écran
rgb_lcd lcd;

//addresses des moteurs de propulsion
#define MOTEUR_GAUCHE 0x66 //moteur de gauche sur la carte I2C
#define MOTEUR_DROIT 0x68 //moteur de droite sur la carte I2C
#define MOTEUR_CATA 0x60 //moteur de catapulte
#define MOTEUR_OR 0x65 //moteur de d'inclinaison de tir

//commandes de direction pour les moteurs
#define ARRET 0x00
#define AVANT 0x01
#define ARRIERE 0x02
#define FREIN 0x03

//nombres de dents sur la roue qui sert à determiner l'angle de tir
#define NB_DENTS 60 //a detrminer

//variables globales
const int sigPin=4;//ultrason en D4
volatile long compteur =0; //pour l'angle


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
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), compterImpulsion, RISING);
  lcd.begin(16,2);
  lcd.setRGB(255,255,255);
  lcd.setCursor(0,0);
  lcd.print("Distance : ");
  int angle=0;//angle de la catapulte
}

void compterImpulsion()
{
  compteur++;
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

//fonctions piloter moteur pour la catapulte
void armer()
{
  //remplir
  piloterMoteur(MOTEUR_CATA, ARRIERE, 63);
  delay(6000);//environ 1 tour sans glissement
}



void loop() {
//pour la distance ultrason
  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(sigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(sigPin, LOW);
  pinMode(sigPin, INPUT);
  long duree = pulseIn(sigPin, HIGH, 30000);
  float distance = duree * 0.034 / 2.0;

  if (duree == 0 || distance > 400) {
    Serial.println("PC -> Hors portee");
  } else {
    Serial.print("PC -> Distance : ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  lcd.setCursor(0, 1); 

  if (duree == 0 || distance > 400) {
    lcd.print("Hors portee    "); 
    lcd.setRGB(255, 0, 0); // On met l'écran en ROUGE si c'est hors de portée !
  } else {
    lcd.print(distance);
    lcd.print(" cm      "); 
    lcd.setRGB(0, 255, 0); // On met l'écran en VERT quand on a une mesure !
  }
  //fin ultrason

  //definition angle_voulu
  int angle_voulu=distance*machin + truc//voir plan d'expérience lineair normalement

  //orientation de l'angle de tir
  angle=(2*NB_DENTS)/compteur;
  while(angle != angle_voulu){
    piloterMoteur(MOTEUR_OR, AVANT, 40);
  }
  armer();
  delay(250);
}

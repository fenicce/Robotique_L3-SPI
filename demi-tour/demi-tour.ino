#include <Arduino.h>
#include <MeOrion.h>
#include "MeRGBLineFollower.h"
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_NeoPixel.h>

// Adresses des moteurs de propulsion
#define MOTEUR_GAUCHE 0x66 
#define MOTEUR_DROIT 0x68 
#define PIN_LED 6
#define NUM_LED 30

// Commandes de direction pour les moteurs
#define ARRET 0x00
#define AVANT 0x01
#define ARRIERE 0x02
#define FREIN 0x03

const int sigPin = 4;

Adafruit_NeoPixel ruban = Adafruit_NeoPixel(NUM_LED, PIN_LED, NEO_GRB + NEO_KHZ800);
Adafruit_TCS34725 capteurCouleur = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
MeRGBLineFollower LightSensorRGB_1(PORT_3);

// Prototypes des fonctions
void clignoterLED(int r, int g, int b);
void piloterMoteur(byte adresse, byte direction, byte vitesse);
void avancer();
void stop();
void suivreLigne();
float Distance();

void setup() {
  Serial.begin(115200);
  Serial.println("Demarrage du systeme Cameleon...");
  Wire.begin();
  delay(100);
  Serial.println("Test Mini I2C Motor Driver (DRV8830)");
  LightSensorRGB_1.begin();
  delay(100);
  Serial.println("Test line follower");

  if (!capteurCouleur.begin()) {
    Serial.println("Erreur : Capteur de couleur introuvable.");
    while (1); 
  }

  ruban.begin();
  ruban.setBrightness(50); 
  ruban.show(); // Extinction initiale
}

void loop() {
  float distance = Distance();
  
  Serial.print("Distance : ");
  if (distance >= 400.0 || distance <= 0) {
    Serial.println("Hors de portee (> 4m)");
  } else {
    Serial.print(distance);
    Serial.println(" cm");
  }

  // Objectif : S'arrêter précisément à moins de 3 cm
  if (distance > 0 && distance <= 3.0) {
    stop(); // Arrêt immédiat des moteurs
    Serial.println("Obstacle a 3 cm ! Analyse de la couleur...");
    
    // 1. Lecture des données du capteur TCS34725
    float r, g, b;
    capteurCouleur.getRGB(&r, &g, &b);

    // 2. Détermination de la couleur dominante
    int finalR = 0;
    int finalG = 0;
    int finalB = 0;

    if (r > g && r > b) {
      finalR = 255; 
      Serial.println("Couleur detectee : ROUGE");
    } 
    else if (g > r && g > b) {
      finalG = 255; 
      Serial.println("Couleur detectee : VERT");
    } 
    else if (b > r && b > g) {
      finalB = 255; 
      Serial.println("Couleur detectee : BLEU");
    }

    // 3. Clignotement de la couleur dominante (0,5s allumé / 0,5s éteint pendant 3s)
    clignoterLED(finalR, finalG, finalB);
    demitour();
 
    }
   
  else {
    // Si la voie est libre, le robot suit sa ligne normalement
    suivreLigne(); 
    
    // Le ruban reste éteint pendant le suivi de ligne
    for(int t = 0; t < ruban.numPixels(); t++){
      ruban.setPixelColor(t, ruban.Color(0, 0, 0));
    }
    ruban.show();
  }
}

// Fonction de clignotement stricte : 0,5s ON / 0,5s OFF répétée 3 fois (= 3 secondes au total)
void clignoterLED(int r, int g, int b){
  for(int i = 0; i < 3; i++) {
    // Allumer pendant 0,5 seconde (500 millisecondes)
    for(int t = 0; t < ruban.numPixels(); t++) {
      ruban.setPixelColor(t, ruban.Color(r, g, b));
    }
    ruban.show();
    delay(500);

    // Éteindre pendant 0,5 seconde (500 millisecondes)
    for(int t = 0; t < ruban.numPixels(); t++) {
      ruban.setPixelColor(t, ruban.Color(0, 0, 0));
    }
    ruban.show();
    delay(500);
  }
}

// Lecture du capteur de distance à ultrason (Broche SIG unique)
float Distance(){
  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(sigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(sigPin, LOW);

  pinMode(sigPin, INPUT);
  long duree = pulseIn(sigPin, HIGH, 30000);
  float distance = duree * 0.034 / 2.0;
  return distance;
}

// Commande des moteurs via le driver I2C DRV8830
void piloterMoteur(byte adresse, byte direction, byte vitesse) {
  if (vitesse > 63) vitesse = 63;
  byte commande = (vitesse << 2) | direction;
  
  Wire.beginTransmission(adresse);
  Wire.write(0x00);      
  Wire.write(commande);  
  Wire.endTransmission();
}

void droiteLeger() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT, 40); 
  piloterMoteur(MOTEUR_DROIT, ARRIERE, 20);
}
void droiteFort() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT, 50); 
  piloterMoteur(MOTEUR_DROIT, AVANT, 10);
}
void gaucheLeger() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT, 20); 
  piloterMoteur(MOTEUR_DROIT, ARRIERE, 40);
}
void gaucheFort() {
  piloterMoteur(MOTEUR_GAUCHE, ARRIERE, 10); 
  piloterMoteur(MOTEUR_DROIT, ARRIERE, 50);
}
void avancer() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT, 40); 
  piloterMoteur(MOTEUR_DROIT, ARRIERE, 40);
}
void stop() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT, 0); 
  piloterMoteur(MOTEUR_DROIT, ARRIERE, 0);
}

// Algorithme de suivi de ligne (vitesse de scrutation optimisée à 10ms)
void suivreLigne() {
  LightSensorRGB_1.updataAllSensorValue();
  int position = LightSensorRGB_1.getPositionState();
  
  if(position == 9) {
    avancer();
  }
  else if(position == 3 || position == 1 || position == 11) {
    gaucheLeger();
  }
  else if(position == 8) {
    gaucheFort();
  }
  else if(position == 12 || position == 13) { 
    droiteLeger();
  }
  else if(position == 14) {
    droiteFort();
  }
  else if(position == 15) {
    stop();
  }
  delay(10); 
}

void demitour() {
    piloterMoteur(MOTEUR_GAUCHE, ARRIERE, 40); 
    piloterMoteur(MOTEUR_DROIT, AVANT, 40);
    delay(1000);
    Serial.println("Début du demi-tour initial...");
    piloterMoteur(MOTEUR_GAUCHE, AVANT, 30);
    piloterMoteur(MOTEUR_DROIT, AVANT, 30);
    delay(500);

    int position=15;
    while(position==15){
      LightSensorRGB_1.updataAllSensorValue();
      position = LightSensorRGB_1.getPositionState();
      delay(10);
    }
  stop();
  delay(20);
  Serial.println("Demi-tour terminé avec succès !");
}

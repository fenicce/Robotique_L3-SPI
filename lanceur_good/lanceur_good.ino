#include <Arduino.h>
#include <MeOrion.h>
#include "MeRGBLineFollower.h"
#include <Wire.h>
#include "rgb_lcd.h" 

// Création de l'objet écran
rgb_lcd lcd;

// Adresses des moteurs
#define MOTEUR_GAUCHE 0x66 
#define MOTEUR_DROIT 0x68 
#define MOTEUR_CATA 0x60 
#define MOTEUR_OR 0x65 

#define PIN_SERVO A0
Servo servoUltrason;

// Commandes de direction pour les moteurs
#define ARRET 0x00
#define AVANT 0x01
#define ARRIERE 0x02
#define FREIN 0x03

// Paramètres de l'encodeur filtré
#define tick 5 // Résolution de 5° car on détecte chaque changement d'état (équivalent CHANGE)
const unsigned long delaiFiltrage = 500; // Temps de stabilité requis en microsecondes (ajustable)

#define ANG_MILIEU 70

// Variables globales pour le capteur optique
int etatFiltre = LOW;          
int dernierEtatBrut = LOW;     
unsigned long dernierTempsChangement = 0;
long compteur = 0; 

const int sigPin = 4; // Ultrason en D4

MeRGBLineFollower LightSensorRGB_1(PORT_3);

void piloterMoteur(byte adresse, byte direction, byte vitesse) {
  if (vitesse > 63) vitesse = 63;
  byte commande = (vitesse << 2) | direction;
  
  Wire.beginTransmission(adresse);
  Wire.write(0x00);      
  Wire.write(commande);  
  Wire.endTransmission();
}

void declencher() {
  piloterMoteur(MOTEUR_CATA, ARRIERE, 63);
  delay(3000); // Environ 1 tour sans glissement
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  delay(100);
  Serial.println("Système de tir initialisé - Mode Tir Unique");
  
  LightSensorRGB_1.begin();
  delay(100);
  
  servoUltrason.attach(PIN_SERVO);
  servoUltrason.write(ANG_MILIEU);
  delay(200);

  // Configuration de la broche de l'encodeur avec Pull-up interne
  pinMode(2, INPUT_PULLUP); 
  etatFiltre = digitalRead(2);
  dernierEtatBrut = etatFiltre;

  lcd.begin(16,2);
  lcd.setRGB(255,255,255);
  lcd.setCursor(0,0);
  lcd.print("Distance : ");

  // ==========================================
  // EXÉCUTION DU TIR UNIQUE (ANCIENNE LOOP)
  // ==========================================

  // 1. MESURE DE LA DISTANCE (ULTRASON)
  // On boucle tant qu'on n'a pas une mesure valide (pour éviter de tirer dans le vide au démarrage)
  float distance = 0;
  long duree = 0;
  
  Serial.println("Attente d'une cible valide...");
  while (duree == 0 || distance > 400 || distance < 10) {
    pinMode(sigPin, OUTPUT);
    digitalWrite(sigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(sigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(sigPin, LOW);
    pinMode(sigPin, INPUT);
    duree = pulseIn(sigPin, HIGH, 30000);
    distance = duree * 0.034 / 2.0;

    lcd.setCursor(0, 1); 
    if (duree == 0 || distance > 400) {
      Serial.println("PC -> Hors portee");
      lcd.print("Hors portee    "); 
      lcd.setRGB(255, 0, 0); 
    } else {
      Serial.print("PC -> Distance : ");
      Serial.print(distance);
      Serial.println(" cm");
      lcd.print(distance);
      lcd.print(" cm      "); 
      lcd.setRGB(0, 255, 0); 
    }
    delay(200); // Petite pause entre les recherches de cible
  }

  // 2. CALCUL DE L'ANGLE DE TIR (Modèle linéaire : 100cm -> 70° | 300cm -> 40°)
  float angle_voulu = 85.0 - (0.15 * distance);
  
  // Bornage de sécurité [40° - 70°]
  if (angle_voulu < 40.0) angle_voulu = 40.0;
  if (angle_voulu > 70.0) angle_voulu = 70.0;

  Serial.print("Angle voulu : ");
  Serial.println(angle_voulu);

  // Conversion en nombre de ticks cibles (Départ à 40°)
  int ticks_cibles = round((angle_voulu - 40.0) / (float)tick);
  if (ticks_cibles < 0) ticks_cibles = 0;

  Serial.print("Ticks cibles calcules : ");
  Serial.println(ticks_cibles);

  // 3. ORIENTATION DE LA RAMPE
  compteur = 0; 
  
  if (ticks_cibles > 0) {
    piloterMoteur(MOTEUR_OR, AVANT, 40);

    while (compteur < ticks_cibles) {
      int lectureBrute = digitalRead(2);
      unsigned long tempsActuel = micros();

      if (lectureBrute != dernierEtatBrut) {
        dernierTempsChangement = tempsActuel;
        dernierEtatBrut = lectureBrute;
      }

      if ((tempsActuel - dernierTempsChangement) > delaiFiltrage) {
        if (lectureBrute != etatFiltre) {
          etatFiltre = lectureBrute;
          compteur++; 
          Serial.print("Ticks : ");
          Serial.println(compteur);
        }
      }
    }
    piloterMoteur(MOTEUR_OR, FREIN, 0); // Arrêt net
    delay(500); 
  }

  // 4. LE TIR
  Serial.println("--- TIR ! ---");
  declencher();
  
  Serial.println("Fin du programme. Prêt pour extinction ou reset.");
}

void loop() {
  // La loop est vide : l'Arduino reste allumé mais ne fait plus rien.
  // Pour refaire un tir, il te suffira d'appuyer sur le bouton RESET de la carte.
}
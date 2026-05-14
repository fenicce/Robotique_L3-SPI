#include <Wire.h>
#include "Adafruit_TCS34725.h"

// Initialisation du capteur (temps d'intégration de 50ms et gain de 4x pour une bonne sensibilité)
Adafruit_TCS34725 capteurCouleur = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() {
  Serial.begin(115200);
  Serial.println("\nDemarrage du test de couleur...");

  // On vérifie si le capteur est bien là
  if (capteurCouleur.begin()) {
    Serial.println("Capteur TCS34725 detecte avec succes !");
  } else {
    Serial.println("Capteur introuvable. Verifiez le cablage.");
    while (1); // On bloque le programme ici si erreur
  }
}

void loop() {
  // Variables pour stocker les couleurs
  uint16_t rouge, vert, bleu, clair;

  // Lecture des valeurs
  capteurCouleur.getRawData(&rouge, &vert, &bleu, &clair);

  // Affichage dans le moniteur série
  Serial.print("Rouge : "); Serial.print(rouge);
  Serial.print(" | Vert : "); Serial.print(vert);
  Serial.print(" | Bleu : "); Serial.print(bleu);
  Serial.print(" | Luminosite : "); Serial.println(clair);

  delay(300);
}
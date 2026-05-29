#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_NeoPixel.h>

// --- Configuration ---
#define PIN_LED 6         // Le ruban écoute sur D6
#define NUM_LEDS 30       // Tes 30 LEDs

Adafruit_NeoPixel ruban = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);
Adafruit_TCS34725 capteurCouleur = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() {
  Serial.begin(115200);
  Serial.println("Demarrage du systeme Cameleon Dominant...");

  if (!capteurCouleur.begin()) {
    Serial.println("Erreur : Capteur de couleur introuvable.");
    while (1); 
  }

  ruban.begin();
  ruban.setBrightness(50); // Luminosité globale (laisse à 50 pour protéger l'USB)
  ruban.show();            
}

void loop() {
  float r, g, b;

  // 1. Lecture des valeurs
  capteurCouleur.getRGB(&r, &g, &b);

  Serial.print("Valeurs brutes -> R: "); Serial.print((int)r);
  Serial.print(" | G: "); Serial.print((int)g);
  Serial.print(" | B: "); Serial.println((int)b);

  // 2. L'arbitre : on prépare les couleurs finales (éteint par défaut)
  int finalR = 0;
  int finalG = 0;
  int finalB = 0;

  // Si le Rouge est plus fort que le Vert ET plus fort que le Bleu
  if (r > g && r > b) {
    finalR = 255; // Rouge à fond !
    Serial.println("DOMINANCE : ROUGE");
  } 
  // Sinon, si le Vert est plus fort que le Rouge ET plus fort que le Bleu
  else if (g > r && g > b) {
    finalG = 255; // Vert à fond !
    Serial.println("DOMINANCE : VERT");
  } 
  // Sinon, c'est que le Bleu est le plus fort
  else if (b > r && b > g) {
    finalB = 255; // Bleu à fond !
    Serial.println("DOMINANCE : BLEU");
  }

  // 3. On applique la couleur forte choisie à tout le ruban
  for(int i = 0; i < NUM_LEDS; i++) {
    ruban.setPixelColor(i, ruban.Color(finalR, finalG, finalB)); 
  }
  
  ruban.show(); 
  delay(100); // Un peu plus lent (100ms) pour éviter que ça clignote trop vite si on hésite entre deux couleurs
}
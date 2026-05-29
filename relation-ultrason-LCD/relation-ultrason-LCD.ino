#include <Wire.h>
#include "rgb_lcd.h" // La nouvelle bibliothèque spécifique pour ton écran Grove

// Création de l'objet écran
rgb_lcd lcd;

const int sigPin = 4; // Ton ultrason est toujours sur D4

void setup() {
  Serial.begin(115200);
  
  // Initialisation de l'écran Grove (16 colonnes, 2 lignes)
  lcd.begin(16, 2);
  
  // Bonus Grove : On allume le rétroéclairage en Blanc (Rouge, Vert, Bleu de 0 à 255)
  lcd.setRGB(255, 255, 255); 
  
  lcd.setCursor(0, 0);
  lcd.print("Distance :");
}

void loop() {
  // --- Lecture de l'ultrason ---
  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(sigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(sigPin, LOW);

  pinMode(sigPin, INPUT);
  long duree = pulseIn(sigPin, HIGH, 30000);
  float distance = duree * 0.034 / 2.0;

  // --- Affichage PC ---
  if (duree == 0 || distance > 400) {
    Serial.println("PC -> Hors portee");
  } else {
    Serial.print("PC -> Distance : ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  // --- Affichage LCD Grove ---
  lcd.setCursor(0, 1); 
  
  if (duree == 0 || distance > 400) {
    lcd.print("Hors portee    "); 
    lcd.setRGB(255, 0, 0); // On met l'écran en ROUGE si c'est hors de portée !
  } else {
    lcd.print(distance);
    lcd.print(" cm      "); 
    lcd.setRGB(0, 255, 0); // On met l'écran en VERT quand on a une mesure !
  }

  delay(250); 
}
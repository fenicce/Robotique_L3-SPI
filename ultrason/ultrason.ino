#include <Arduino.h>

// On définit la broche D4, là où est branché ton fil jaune (SIG)
const int sigPin = 4; 

void setup() {
  Serial.begin(115200); 
  Serial.println("Test du capteur ultrason en direct sur D4...");
}

void loop() {
  // 1. On règle la broche en sortie pour envoyer l'impulsion sonore
  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(sigPin, HIGH); // Déclenchement du son
  delayMicroseconds(10);
  digitalWrite(sigPin, LOW);

  // 2. On règle la même broche en entrée pour écouter l'écho
  pinMode(sigPin, INPUT);
  
  // pulseIn compte combien de microsecondes la broche reste à l'état HAUT
  // On met un délai maximum (timeout) de 30000 µs pour ne pas bloquer le code
  long duree = pulseIn(sigPin, HIGH, 30000); 

  // 3. Calcul de la distance
  // La vitesse du son est de 0.034 cm/microseconde.
  // On divise par 2 car le son fait un aller (vers l'objet) et un retour (l'écho).
  float distance = duree * 0.034 / 2.0;

  if (duree == 0) {
    Serial.println("Distance : 400.0 cm (Hors de portee)");
  } else {
    Serial.print("Distance : ");
    Serial.print(distance);
    Serial.println(" cm");
  }
  
  delay(150); // Petit délai pour rendre la lecture fluide sur le moniteur
}
#include <Servo.h>

// Création de l'objet servomoteur
Servo monServo; 

// La broche analogique A0 fonctionne parfaitement ici
const int pinServo = A0; 

void setup() {
  Serial.begin(115200);
  
  // On attache le servo à sa broche A0
  monServo.attach(pinServo);
  
  Serial.println("--- TEST SERVOMOTEUR SUR A0 ---");
}

void loop() {
  Serial.println("Moteur a 0 degre");
  monServo.write(0);   
  delay(2000);         

  Serial.println("Moteur a 90 degres (Milieu)");
  monServo.write(90);  
  delay(2000);

  Serial.println("Moteur a 180 degres (Max)");
  monServo.write(180); 
  delay(2000);
}
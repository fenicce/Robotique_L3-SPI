volatile long compteur = 0;
void compterImpulsion() {
  compteur++;
}
void setup(){
  Serial.begin(9600);
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), compterImpulsion, RISING);
}
void loop(){
  Serial.print("Impulsions: ");
  Serial.println(compteur);
  delay(300);
}
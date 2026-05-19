//bibliothques
#include <Arduino.h>
#include <MeOrion.h>
#include "MeRGBLineFollower.h"
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_NeoPixel.h>
#include "rgb_lcd.h"
#include <Servo.h>

// Création de l'objet écran
rgb_lcd lcd;

// On passe sur le PORT_3 qui gère l'I2C
MeRGBLineFollower LightSensorRGB_1(PORT_3);

//addresses des moteurs de propulsion
#define MOTEUR_GAUCHE 0x66 //moteur de gauche sur la carte I2C
#define MOTEUR_DROIT 0x68 //moteur de droite sur la carte I2C
#define MOTEUR_CATA 0x60 //moteur de catapulte
#define MOTEUR_OR 0x65 //moteur de d'inclinaison de tir

//nombres de dents sur la roue qui sert à determiner l'angle de tir
#define NB_DENTS 60 //a detrminer

//commandes de direction pour les moteurs
#define ARRET 0x00
#define AVANT 0x01
#define ARRIERE 0x02
#define FREIN 0x03

//variables globales
const int sigPin=4;//ultrason en D4
volatile long compteur =0; //pour l'angle

void setup() {
  // put your setup code here, to run once:
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

//fonctions piloter moteur pour la catapulte
void armer()
{
  //remplir
  piloterMoteur(MOTEUR_CATA, ARRIERE, 63);
  delay(6000);//environ 1 tour sans glissement
}


//fonctions comportementales

void SUIVRELIGNE(){
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
}

void LANCEMENT(){
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
}


void loop() {
  // put your main code here, to run repeatedly:
  switch (currentState) {
    case suivreLigne: //suivre la ligne
    //remplir
    SUIVRELIGNE();
    break;//sortir du block

    case tunnel: //partie tunnel
    //remplir
    break;

    case evitemmentGauche: //évitemment par la gauche
    //remplir
    break;

    case evitemmentDroit: //évitemment par la droite
    //remplir
    break;

    case chercherLigne: //recherche d'un ligne
    //remplir
    break;

    case detectionCouleur: //section de la couleur qui a aussi le demi-tour
    //remplir
    break;

    case lancement: //partie catapulte
    //remplir
    LANCEMENT();
    break;

    default://je ne sais pas encore 
    //remplir affichage lcd "a l'aide"
    //surement faire le recherche de ligne ici en fait
    break;
  }
}

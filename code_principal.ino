// ── BIBLIOTHÈQUES ──────────────────────────────────────────────
#include <Arduino.h>
#include <MeOrion.h>
#include "MeRGBLineFollower.h"
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_NeoPixel.h>
#include "rgb_lcd.h"
#include <math.h>

// ── CONFIGURATION MATÉRIELLE ──────────────────────────────────
rgb_lcd lcd;
MeRGBLineFollower LightSensorRGB_1(PORT_3);
Servo servoUltrason;

#define PIN_LED 5
#define NUM_LED 30
#define PIN_SERVO A0
const int sigPin = 4; // Ultrason sur D4

// Note : Si les couleurs s'allument mal (ex: rouge devient vert), 
// change NEO_GRB en NEO_RGB juste en dessous !
Adafruit_NeoPixel ruban = Adafruit_NeoPixel(NUM_LED, PIN_LED, NEO_GRB + NEO_KHZ800);
Adafruit_TCS34725 capteurCouleur = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// Adresses des moteurs I2C (DRV8830)
#define MOTEUR_GAUCHE 0x66 
#define MOTEUR_DROIT  0x68 
#define MOTEUR_CATA   0x60 
#define MOTEUR_OR     0x65 

#define NB_DENTS 36 

// Commandes de direction pour les moteurs
#define ARRET   0x00
#define AVANT   0x01
#define ARRIERE 0x02
#define FREIN   0x03

// ── PARAMÈTRES ET VITESSES DU ROBOT ───────────────────────────
#define VITESSE_SUIVI           40
#define VITESSE_VIRAGE          25
#define VITESSE_VIRAGE_LEGER    13
#define VITESSE_LONGER          50
#define VITESSE_AVANCE          55
#define VITESSE_CHERCHER_LIGNE  37
#define VITESSE_RECUL           25
#define VITESSE_PIVOT_RECUP     55
#define CORRECTION              17

#define RATIO_MOTEUR_GAUCHE   0.92f

// Angles Servo (Ajustement basé sur le milieu physique)
#define ANG_MILIEU 70
#define ANG_GAUCHE 115 
#define ANG_DROITE 25  

// Paramètres PD Récupération de ligne
#define Kp              18
#define Kd              7
#define VITESSE_BASE_PD 17

// Distances de détection
#define DISTANCE_OBSTACLE        15
#define DISTANCE_VIDE           100

// Timings Évitement Obstacles
#define PAUSE_ARRET                       250
#define TEMPS_RECUL                      2000
#define TEMPS_VIRAGE_LONG                2000
#define TEMPS_VIRAGE_COURT                180
#define TEMPS_VIRAGE_REALIGN               50
#define TEMPS_VIRAGE_GAUCHE_1_OBS1       2500
#define TEMPS_AVANCE_DESSUS_OBS1         2200
#define TEMPS_AVANCE_APRES_PAROI          250
#define TEMPS_AVANCE_APRES_PAROI_OBS1_2   250
#define TEMPS_VIRAGE_GAUCHE_2            1700
#define TEMPS_LONGE_OBS1                  550
#define TEMPS_LONGE_OBS2                 1300
#define TEMPS_VIRAGE_LONG_GOBS2          2000
#define TEMPS_VIRAGE_DROITE_REALIGN_OBS2  130
#define TEMPS_VIRAGE_DROITE_1_OBS2       2400
#define TEMPS_AVANCE_DESSUS_OBS2        50000

// Paramètres de l'encodeur filtré
#define tick 5 
const unsigned long delaiFiltrage = 500; 

// ── ÉTATS DE LA MACHINE PRINCIPALE NOUVELLE SÉQUENCE ──────────
enum EtatRobot {
  depart, L1, tunnel, L2, O1, L3, O2, rampe, couleur, L5, lancement
};
EtatRobot currentState = depart; 

// Sous-états pour la routine d'évitement d'obstacle complexe
enum EtatObstacleRoutine {
  ARR_OBS,
  O_RECUL, O_VIR_EXT, O_LONG, O_AV_PAROI1, O_VIR_INT_REALIGN, O_VIR_INT1, O_AV_DESSUS, O_AV_PAROI2, O_VIR_INT2, O_APP_LIGNE, O_RECUP
};
EtatObstacleRoutine subEtatObs = ARR_OBS;

// ── VARIABLES GLOBALES ────────────────────────────────────────
volatile long compteur = 0; 
uint32_t chronoTunnel = 0;
uint32_t tempsEntreeTunnel = 0;
uint32_t tempsEntreeL5 = 0; 
int sousEtapeTunnel = 0;
float distGauche = 100.0f; 
float distDroite = 100.0f; 

int etatFiltre = LOW;          
int dernierEtatBrut = LOW;     
unsigned long dernierTempsChangement = 0;

int stopi = 0;
int dernierEtatCapteurs = 9;
int numeroObstacle = 0;
uint32_t dernierTempsDetection = 0;
uint32_t tempsDebutSubEtat = 0;

bool ligneTrouvee = false;
bool alignementTermine = false;
bool stabilisationFaite = false;
int erreurPrecedente = 0;

// Verrou global pour n'autoriser le tunnel qu'une seule fois
bool tunnelEffectue = false; 

// Variables pour forcer le scan sous le tunnel sans ligne
static bool gaucheScanee = false;
static bool droiteScantee = false;

// ── PROTOTYPES DE TOUTES LES FONCTIONS ─────────────────────────
void piloterMoteur(byte adresse, byte direction, byte vitesse);
void stop();
void avancer();
void reculer();
void droiteLeger();
void droiteFort();
void gaucheLeger();
void gaucheFort();
void avancerAsym(byte vitG, byte vitD);
float Distance(); 
int calculerErreurSuivi(int e);
void allerSubEtatObs(EtatObstacleRoutine nouvelEtat); 
void recupererLigne(int positionLigne, EtatRobot etatSuivant, bool forcePivotDroite);
void compterImpulsion();
void clignoterLED(int r, int g, int b);
void demitour();
void LANCEMENT(); 
void LEDS_DEMITOUR(); 
void SUIVRELIGNE(int position);
void afficherEtatLCD(EtatRobot etat);

// ── SETUP ─────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(100);
  
  LightSensorRGB_1.begin();
  delay(100);
  
  pinMode(2, INPUT_PULLUP); 
  etatFiltre = digitalRead(2);
  dernierEtatBrut = etatFiltre;
  attachInterrupt(digitalPinToInterrupt(2), compterImpulsion, RISING);
  
  lcd.begin(16,2);
  lcd.setRGB(255,255,255);
  lcd.setCursor(0,0);
  lcd.print("Etat: ");
  
  capteurCouleur.begin();
  
  ruban.begin();
  ruban.setBrightness(50); 
  ruban.show(); 

  servoUltrason.attach(PIN_SERVO);
  servoUltrason.write(ANG_MILIEU);
  delay(200);

  stop();
}

void compterImpulsion() {
  compteur++;
}

// ── FONCTIONS MOTEURS ─────────────────────────────────────────
void piloterMoteur(byte adresse, byte direction, byte vitesse) {
  if (vitesse > 63) vitesse = 63;
  byte commande = (vitesse << 2) | direction;
  Wire.beginTransmission(adresse);
  Wire.write(0x00);      
  Wire.write(commande);  
  Wire.endTransmission();
}

void stop() {
  piloterMoteur(MOTEUR_GAUCHE, FREIN, 0); 
  piloterMoteur(MOTEUR_DROIT, FREIN, 0);
}

void avancer() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT, (byte)(VITESSE_SUIVI * RATIO_MOTEUR_GAUCHE)); 
  piloterMoteur(MOTEUR_DROIT, ARRIERE, VITESSE_SUIVI);
}

void reculer() {
  piloterMoteur(MOTEUR_GAUCHE, ARRIERE, VITESSE_RECUL);
  piloterMoteur(MOTEUR_DROIT, AVANT, VITESSE_RECUL);
}

void droiteLeger() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT, VITESSE_SUIVI); 
  piloterMoteur(MOTEUR_DROIT, ARRIERE, VITESSE_SUIVI - CORRECTION);
}

void droiteFort() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT, VITESSE_SUIVI); 
  piloterMoteur(MOTEUR_DROIT, ARRIERE, VITESSE_VIRAGE_LEGER);
}

void gaucheLeger() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT, VITESSE_SUIVI - CORRECTION); 
  piloterMoteur(MOTEUR_DROIT, ARRIERE, VITESSE_SUIVI);
}

void gaucheFort() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT, VITESSE_VIRAGE_LEGER); 
  piloterMoteur(MOTEUR_DROIT, ARRIERE, VITESSE_SUIVI);
}

void avancerAsym(byte vitG, byte vitD) {
  vitG = constrain((byte)(vitG * RATIO_MOTEUR_GAUCHE), 0, 63);
  vitD = constrain(vitD, 0, 63);
  piloterMoteur(MOTEUR_GAUCHE, vitG > 0 ? AVANT : ARRET, vitG);
  piloterMoteur(MOTEUR_DROIT, vitD > 0 ? ARRIERE : ARRET, vitD);
}

void declencher() {
  piloterMoteur(MOTEUR_CATA, ARRIERE, 63);
  delay(3000); 
  piloterMoteur(MOTEUR_CATA, ARRET, 0);
}

// ── COMPORTEMENTS ET ALGORITHMES SUIVI / REPRISE ───────────────
int calculerErreurSuivi(int e) {
  switch (e) {
    case 1:  case 7:   return -6;   
    case 3:  case 11:  return -3;   
    case 2:            return -1;   
    case 6:  case 9:   return  0;   
    case 4:            return  1;   
    case 12: case 13:  return  3;   
    case 8:  case 14:  return  6;   
    default:           return  0;
  }
}

void SUIVRELIGNE(int position) {
  for(int t = 0; t < ruban.numPixels(); t++){
    ruban.setPixelColor(t, ruban.Color(0, 0, 0));
  }
  ruban.show();

  if (position != 0) dernierEtatCapteurs = position;

  switch (position) {
    case 6: case 9:  avancer();     break;
    case 2: case 3:  case 11: gaucheLeger(); break;
    case 1: case 7:  gaucheFort();  break;
    case 4: case 12: case 13: droiteLeger(); break;
    case 8: case 14: droiteFort();  break;
    case 0: 
      if (dernierEtatCapteurs == 1 || dernierEtatCapteurs == 7 || dernierEtatCapteurs == 3) gaucheFort();
      else if (dernierEtatCapteurs == 8 || dernierEtatCapteurs == 14 || dernierEtatCapteurs == 12) droiteFort();
      else avancer();
      break;
    default: break;
  }
}

void recupererLigne(int positionLigne, EtatRobot etatSuivant, bool forcePivotDroite) {
  if (!ligneTrouvee) {
    avancerAsym(VITESSE_CHERCHER_LIGNE, VITESSE_CHERCHER_LIGNE);
    if (positionLigne != 0) {
      ligneTrouvee = true;
      stabilisationFaite = false;
      piloterMoteur(MOTEUR_GAUCHE, FREIN, 63);
      piloterMoteur(MOTEUR_DROIT,  FREIN, 63);
      delay(400);
      stop();
      delay(500);
      erreurPrecedente = calculerErreurSuivi(positionLigne);
    }
    return;
  }
  else if (!stabilisationFaite) {
    if (forcePivotDroite) {
      if (positionLigne == 15) {
        piloterMoteur(MOTEUR_GAUCHE, AVANT, VITESSE_PIVOT_RECUP);
        piloterMoteur(MOTEUR_DROIT,  AVANT, VITESSE_PIVOT_RECUP);
        return;
      }
      if (positionLigne == 1) {
        avancerAsym(VITESSE_CHERCHER_LIGNE, VITESSE_CHERCHER_LIGNE);
        delay(400);
        stop();
      }
      stabilisationFaite = true;
    } else {
      if (positionLigne == 8 || positionLigne == 14 || positionLigne == 4) {
        piloterMoteur(MOTEUR_GAUCHE, ARRIERE, VITESSE_PIVOT_RECUP);
        piloterMoteur(MOTEUR_DROIT,  ARRIERE, VITESSE_PIVOT_RECUP);
        return;
      } else if (positionLigne == 1 || positionLigne == 3 || positionLigne == 7 || positionLigne == 2) {
        piloterMoteur(MOTEUR_GAUCHE, AVANT,    VITESSE_PIVOT_RECUP);
        piloterMoteur(MOTEUR_DROIT,  AVANT,    VITESSE_PIVOT_RECUP);
        return;
      }
      stabilisationFaite = true;
    }
  }
  else if (!alignementTermine) {
    static uint8_t compteurCentre = 0;
    int erreur = calculerErreurSuivi(positionLigne);
    if (positionLigne == 0) {
      erreur = (erreurPrecedente > 0) ? 5 : -5;
    }
    int derivee = erreur - erreurPrecedente;
    if (positionLigne != 0) erreurPrecedente = erreur;

    int correction = (Kp * erreur) + (Kd * derivee);
    int vitG = constrain(VITESSE_BASE_PD + correction, 0, 63);
    int vitD = constrain(VITESSE_BASE_PD - correction, 0, 63);

    if (erreur ==  5) vitG = 0;
    if (erreur == -5) vitD = 0;

    avancerAsym(vitG, vitD);

    if (positionLigne == 9 || positionLigne == 6 || positionLigne == 3 || positionLigne == 12) {
      if (++compteurCentre >= 2) {
        stop();
        alignementTermine = true;
        compteurCentre = 0;
      }
    } else {
      compteurCentre = 0;
    }
  }
  else {
    servoUltrason.write(ANG_MILIEU);
    delay(200);
    stop();
    currentState = etatSuivant;
  }
}

void allerSubEtatObs(EtatObstacleRoutine nouvelEtat) {
  stop();
  subEtatObs = nouvelEtat;
  tempsDebutSubEtat = millis();
  ligneTrouvee = false;
  alignementTermine = false;
  stabilisationFaite = false;
  erreurPrecedente = 0;
}

// ── CAPTEUR DISTANCE ──────────────────────────────────────────
float Distance() {
  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(sigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(sigPin, LOW);

  pinMode(sigPin, INPUT);
  long duree = pulseIn(sigPin, HIGH, 12000); 
  if (duree == 0) return -1.0f; 
  return (duree * 0.034f) / 2.0f;
}

void clignoterLED(int r, int g, int b){
  for(int i = 0; i < 3; i++) {
    for(int t = 0; t < ruban.numPixels(); t++) ruban.setPixelColor(t, ruban.Color(r, g, b));
    ruban.show();
    delay(500);
    for(int t = 0; t < ruban.numPixels(); t++) ruban.setPixelColor(t, ruban.Color(0, 0, 0));
    ruban.show();
    delay(500);
  }
}

// ── DEMI-TOUR (CORRIGÉ POUR PIVOT CONTINU) ───────────────────
void demitour() {
  // 1. Dégagement initial (sortir de la ligne)
  piloterMoteur(MOTEUR_GAUCHE, ARRIERE, 40); 
  piloterMoteur(MOTEUR_DROIT, AVANT, 40);
  delay(800); 

  // 2. Rotation continue jusqu'à voir la ligne
  int position = 15;
  while(position == 15){
    LightSensorRGB_1.updataAllSensorValue();
    position = LightSensorRGB_1.getPositionState();
    delay(10);
  }
  
  // 3. Freinage
  stop();
  delay(200);
}

void LANCEMENT(){
  stop(); // SÉCURITÉ : Forcer l'arrêt
  
  float distance = 0;
  long duree = 0;
  uint32_t chronoSecurite = millis(); 
  
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
      lcd.print("Hors portee    "); 
      lcd.setRGB(255, 0, 0); 
    } else {
      lcd.print(distance);
      lcd.print(" cm      "); 
      lcd.setRGB(0, 255, 0); 
    }
    
    if (millis() - chronoSecurite > 3000) {
      distance = 150.0; // Distance par défaut
      break; 
    }
    delay(100); 
  }

  float angle_voulu = 85.0 - (0.15 * distance);
  if (angle_voulu < 40.0) angle_voulu = 40.0;
  if (angle_voulu > 70.0) angle_voulu = 70.0;

  int ticks_cibles = round((angle_voulu - 40.0) / (float)tick);
  if (ticks_cibles < 0) ticks_cibles = 0;

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
        }
      }
    }
    piloterMoteur(MOTEUR_OR, FREIN, 0); 
  }
  declencher();
}

// ── DÉTECTION COULEUR (CORRIGÉE POUR MIEUX DIFFÉRENCIER) ─────
void LEDS_DEMITOUR() {
  stop(); 
  delay(400); // Laisser le capteur intégrer la couleur
  
  float r, g, b;
  capteurCouleur.getRGB(&r, &g, &b);
  
  // Affichage sur le moniteur série pour t'aider à calibrer
  Serial.print("Couleur lue -> R:"); Serial.print(r);
  Serial.print(" G:"); Serial.print(g);
  Serial.print(" B:"); Serial.println(b);

  int finalR = 0, finalG = 0, finalB = 0;

  // On ajoute une marge (+10) pour être sûr que la couleur domine vraiment
  if (r > (g + 10) && r > (b + 10)) { 
    finalR = 255; 
    Serial.println("-> ROUGE detecte");
  }
  else if (g > (r + 10) && g > (b + 10)) { 
    finalG = 255; 
    Serial.println("-> VERT detecte");
  }
  else if (b > (r + 10) && b > (g + 10)) { 
    finalB = 255; 
    Serial.println("-> BLEU detecte");
  }
  else {
    finalR = 255; finalG = 255; finalB = 255;
    Serial.println("-> Non reconnu (Blanc)");
  }

  clignoterLED(finalR, finalG, finalB);
  demitour();
  
  dernierTempsDetection = millis(); 
  tempsEntreeL5 = millis(); 
  currentState = L5; 
}

void afficherEtatLCD(EtatRobot etat) {
  lcd.setCursor(0, 0);
  switch(etat) {
    case depart:           lcd.print("Etat: depart    "); break;
    case L1:               lcd.print("Etat: L1 suivi1  "); break;
    case tunnel:           lcd.print("Etat: Tunnel    ");  break;
    case L2:               lcd.print("Etat: L2 suivi2  ");  break;
    case O1:               lcd.print("Etat: Obstacle 1");  break;
    case L3:               lcd.print("Etat: L3 suivi3  ");  break;
    case O2:               lcd.print("Etat: Obstacle 2");  break;
    case rampe:            lcd.print("Etat: Rampe     ");  break;
    case couleur:          lcd.print("Etat: Sequence C ");  break;
    case L5:               lcd.print("Etat: L5 Final   ");  break;
    case lancement:        lcd.print("Etat: Lancement ");  break;
    default:               lcd.print("Etat: Inconnu   ");  break;
  }
}

// ── LOOP PRINCIPALE (AIGUILLAGE CHRONOLOGIQUE PARFAIT) ────────
void loop() {
  LightSensorRGB_1.updataAllSensorValue();
  int position = LightSensorRGB_1.getPositionState();
  float dObstacle = Distance(); 
  uint32_t tempsEcouleObs = millis() - tempsDebutSubEtat;

  // ── MACHINE À ÉTATS PHASES DE VIE ────────────────────────────
  switch (currentState) {

    case depart: 
      if (position == 15) {
        delay(1000);
        avancer();
      }
      else if (position != 15 && position != 0) {
        delay(40);
        LightSensorRGB_1.updataAllSensorValue();
        int verifPos = LightSensorRGB_1.getPositionState();
        if (verifPos != 15 && verifPos != 0) {
          currentState = L1;
        }
      }
      else {
        avancer();
      }
      break;
    
    case L1: 
      SUIVRELIGNE(position);
      if (position == 15 && !tunnelEffectue) {
        stop();
        sousEtapeTunnel = 0; 
        distGauche = 0.0f; 
        distDroite = 0.0f;
        gaucheScanee = false;
        droiteScantee = false;
        chronoTunnel = millis();
        tempsEntreeTunnel = millis(); 
        currentState = tunnel; 
      }
      break;

    case tunnel: 
      if (gaucheScanee && droiteScantee && (millis() - tempsEntreeTunnel > 2000)) {
        if (position !=15) {
          stop();
          servoUltrason.write(ANG_MILIEU); 
          delay(200);
          tunnelEffectue = true; 
          
          ligneTrouvee = false;
          stabilisationFaite = false;
          alignementTermine = false;
          
          currentState = L2; 
          break; 
        }
      }

      {
        float seuilDangerG = 23.5f; 
        float seuilDangerD = 23.5f; 

        if (distGauche > 0.1f && distGauche < seuilDangerG) {
          piloterMoteur(MOTEUR_GAUCHE, AVANT, (byte)(VITESSE_SUIVI * 0.45)); 
          piloterMoteur(MOTEUR_DROIT, ARRIERE, VITESSE_SUIVI);
        } 
        else if (distDroite > 0.1f && distDroite < seuilDangerD) {
          piloterMoteur(MOTEUR_GAUCHE, AVANT, VITESSE_SUIVI);
          piloterMoteur(MOTEUR_DROIT, ARRIERE, (byte)(VITESSE_SUIVI * 0.45));
        } 
        else {
          avancer();
        }
      }

      switch (sousEtapeTunnel) {
        case 0: 
          servoUltrason.write(ANG_GAUCHE);
          chronoTunnel = millis();
          sousEtapeTunnel = 1;
          break;
        case 1: 
          if (millis() - chronoTunnel > 280) { 
            float d = Distance(); 
            if (d > 0.0f) { distGauche = d; gaucheScanee = true; } 
            servoUltrason.write(ANG_DROITE); 
            chronoTunnel = millis();
            sousEtapeTunnel = 2;
          }
          break;
        case 2: 
          if (millis() - chronoTunnel > 380) { 
            float d = Distance(); 
            if (d > 0.0f) { distDroite = d; droiteScantee = true; } 
            servoUltrason.write(ANG_GAUCHE); 
            chronoTunnel = millis();
            sousEtapeTunnel = 1; 
          }
          break;
      }
      break;

    case L2: 
      if (!alignementTermine) {
        recupererLigne(position, L2, false); 
      } else {
        SUIVRELIGNE(position);
        if (dObstacle > 5.0f && dObstacle <= DISTANCE_OBSTACLE) {
          allerSubEtatObs(ARR_OBS);
          currentState = O1; 
        }
      }
      break;

    case O1: 
      switch (subEtatObs) {
        case ARR_OBS:
          stop();
          servoUltrason.write(ANG_DROITE); 
          if (tempsEcouleObs >= PAUSE_ARRET) allerSubEtatObs(O_RECUL);
          break;
        case O_RECUL:
          servoUltrason.write(ANG_MILIEU);
          reculer();
          if (tempsEcouleObs >= TEMPS_RECUL) allerSubEtatObs(O_VIR_EXT);
          break;
        case O_VIR_EXT:
          servoUltrason.write(ANG_DROITE);
          avancerAsym(VITESSE_VIRAGE, 0); 
          if (tempsEcouleObs >= TEMPS_VIRAGE_LONG) allerSubEtatObs(O_LONG);
          break;
        case O_LONG:
          avancerAsym(VITESSE_LONGER, 50);
          if (tempsEcouleObs >= TEMPS_LONGE_OBS1 && dObstacle >= DISTANCE_VIDE) allerSubEtatObs(O_AV_PAROI1);
          break;
        case O_AV_PAROI1:
          avancerAsym(VITESSE_LONGER, 50);
          if (tempsEcouleObs >= TEMPS_AVANCE_APRES_PAROI) allerSubEtatObs(O_VIR_INT_REALIGN);
          break;
        case O_VIR_INT_REALIGN:
          avancerAsym(0, VITESSE_VIRAGE_LEGER);
          if (tempsEcouleObs >= TEMPS_VIRAGE_REALIGN) allerSubEtatObs(O_VIR_INT1);
          break;
        case O_VIR_INT1:
          avancerAsym(0, VITESSE_VIRAGE);
          if (tempsEcouleObs >= TEMPS_VIRAGE_GAUCHE_1_OBS1) allerSubEtatObs(O_AV_DESSUS);
          break;
        case O_AV_DESSUS:
          if (dObstacle < DISTANCE_VIDE && dObstacle > 0) avancerAsym(VITESSE_LONGER, 50);
          else avancer();
          if (tempsEcouleObs >= TEMPS_AVANCE_DESSUS_OBS1 && dObstacle >= DISTANCE_VIDE) allerSubEtatObs(O_AV_PAROI2);
          break;
        case O_AV_PAROI2:
          avancerAsym(VITESSE_LONGER, 50);
          if (tempsEcouleObs >= TEMPS_AVANCE_APRES_PAROI_OBS1_2) allerSubEtatObs(O_VIR_INT2);
          break;
        case O_VIR_INT2:
          avancerAsym(0, VITESSE_VIRAGE);
          if (tempsEcouleObs >= TEMPS_VIRAGE_GAUCHE_2) allerSubEtatObs(O_APP_LIGNE);
          break;
        case O_APP_LIGNE:
          servoUltrason.write(ANG_MILIEU);
          avancerAsym(VITESSE_CHERCHER_LIGNE, VITESSE_CHERCHER_LIGNE);
          if (position != 0) allerSubEtatObs(O_RECUP);
          break;
        case O_RECUP:
          if (tempsEcouleObs < 500) { stop(); break; }
          if (dObstacle > 0.0f && dObstacle <= 3.5f) { stop(); currentState = couleur; break; }
          
          recupererLigne(position, L3, true); 
          break;
      }
      break;

    case L3: 
      if (!alignementTermine) {
        recupererLigne(position, L3, true);
      } else {
        SUIVRELIGNE(position);
        if (dObstacle > 3.5f && dObstacle <= DISTANCE_OBSTACLE) {
          allerSubEtatObs(ARR_OBS);
          currentState = O2; 
        }
      }
      break;

    case O2: 
      switch (subEtatObs) {
        case ARR_OBS:
          stop();
          servoUltrason.write(ANG_GAUCHE);
          if (tempsEcouleObs >= PAUSE_ARRET) allerSubEtatObs(O_RECUL);
          break;
        case O_RECUL:
          servoUltrason.write(ANG_MILIEU);
          reculer();
          if (tempsEcouleObs >= TEMPS_RECUL) allerSubEtatObs(O_VIR_EXT);
          break;
        case O_VIR_EXT:
          servoUltrason.write(ANG_GAUCHE);
          avancerAsym(0, VITESSE_VIRAGE);
          if (tempsEcouleObs >= TEMPS_VIRAGE_LONG_GOBS2) allerSubEtatObs(O_LONG);
          break;
        case O_LONG:
          avancerAsym(VITESSE_LONGER, 50);
          if (tempsEcouleObs >= TEMPS_LONGE_OBS2 && dObstacle >= DISTANCE_VIDE) allerSubEtatObs(O_AV_PAROI1);
          break;
        case O_AV_PAROI1:
          avancerAsym(VITESSE_LONGER, 50);
          if (tempsEcouleObs >= TEMPS_AVANCE_APRES_PAROI) allerSubEtatObs(O_VIR_INT_REALIGN);
          break;
        case O_VIR_INT_REALIGN:
          avancerAsym(VITESSE_VIRAGE_LEGER, 0);
          if (tempsEcouleObs >= TEMPS_VIRAGE_DROITE_REALIGN_OBS2) allerSubEtatObs(O_VIR_INT1);
          break;
        case O_VIR_INT1:
          avancerAsym(VITESSE_VIRAGE, 0);
          if (tempsEcouleObs >= TEMPS_VIRAGE_DROITE_1_OBS2 + 100) allerSubEtatObs(O_AV_DESSUS);
          break;
        case O_AV_DESSUS:
          avancerAsym(VITESSE_AVANCE, 50);
          if (tempsEcouleObs >= TEMPS_AVANCE_DESSUS_OBS2 || position != 0) allerSubEtatObs(O_RECUP);
          break;
        case O_RECUP:
          servoUltrason.write(ANG_MILIEU);
          if (tempsEcouleObs < 500) { stop(); break; }
          
          ligneTrouvee = false;
          stabilisationFaite = false;
          alignementTermine = false;
          currentState = rampe; 
          break;
      }
      break;

    case rampe: 
      if (!alignementTermine) {
        recupererLigne(position, rampe, false);
      } else {
        SUIVRELIGNE(position);
        if (dObstacle > 0.5f && dObstacle <= 3.5f) {
          stop();
          currentState = couleur; 
        }
      }
      break;

    case couleur: 
      LEDS_DEMITOUR(); 
      break;

    case L5: 
      if (position == 0 && (millis() - tempsEntreeL5 > 3000)) {
        stop();
        delay(50); 
        currentState = lancement; 
      } else {
        SUIVRELIGNE(position);
      }
      break;

    case lancement: 
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
      declencher();
      break;
  }

  afficherEtatLCD(currentState);
  delay(10); 
}

#include <Wire.h>
#include <Servo.h>
#include <MeOrion.h>
#include "MeRGBLineFollower.h"

// ── BROCHES & ADRESSES ────────────────────────────────────────
#define MOTEUR_GAUCHE  0x66
#define MOTEUR_DROIT   0x68
#define ARRET          0x00
#define AVANT          0x01
#define ARRIERE        0x02
#define FREIN          0x03
#define PIN_SERVO      A0
#define PIN_ULTRASON   4

// ── ANGLES SERVO ──────────────────────────────────────────────
#define SERVO_CENTRE   70
#define SERVO_GAUCHE  180
#define SERVO_DROITE    0

// ── VITESSES ──────────────────────────────────────────────────
#define VITESSE_SUIVI            52
#define VITESSE_VIRAGE           25
#define VITESSE_VIRAGE_LEGER     13
#define VITESSE_LONGER           50
#define VITESSE_AVANCE           55
#define VITESSE_RECUPERATION     30
#define VITESSE_CHERCHER_LIGNE   30
#define VITESSE_RECUL            25
#define VITESSE_PIVOT_RECUP      55
#define CORRECTION               17

// ── PD RÉCUPÉRATION ───────────────────────────────────────────
#define Kp              15
#define Kd              10
#define VITESSE_BASE_PD 20
#define Kp_SUIVI        9.0f
#define Kd_SUIVI        2.0f
#define VITESSE_BASE_SUIVI 42

// ── DISTANCES ─────────────────────────────────────────────────
#define DISTANCE_OBSTACLE        18
#define DISTANCE_VIDE           100

// ── CORRECTION DÉSÉQUILIBRE MOTEURS ───────────────────────────
#define RATIO_MOTEUR_GAUCHE   0.92f

// ── TIMING ────────────────────────────────────────────────────
#define PAUSE_ARRET                      250
#define TEMPS_RECUL                     2000
#define TEMPS_VIRAGE_LONG               1250
#define TEMPS_VIRAGE_COURT               180
#define TEMPS_VIRAGE_REALIGN              50
#define TEMPS_VIRAGE_DROITE_REALIGN_OBS2 130
#define TEMPS_VIRAGE_DROITE_1_OBS2      2500
#define TEMPS_VIRAGE_GAUCHE_1_OBS1      2000
#define TEMPS_AVANCE_DESSUS_OBS1        2200
#define TEMPS_AVANCE_DESSUS_OBS2       50000
#define TEMPS_AVANCE_APRES_PAROI         800
#define TEMPS_AVANCE_APRES_PAROI_OBS1_2  500
#define TEMPS_VIRAGE_OBS2_RETOUR        1800
#define TEMPS_VIRAGE_GAUCHE_2           2500
#define TIMEOUT_CHERCHER_LIGNE         20000
#define TEMPS_LONGE_OBS1                 550
#define TEMPS_LONGE_OBS2                1300
#define TEMPS_VIRAGE_LONG_GOBS2        2000

// ── ÉTATS ─────────────────────────────────────────────────────
enum EtatRobot {
  SUIVI,
  ARRET_OBSTACLE,

  OBS1_RECUL,
  OBS1_VIRAGE_DROITE_1,
  OBS1_LONGE_OBSTACLE,
  OBS1_AVANCE_APRES_PAROI,
  OBS1_VIRAGE_GAUCHE_REALIGN,
  OBS1_VIRAGE_GAUCHE_1,
  OBS1_AVANCE_DESSUS,
  OBS1_AVANCE_APRES_PAROI_2,
  OBS1_VIRAGE_GAUCHE_2,
  OBS1_APPROCHE_LIGNE,
  OBS1_RECUPERATION_LIGNE,

  OBS2_RECUL,
  OBS2_VIRAGE_GAUCHE_1,
  OBS2_LONGE_OBSTACLE,
  OBS2_AVANCE_APRES_PAROI,
  OBS2_VIRAGE_DROITE_REALIGN,
  OBS2_VIRAGE_DROITE_1,
  OBS2_AVANCE_DESSUS,
  // OBS2_VIRAGE_GAUCHE_2,
  //OBS2_LONGE_RETOUR,
  //OBS2_VIRAGE_DROITE_2,
  OBS2_RECUPERATION_LIGNE
};

// ── VARIABLES GLOBALES ────────────────────────────────────────
EtatRobot etat              = SUIVI;
uint32_t  tempsDebut        = 0;
int       dernierEtatCapteurs   = 9;
int       numeroObstacle        = 0;
uint32_t  dernierTempsDetection = 0;

bool     ligneTrouvee       = false;
bool     alignementTermine  = false;
bool     stabilisationFaite = false;
uint32_t tempsStabilisation  = 0;
uint32_t tempsRechercheDebut = 0;
int erreurPrecedente   = 0;
int erreurPrecSuivi    = 0;

Servo monServo;
MeRGBLineFollower LightSensorRGB_1(PORT_3);

// ── PROTOTYPES ────────────────────────────────────────────────
void  piloterMoteur(byte adresse, byte direction, byte vitesse);
void  avancer();
void  stop();
void  reculer();
void  droiteLeger();
void  droiteFort();
void  gaucheLeger();
void  gaucheFort();
void  avancerAsym(byte vitG, byte vitD);
void  piloterSelonEtat(int e);
void  suivreLigne(int e);
void  recupererLigne(int positionLigne, EtatRobot etatSuivant, bool forcePivotDroite);
void  allerEtat(EtatRobot nouvelEtat);
int   lireCapteurs();
int   calculerErreurSuivi(int e);
float lireUltrason();

// ── LECTURE CAPTEURS RGB ──────────────────────────────────────
int lireCapteurs() {
  LightSensorRGB_1.updataAllSensorValue();
  return LightSensorRGB_1.getPositionState();
}

// ── ERREUR PD ─────────────────────────────────────────────────
int calculerErreurSuivi(int e) {
  switch (e) {
    case 1:  case 7:   return -6;   // extrême gauche
    case 3:  case 11:  return -3;   // gauche
    case 2:            return -1;   // légèrement gauche
    case 6:  case 9:   return  0;   // centré
    case 4:            return  1;   // légèrement droite
    case 12: case 13:  return  3;   // droite
    case 8:  case 14:  return  6;   // extrême droite
    default:           return  0;
  }
}

// ── FONCTIONS MOTEURS ─────────────────────────────────────────
void piloterMoteur(byte adresse, byte direction, byte vitesse) {
  if (vitesse > 63) vitesse = 63;
  Wire.beginTransmission(adresse);
  Wire.write(0x00);
  Wire.write((vitesse << 2) | direction);
  Wire.endTransmission();
}

void avancer() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT,   (byte)(VITESSE_SUIVI * RATIO_MOTEUR_GAUCHE));
  piloterMoteur(MOTEUR_DROIT,  ARRIERE, VITESSE_SUIVI);
}

void stop() {
  piloterMoteur(MOTEUR_GAUCHE, FREIN, 0);
  piloterMoteur(MOTEUR_DROIT,  FREIN, 0);
}

void reculer() {
  piloterMoteur(MOTEUR_GAUCHE, ARRIERE, VITESSE_RECUL);
  piloterMoteur(MOTEUR_DROIT,  AVANT,   VITESSE_RECUL);
}

void droiteLeger() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT,   VITESSE_SUIVI);
  piloterMoteur(MOTEUR_DROIT,  ARRIERE, VITESSE_SUIVI - CORRECTION);
}

void droiteFort() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT,   VITESSE_SUIVI);
  piloterMoteur(MOTEUR_DROIT,  ARRIERE, VITESSE_VIRAGE_LEGER);
}

void gaucheLeger() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT,   VITESSE_SUIVI - CORRECTION);
  piloterMoteur(MOTEUR_DROIT,  ARRIERE, VITESSE_SUIVI);
}

void gaucheFort() {
  piloterMoteur(MOTEUR_GAUCHE, AVANT,   VITESSE_VIRAGE_LEGER);
  piloterMoteur(MOTEUR_DROIT,  ARRIERE, VITESSE_SUIVI);
}

void avancerAsym(byte vitG, byte vitD) {
  vitG = constrain((byte)(vitG * RATIO_MOTEUR_GAUCHE), 0, 63);
  vitD = constrain(vitD, 0, 63);
  piloterMoteur(MOTEUR_GAUCHE, vitG > 0 ? AVANT   : ARRET, vitG);
  piloterMoteur(MOTEUR_DROIT,  vitD > 0 ? ARRIERE : ARRET, vitD);
}

// ── ULTRASON ──────────────────────────────────────────────────
float lireUltrason() {
  const uint8_t NB_MESURES = 3;
  float somme = 0;
  uint8_t valides = 0;

  for (uint8_t i = 0; i < NB_MESURES; i++) {
    pinMode(PIN_ULTRASON, OUTPUT);
    digitalWrite(PIN_ULTRASON, LOW);  delayMicroseconds(2);
    digitalWrite(PIN_ULTRASON, HIGH); delayMicroseconds(10);
    digitalWrite(PIN_ULTRASON, LOW);
    pinMode(PIN_ULTRASON, INPUT);
    long duree = pulseIn(PIN_ULTRASON, HIGH, 20000UL);
    if (duree == 0) { delay(5); continue; }
    float d = duree * 0.017f;
    if (d < 2.5f || d > 400.0f) { delay(5); continue; }
    somme += d;
    valides++;
    delay(5);
  }

  if (valides == 0) return 400.0f;
  return somme / valides;
}

// ── PILOTAGE SELON ÉTAT CAPTEURS ──────────────────────────────
void piloterSelonEtat(int e) {
  switch (e) {
    case 6:  case 9:           avancer();     break;
    case 2:  case 3:  case 11: gaucheLeger(); break;
    case 1:  case 7:           gaucheFort();  break;
    case 4:  case 12: case 13: droiteLeger(); break;
    case 8:  case 14:          droiteFort();  break;
    case 0:  piloterSelonEtat(dernierEtatCapteurs); break;
    default: avancerAsym(VITESSE_CHERCHER_LIGNE, VITESSE_CHERCHER_LIGNE); break;
  }
}

// ── SUIVI DE LIGNE ────────────────────────────────────────────
void suivreLigne(int e) {
  monServo.write(SERVO_CENTRE);
  if (e != 0) dernierEtatCapteurs = e;
  piloterSelonEtat(e);
  delay(10);
}

// ── RÉCUPÉRATION DE LIGNE AVEC PD ─────────────────────────────
// ── RÉCUPÉRATION DE LIGNE AVEC PD ─────────────────────────────
void recupererLigne(int positionLigne, EtatRobot etatSuivant, bool forcePivotDroite) {

  if (!ligneTrouvee) {
    avancerAsym(VITESSE_CHERCHER_LIGNE, VITESSE_CHERCHER_LIGNE);
    if (positionLigne != 0) {
      ligneTrouvee       = true;
      stabilisationFaite = false;
      piloterMoteur(MOTEUR_GAUCHE, FREIN, 63);
      piloterMoteur(MOTEUR_DROIT,  FREIN, 63);
      delay(400);
      stop();
      delay(500);
      tempsStabilisation = millis();
      erreurPrecedente   = calculerErreurSuivi(positionLigne);
    }
    return;
  }

  else if (!stabilisationFaite) {
    if (positionLigne == 15 || positionLigne == 0) {
      stop();
      return;
    }
    if (forcePivotDroite) {
      if (positionLigne == 8 || positionLigne == 14 || positionLigne == 4) {
        piloterMoteur(MOTEUR_GAUCHE, AVANT, VITESSE_PIVOT_RECUP);
        piloterMoteur(MOTEUR_DROIT,  AVANT, VITESSE_PIVOT_RECUP);
        return;
      }
    } else {
      if (positionLigne == 8 || positionLigne == 14 || positionLigne == 4) {
        piloterMoteur(MOTEUR_GAUCHE, ARRIERE, VITESSE_PIVOT_RECUP);
        piloterMoteur(MOTEUR_DROIT,  ARRIERE, VITESSE_PIVOT_RECUP);
        return;
      } else if (positionLigne == 1 || positionLigne == 3 ||
                 positionLigne == 7 || positionLigne == 2) {
        piloterMoteur(MOTEUR_GAUCHE, AVANT, VITESSE_PIVOT_RECUP);
        piloterMoteur(MOTEUR_DROIT,  AVANT, VITESSE_PIVOT_RECUP);
        return;
      }
    }
    stabilisationFaite = true;
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

    if (positionLigne == 9  || positionLigne == 6 ||
        positionLigne == 3  || positionLigne == 12) {
      if (++compteurCentre >= 2) {
        stop();
        alignementTermine = true;
        compteurCentre    = 0;
      }
    } else {
      compteurCentre = 0;
    }
  }

  else {
    monServo.write(SERVO_CENTRE);
    delay(200);
    allerEtat(etatSuivant);
  }
}

// ── CHANGEMENT D'ÉTAT ─────────────────────────────────────────
void allerEtat(EtatRobot nouvelEtat) {
  stop();
  etat               = nouvelEtat;
  tempsDebut         = millis();
  tempsRechercheDebut = millis();
  ligneTrouvee       = false;
  alignementTermine  = false;
  stabilisationFaite = false;
  erreurPrecedente   = 0;
}

// ── SETUP ─────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(100);
  LightSensorRGB_1.begin();
  delay(100);
  monServo.attach(PIN_SERVO);
  monServo.write(SERVO_CENTRE);
  delay(200);
  stop();
  delay(500);
  avancer();
  Serial.println(F("=== Robot pret ==="));
}

// ── LOOP PRINCIPALE ───────────────────────────────────────────
void loop() {
  int   positionLigne    = lireCapteurs();
  float distanceObstacle = lireUltrason();
  uint32_t tempsEcoule   = millis() - tempsDebut;

  Serial.print(F("Pos=")); Serial.print(positionLigne, BIN);
  Serial.print(F(" Dist=")); Serial.print(distanceObstacle);
  Serial.print(F(" Etat=")); Serial.println(etat);

  // ── DÉTECTION OBSTACLE ──────────────────────────────────────
  if (etat == SUIVI && positionLigne != 0
      && distanceObstacle > 2.5f && distanceObstacle < DISTANCE_OBSTACLE
      && (millis() - dernierTempsDetection) > 2000) {
    numeroObstacle++;
    dernierTempsDetection = millis();
    Serial.print(F("Obstacle : ")); Serial.println(numeroObstacle);
    allerEtat(ARRET_OBSTACLE);
    return;
  }

  switch (etat) {

    case SUIVI:
      suivreLigne(positionLigne);
      break;

    case ARRET_OBSTACLE:
      stop();
      if (numeroObstacle == 1) {
        monServo.write(SERVO_DROITE);
        if (tempsEcoule >= PAUSE_ARRET) allerEtat(OBS1_RECUL);
      } else {
        monServo.write(SERVO_GAUCHE);
        if (tempsEcoule >= PAUSE_ARRET) allerEtat(OBS2_RECUL);
      }
      break;

    // ════════════════════════════════════════════════════════
    //  OBS1 — contournement par la droite
    // ════════════════════════════════════════════════════════

    case OBS1_RECUL:
      monServo.write(SERVO_CENTRE);
      reculer();
      Serial.println(F("OBS1_RECUL"));
      if (tempsEcoule >= TEMPS_RECUL) allerEtat(OBS1_VIRAGE_DROITE_1);
      break;

    case OBS1_VIRAGE_DROITE_1:
      monServo.write(SERVO_DROITE);
      avancerAsym(VITESSE_VIRAGE, 0);
      Serial.println(F("OBS1_VIRAGE_DROITE_1"));
      if (tempsEcoule >= TEMPS_VIRAGE_LONG) allerEtat(OBS1_LONGE_OBSTACLE);
      break;

    case OBS1_LONGE_OBSTACLE:
      monServo.write(SERVO_DROITE);
      avancerAsym(VITESSE_LONGER, 50);
      Serial.println(F("OBS1_LONGE_OBSTACLE"));
      if (tempsEcoule >= TEMPS_LONGE_OBS1
          && distanceObstacle >= DISTANCE_VIDE)
        allerEtat(OBS1_AVANCE_APRES_PAROI);
      break;

    case OBS1_AVANCE_APRES_PAROI:
      monServo.write(SERVO_DROITE);
      avancerAsym(VITESSE_LONGER, 50);
      Serial.println(F("OBS1_AVANCE_APRES_PAROI"));
      if (tempsEcoule >= TEMPS_AVANCE_APRES_PAROI)
        allerEtat(OBS1_VIRAGE_GAUCHE_REALIGN);
      break;

    case OBS1_VIRAGE_GAUCHE_REALIGN:
      monServo.write(SERVO_DROITE);
      avancerAsym(0, VITESSE_VIRAGE_LEGER);
      Serial.println(F("OBS1_VIRAGE_GAUCHE_REALIGN"));
      if (tempsEcoule >= TEMPS_VIRAGE_REALIGN)
        allerEtat(OBS1_VIRAGE_GAUCHE_1);
      break;

    case OBS1_VIRAGE_GAUCHE_1:
      monServo.write(SERVO_DROITE);
      avancerAsym(0, VITESSE_VIRAGE);
      Serial.println(F("OBS1_VIRAGE_GAUCHE_1"));
      if (tempsEcoule >= TEMPS_VIRAGE_GAUCHE_1_OBS1)
        allerEtat(OBS1_AVANCE_DESSUS);
      break;

    case OBS1_AVANCE_DESSUS:
      monServo.write(SERVO_DROITE);
      if (distanceObstacle < DISTANCE_VIDE) {
        avancerAsym(VITESSE_LONGER, 50);
      } else {
        avancer();
      }
      Serial.println(F("OBS1_AVANCE_DESSUS"));
      if (tempsEcoule >= TEMPS_AVANCE_DESSUS_OBS1
          && distanceObstacle >= DISTANCE_VIDE)
        allerEtat(OBS1_AVANCE_APRES_PAROI_2);
      break;

    case OBS1_AVANCE_APRES_PAROI_2:
      monServo.write(SERVO_DROITE);
      avancerAsym(VITESSE_LONGER, 50);
      Serial.println(F("OBS1_AVANCE_APRES_PAROI_2"));
      if (tempsEcoule >= TEMPS_AVANCE_APRES_PAROI_OBS1_2)
        allerEtat(OBS1_VIRAGE_GAUCHE_2);
      break;

    case OBS1_VIRAGE_GAUCHE_2:
      monServo.write(SERVO_DROITE);
      avancerAsym(0, VITESSE_VIRAGE);
      Serial.println(F("OBS1_VIRAGE_GAUCHE_2"));
      if (tempsEcoule >= TEMPS_VIRAGE_GAUCHE_2)
        allerEtat(OBS1_APPROCHE_LIGNE);
      break;

      case OBS1_APPROCHE_LIGNE:
      monServo.write(SERVO_CENTRE);
      // Étape d'approche lente demandée : avance doucement en longeant
      avancerAsym(VITESSE_CHERCHER_LIGNE, VITESSE_CHERCHER_LIGNE);
      
      // Enclenche la récupération UNIQUEMENT s'il voit la ligne
      if (positionLigne != 0) {
        allerEtat(OBS1_RECUPERATION_LIGNE);
      }
      break;

    case OBS1_RECUPERATION_LIGNE:
      monServo.write(SERVO_CENTRE);
      if (tempsEcoule < 500) { stop(); break; }
      if (!ligneTrouvee) erreurPrecedente = 0;
      recupererLigne(positionLigne, SUIVI, true);
      break;

    // ════════════════════════════════════════════════════════
    //  OBS2 — contournement par la gauche
    // ════════════════════════════════════════════════════════

    case OBS2_RECUL:
      monServo.write(SERVO_CENTRE);
      reculer();
      Serial.println(F("OBS2_RECUL"));
      if (tempsEcoule >= TEMPS_RECUL) allerEtat(OBS2_VIRAGE_GAUCHE_1);
      break;

    case OBS2_VIRAGE_GAUCHE_1:
      monServo.write(SERVO_GAUCHE);
      avancerAsym(0, VITESSE_VIRAGE);
      if (tempsEcoule >= TEMPS_VIRAGE_LONG_GOBS2) allerEtat(OBS2_LONGE_OBSTACLE);
      break;

    case OBS2_LONGE_OBSTACLE:
      monServo.write(SERVO_GAUCHE);
      avancerAsym(VITESSE_LONGER, 50);
      if (tempsEcoule >= TEMPS_LONGE_OBS2
          && distanceObstacle >= DISTANCE_VIDE)
        allerEtat(OBS2_AVANCE_APRES_PAROI);
      break;

    case OBS2_AVANCE_APRES_PAROI:
      monServo.write(SERVO_GAUCHE);
      avancerAsym(VITESSE_LONGER, 50);
      if (tempsEcoule >= TEMPS_AVANCE_APRES_PAROI)
        allerEtat(OBS2_VIRAGE_DROITE_REALIGN);
      break;

    case OBS2_VIRAGE_DROITE_REALIGN:
      monServo.write(SERVO_GAUCHE);
      avancerAsym(VITESSE_VIRAGE_LEGER, 0);
      if (tempsEcoule >= TEMPS_VIRAGE_DROITE_REALIGN_OBS2)
        allerEtat(OBS2_VIRAGE_DROITE_1);
      break;

    case OBS2_VIRAGE_DROITE_1:
      monServo.write(SERVO_GAUCHE);
      avancerAsym(VITESSE_VIRAGE, 0);
      if (tempsEcoule >= TEMPS_VIRAGE_DROITE_1_OBS2 + 100)
        allerEtat(OBS2_AVANCE_DESSUS);
      break;

   // APRÈS
// APRÈS
case OBS2_AVANCE_DESSUS:
  monServo.write(SERVO_GAUCHE);
  avancerAsym(VITESSE_AVANCE, 50);
  if (positionLigne != 0 && tempsEcoule >= 500) {
    allerEtat(OBS2_RECUPERATION_LIGNE);
    break;
  }
  // Fallback chrono (50000ms = sécurité ultime)
  if (tempsEcoule >= TEMPS_AVANCE_DESSUS_OBS2)
    allerEtat(OBS2_RECUPERATION_LIGNE);
  break;

//    case OBS2_VIRAGE_DROITE_2:
      monServo.write(SERVO_DROITE);
      avancerAsym(VITESSE_VIRAGE, 0);
      // Si voit la ligne pendant le virage → suivi direct
      if (positionLigne != 0) {
        allerEtat(SUIVI);
        break;
      }
      if (tempsEcoule >= TEMPS_VIRAGE_OBS2_RETOUR)
        //allerEtat(OBS2_LONGE_RETOUR);
      break;

  //  case OBS2_LONGE_RETOUR:
      monServo.write(SERVO_GAUCHE);
      avancerAsym(VITESSE_LONGER, VITESSE_LONGER);
      // Si voit la ligne pendant le longe → récupération
      if (positionLigne != 0) {
        allerEtat(OBS2_RECUPERATION_LIGNE);
        break;
      }
      if (tempsEcoule >= 600) allerEtat(OBS2_RECUPERATION_LIGNE);
      break;

    case OBS2_RECUPERATION_LIGNE:
      monServo.write(SERVO_CENTRE);
      if (tempsEcoule < 500) { stop(); break; }
      if (!ligneTrouvee) {
        // Avance tant qu'il ne trouve pas la ligne
        avancerAsym(VITESSE_CHERCHER_LIGNE, VITESSE_CHERCHER_LIGNE);
      }
      // Enclenche le calcul de récupération dès qu'il la voit
      if (positionLigne != 0 || ligneTrouvee) {
        if (!ligneTrouvee) erreurPrecedente = 3;
        recupererLigne(positionLigne, SUIVI, false);
      }
      break;
  }

  delay(10);
}
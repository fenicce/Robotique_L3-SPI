# Circuit de chronométrage

## Description
Ce dossier contient les fichiers liés à la conception et à la réalisation du circuit de chronométrage du robot.
Le circuit permet de mesurer le temps de passage du robot dans la section 8 du parcours et d’afficher les secondes sur deux afficheurs 7 segments.
## Composants utilisés
- NE555
- CD4013
- CD4518
- CD4511
- Connecteur DB25
- Résistances et condensateurs

## Fonctionnement

Le circuit de chronométrage permet de mesurer le temps de passage du robot dans la section 8 du parcours.
Le NE555 est configuré en mode astable afin de générer une horloge de 1 Hz correspondant à une impulsion par seconde.
Lorsque le robot coupe le premier faisceau infrarouge IR1, la bascule CD4013 active le signal ENABLE du compteur CD4518 et le comptage démarre.
Le CD4518 compte alors les impulsions d’horloge générées par le NE555 et fournit un code BCD sur ses sorties.
Les décodeurs CD4511 convertissent ce code BCD en signaux permettant de piloter les deux afficheurs 7 segments.
Lorsque le robot coupe le second faisceau infrarouge IR2, le signal RESET du CD4013 est activé, ce qui désactive le comptage et fige l’affichage du temps mesuré.

## Réalisation
- Schéma sous KiCad
- Routage PCB
- Typon
- Gravure et perçage
- Soudage des composants
- Tests du circuit

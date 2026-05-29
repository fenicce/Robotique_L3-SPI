# Cahier des charges — Passerelle treillis

## Contexte
Concevoir et fabriquer une passerelle en bâtonnets de bois capable de franchir une distance donnée tout en supportant une charge centrée. Le projet est évalué sur la rigidité, la résistance et la masse.

## Contraintes géométriques

| Paramètre | Valeur |
|---|---|
| Longueur totale | **900 mm** (imposée) |
| Portée libre entre appuis | **780 mm** (60 + 780 + 60) |
| Bande de roulage | PS choc, **300 × 1,5 mm** |

## Matériau

| Élément | Caractéristique |
|---|---|
| Type | Bâtonnets de bois |
| Dimensions unitaires | **114 × 10 × 2 mm** |
| Masse unitaire | **1,3 g** |
| Limite de bâtonnets | **150 maximum** |

## Conditions d'essai

### Test de rigidité
- Charge centrée appliquée : **15 N**
- Grandeur mesurée : flèche d (mm)

### Test de résistance
- Charge progressive : **15 N → 150 N**
- Grandeur mesurée : charge à la rupture F (N)

### Dispositif de chargement
- Type **pince** : deux plaques serrant la passerelle au centre
- Tige filetée traversante
- Masse calibrée suspendue à 162 mm sous la passerelle

## Indices de performance

### Indice de rigidité k
```
k = m² / (d × M²)
```
- m = masse d'un bâtonnet = 1,3 g
- d = flèche mesurée à 15 N (mm)
- M = masse totale de la passerelle (g)

### Indice de résistance r
```
r = F × m² / M²
```
- F = charge à la rupture (N)
- m, M : idem ci-dessus

**Objectif** : maximiser k et r tout en minimisant M.

## Critères d'évaluation

- Respect du cahier des charges (longueur, nombre de bâtonnets)
- Indices k et r calculés à partir des tests physiques
- Qualité de la conception (simulation, justification des choix)
- Qualité de la fabrication (rendu, alignement, joints)
- Rapport et soutenance

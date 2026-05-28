# 🏎️ Robot Morgan Gouro — Groupe 6

> Robot autonome suiveur de ligne avec catapulte intégrée, inspiré de la **Morgan 3 Wheeler** et habillé d'un **masque Gouro**.

**Maximin Ballot** — Licence Sciences pour l'Ingénieur, option Génie Mécanique  
Université d'Évry Paris-Saclay

---

## 📖 Concept

Ce robot fusionne deux univers : la **Morgan 3 Wheeler** (voiture britannique vintage des années 1930 avec son long capot et son moteur V-Twin exposé) et l'art africain à travers un **masque Gouro** (Côte d'Ivoire) qui remplace le bloc moteur traditionnel.

Le résultat : une carrosserie organique, un visage expressif à l'avant, et un personnage à part entière plutôt qu'une simple machine.

![Robot final assemblé](./photos/robot_final.jpg)

---

## 🎯 Mission

Le robot doit accomplir un parcours autonome avec plusieurs épreuves :

- **Suivi de ligne noire** au sol (capteur infrarouge sous le nez)
- **Passage de tunnel** : 500 × 500 mm de section
- **Franchissement d'une pente** à 8°
- **Traversée d'un pont** étroit
- **Lancement d'un projectile** via la catapulte intégrée sur cible

---

## 🛠️ Architecture matérielle

### Châssis
- **Base MakeBlock** avec 2 roues motrices et 1 roue caster
- Voie : 300 mm
- Hauteur totale : ≤ 500 mm (contrainte tunnel)

### Carrosserie (coque)
- **Thermoformage** sur moule en bois (cf. photos)
- Matériau : PS choc, épaisseur 2 mm
- Forme : silhouette Morgan avec capot bas et cockpit reculé
- Réalisée en 2 demi-coques collées

### Composants décoratifs
- **Masque Gouro** : peint à la main (yeux jaunes cartoon, nez vert, cornes rouges en boudin, "oreilles" en carton dentelé), collé à l'avant
- **Pots d'échappement** : impression 3D PLA avec coton blanc à l'extrémité (effet fumée), fixés latéralement
- **Touffes de coton** : effet ouate décoratif autour des roues

### Électronique embarquée
| Composant | Rôle |
|---|---|
| Carte Arduino + shield | Cerveau principal |
| Capteur suiveur de ligne (IR) | Détection du tracé au sol |
| Capteur ultrason HC-SR04 | Détection d'obstacles avant |
| Capteur couleur TCS3200 | Identification de cibles |
| Servomoteur | Mécanisme catapulte |
| Bandeau LED RGB | Effets lumineux |
| Écran LCD | Retour d'information |
| Pack batterie | Alimentation |

### Mécanisme catapulte
- Bras pivotant monté sur tour MakeBlock verticale
- Ressort de rappel pour stocker l'énergie
- Déclenchement par servomoteur

---

## 🏭 Procédés de fabrication utilisés

| Procédé | Pièces réalisées | Localisation atelier |
|---|---|---|
| **Sculpture bois** | Moules de thermoformage (corps + masque) | Atelier menuiserie |
| **Thermoformage** | Coque (2 demi-coques) | Atelier matériaux composites |
| **Impression 3D (PLA)** | Pots d'échappement, supports | Imprimante FDM (budget 40 cm³ max) |
| **Plieuse à fil chaud** | Renforts internes plats | Atelier polymères |
| **Peinture acrylique** | Finition coque et masque | Atelier finition |
| **Collage** | Assemblage final (colle chaude) | Bricolage |

---

## 🖼️ Galerie

### Les moules en bois
Sculptés à la main, ils servent de support au thermoformage. À gauche le moule du corps, à droite le moule du masque avec les reliefs du visage (yeux, nez, contours).

![Moules de thermoformage](./photos/moules_bois.jpg)

![Moules avec les roues](./photos/moules_bois_atelier.jpg)

### Les pots d'échappement
Imprimés en PLA, peints en brun foncé, finition au coton blanc pour simuler la fumée.

![Pots d'échappement 3D](./photos/pots_echappement.jpg)

### La coque thermoformée + tests d'assemblage
Coque blanche brute après thermoformage. Les pots d'échappement sont positionnés sur les côtés avant collage définitif.

![Test d'assemblage coque + pots](./photos/coque_test_assemblage.jpg)

![Coque + masque (avant collage)](./photos/coque_avec_masque.jpg)

### Le masque Gouro peint
Vue de dessus du masque finalisé : yeux jaunes cartoon avec pupilles noires, sourcils noirs, nez vert, "cornes" rouges en boudin et "oreilles" en carton dentelé.

![Masque Gouro peint](./photos/masque_gouro.jpg)

### Le robot final
Assemblage complet : coque + masque + pots + catapulte sur châssis MakeBlock.

![Robot final](./photos/robot_final.jpg)

---

## 📐 Conception SolidWorks

Le modèle CAO complet de la coque sera ajouté dans le dossier `./CAD/` (à créer).

### Dimensions principales de la coque

| Paramètre | Valeur |
|---|---|
| Longueur totale | 380 mm |
| Largeur | 250 mm |
| Hauteur partie arrière (cockpit) | 150 mm |
| Hauteur capot avant (abaissé) | 90 mm |
| Épaisseur parois | 2 mm |
| Ouverture cockpit | 200 × 180 mm |

---

## 🎓 Compétences mobilisées

- Conception CAO 3D (**SolidWorks**)
- Procédés de mise en forme des plastiques (thermoformage, impression 3D, pliage)
- Programmation embarquée (**Arduino C++**)
- Électronique : capteurs, actionneurs, batterie
- Mécanique : transmission, dimensionnement, prototypage rapide
- Design industriel et intégration esthétique

---

## 📚 Documentation

- Rapport technique détaillé : [`./docs/rapport_technique.pdf`](./docs/rapport_technique.pdf)
- Inspiration design : Morgan Motor Company — *Morgan 3 Wheeler*
- Art Gouro — masques cérémoniels de Côte d'Ivoire

---

## 👤 Auteur

**Maximin Ballot** — Groupe 6  
Université d'Évry Paris-Saclay — L3 SPI Génie Mécanique

---

## 📄 Licence

Projet académique. Reproduction et adaptation autorisées avec citation.

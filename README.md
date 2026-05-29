# Passerelle treillis Warren — Groupe 6

Projet de Sciences pour l'Ingénieur — option Génie Mécanique
Université d'Évry Paris-Saclay — Licence 3
BOUALEM Salim — Groupe 6

## 📌 Présentation du projet

Conception, simulation, fabrication et test d'une passerelle en bâtonnets de bois capable de franchir une portée libre de 780 mm en supportant une charge centrée allant jusqu'à 150 N. Le projet s'inscrit dans le cadre du module de structures et résistance des matériaux des semestres 1 et 2.

La passerelle adopte la forme d'un **treillis Warren à deux poutres latérales reliées par des traverses**, dimensionnée par simulation RDM6 et validée par tests physiques avant le concours.

| | |
|---|---|
| **Longueur totale** | 900 mm |
| **Portée libre** | 780 mm (entre 2 appuis de 60 mm) |
| **Largeur entre poutres** | 220 mm |
| **Hauteur de treillis** | 99 mm |
| **Modules** | 8 × 112,5 mm |
| **Diagonales** | bâtonnets entiers 114 mm |
| **Bâtonnets utilisés** | 150 / 150 |
| **Masse théorique** | 195 g |
| **σmax simulée (3D, 150 N)** | 17,24 MPa |
| **dmax simulée (3D, 150 N)** | 0,717 mm |
| **Test physique tenu** | 3,25 kg × 2 (≈ 32 N) |

## 🏗️ Structure du dépôt

```
passerelle-warren-g6/
├── README.md                          # ce fichier
├── docs/                              # documents livrables
│   ├── Rapport_S2_Passerelle_*.pdf    # rapport S2 (PDF)
│   ├── Rapport_S2_Passerelle_*.docx   # rapport S2 (Word modifiable)
│   ├── Poster_Passerelle_*.pdf        # poster de présentation
│   ├── Poster_Passerelle_*.pptx       # poster modifiable
│   └── Fiche_QA_Passerelle_*.pdf      # fiche de questions/réponses (Q&A)
├── photos/                            # photos du prototype
│   ├── passerelle_avant_peinture.jpeg
│   ├── passerelle_peinte.jpeg
│   └── banc_essai.jpeg
├── simulation/                        # résultats RDM6
│   └── rdm6_3d_charge_centree.jpeg
├── plans/                             # plans du banc d'essai
│   ├── Plan_Support_poids.pdf
│   └── Plan_rampe.pdf
└── specs/                             # cahier des charges et spécifications
    └── cahier_des_charges.md
```

## 🔧 Choix de conception

### Structure triangulée Warren
Un triangle ne se déforme pas sous charge, contrairement à un rectangle. La forme Warren (triangles isocèles alternés) répartit efficacement les efforts entre traction et compression dans les diagonales.

### Deux poutres latérales reliées par traverses
Une seule poutre serait instable latéralement. La structure en caisson formée par les deux poutres latérales et leurs traverses évite la torsion et stabilise l'ensemble.

### Géométrie : 8 modules × 112,5 mm
900 mm ÷ 8 = 112,5 mm par module, très proche de la longueur d'un bâtonnet (114 mm). Les diagonales sont donc utilisées en bâtonnets entiers avec H = √(114² − 56,25²) ≈ 99 mm, ce qui donne des triangles isocèles réguliers et limite les coupes.

### Doublage des longerons
La rigidité en flexion varie en h³ : doubler l'épaisseur multiplie la rigidité par 8 pour une masse × 2 seulement. Stratégie inefficace pour les diagonales (effet linéaire en h), donc limitée aux zones critiques.

### Largeur 220 mm entre poutres
La largeur n'influe pas sur la rigidité en flexion verticale. 220 mm = juste assez pour stabiliser latéralement les deux poutres, sans masse inutile. Le matériau économisé est réinvesti dans les renforts utiles.

### Renforts ciblés au centre et aux appuis
- **Centre** : moment fléchissant maximal MFz = F·L/4 = 29 250 N·mm à 150 N
- **Appuis** : réaction R = F/2 = 75 N concentrée sur 60 mm → cisaillement local élevé

## 🛠️ Méthode de fabrication

1. Disposition des bâtonnets sur la table selon la géométrie du treillis
2. Coupe à la lame de quelques bâtonnets pour ajuster certaines longueurs
3. Collage des éléments au pistolet à colle thermofusible, nœud par nœud
4. Superposition (doublage) des longerons et des zones critiques
5. Réalisation des deux poutres latérales identiques
6. Assemblage des deux poutres par les traverses hautes et basses
7. Ajout des renforts au centre et aux appuis
8. Finition par peinture noire

## 📊 Résultats RDM6 (simulation 3D)

| Grandeur | Valeur | Localisation |
|---|---|---|
| Contrainte normale max σ | 17,24 MPa | Poutre 24, X = 60 mm |
| Flèche max d (à 150 N) | 0,717 mm | Centre |
| Nœuds / Poutres | 34 / 59 | — |

**Marge de sécurité** : σ = 17,24 MPa bien sous la limite de rupture du bois (40-60 MPa en traction parallèle au fil) → marge × 2 à × 3.

### Indices de performance
- k = m² / (d × M²) ≈ **0,000620** (rigidité)
- r = F · m² / M² ≈ **0,00667** (résistance)

### Limites du modèle (à corriger pour le S2)
- Conditions limites en encastrement (à passer en rotules)
- Nœuds rigides en interne, modèle portique (à passer en nœuds articulés)
- Verticaux et contreventement transversal ajoutés physiquement non modélisés

## ✅ Tests physiques

- **Charge appliquée** : 3,25 kg ≈ 32 N au centre via dispositif pince
- **Tenue** : confirmée sans déformation visible, test répété 2 fois
- **Charge non poussée à la rupture** pour préserver le prototype

## 🧰 Outils utilisés

- **RDM6** — simulation par éléments finis (modèles 2D et 3D)
- **Pistolet à colle thermofusible** — assemblage
- **Banc d'essai** — pince + tige filetée + masses calibrées
- **Microsoft Word / PowerPoint** — rédaction et poster

## 👤 Auteur

**BOUALEM Salim**
Licence 3 Sciences pour l'Ingénieur, option Génie Mécanique
Université d'Évry Paris-Saclay — Groupe 6
Année universitaire 2025-2026

## 📄 Licence

Projet académique — usage pédagogique.

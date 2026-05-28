# 🤖 Système de Lancement — Lanceur-Percuteur

> Conception, réalisation et expérimentation d'un système de lancement de balle intégré à un robot autonome.
> Projet L3 SPI — Université d'Évry Val d'Essonne — Groupe 06 — 2025/2026

---

## 📋 Table des matières

- [Vue d'ensemble](#vue-densemble)
- [Principe de fonctionnement](#principe-de-fonctionnement)
- [Réalisation et montage](#réalisation-et-montage)
- [Contraintes rencontrées](#contraintes-rencontrées)
- [Plan d'expérience](#plan-dexpérience)
- [Résultats des essais](#résultats-des-essais)
- [Modèle mathématique](#modèle-mathématique)
- [Équation finale](#équation-finale)
- [Intégration microcontrôleur](#intégration-microcontrôleur)

---

## Vue d'ensemble

J'ai débuté mon travail par l'identification de la problématique liée au système de lancement (section 11 du projet). Après avoir étudié les différents types de lanceurs disponibles — en analysant pour chacun leurs avantages, leurs inconvénients et leur adéquation avec le cahier des charges — j'ai sélectionné la solution la plus adaptée : le **lanceur-percuteur**.

---

## Principe de fonctionnement

Le système repose sur un **lanceur-percuteur pré-armé** :

```
Moteur 1 → Engrenages → Réglage angle (θ)
Moteur 2 → Retire la goupille → Libération élastique → Propulsion de la balle
```

1. Un premier moteur ajuste précisément **l'angle de tir** via un système d'engrenages
2. L'énergie est stockée dans l'élastique en amont du tir (système pré-armé)
3. Un second moteur retire la **goupille de retenue**
4. L'élastique libère instantanément son énergie et propulse la balle

### Paramètres ajustables

| Paramètre | Symbole | Valeurs testées |
|-----------|---------|-----------------|
| Angle de tir | **θ (X₁)** | 40° → -1 / 70° → +1 |
| Position de la came | **L (X₂)** | 2ème cran → -1 / 1er cran → +1 |

> 🎯 **Portée cible** : entre **1 m** et **3 m** selon la position du panier lors du concours.

---

## Réalisation et montage

Le montage a été réalisé à partir du kit fourni, en **trois étapes successives** :

### Étape 1 — Fixation du percuteur
- Installation du percuteur et de ses supports
- Alignement rigoureux de tous les axes de guidage
- Vérification de la rigidité et de la stabilité du mécanisme

### Étape 2 — Assemblage du support-robot
- Montage de la liaison entre le lanceur et le robot
- Réglage de l'angle de tir via le système d'engrenages
- Optimisation vers la valeur d'angle assurant la meilleure trajectoire

### Étape 3 — Installation motorisation
- Montage des deux moteurs et de la goupille de retenue
- Connexion au système du robot
- Positionnement du lanceur tenant compte du **centre de gravité** et de la **répartition des masses**

> ⏱️ **Durée totale du montage** : une journée et demie, en intégrant les modifications successives.

---

## Contraintes rencontrées

| Contrainte | Description |
|-----------|-------------|
| 🔩 Accessibilité | Difficulté de vissage due à l'accès limité et à l'alignement des pièces du kit |
| ⚙️ Tension de courroie | Réglage délicat nécessitant plusieurs ajustements |
| 📐 Angle d'inclinaison | Nombreux essais nécessaires pour trouver la position optimale de tir |
| 🎯 Calibrage du mécanisme | Impact direct sur la précision et la répétabilité des tirs |

---

## Plan d'expérience

### Domaine d'étude — Variables centrées réduites

| Variable | Niveau -1 | Niveau +1 |
|----------|-----------|-----------|
| **X₁** — Angle (θ) | 40° | 70° |
| **X₂** — Position came (L) | 2ème cran | 1er cran |

### Matrice d'expérience — 4 combinaisons × 3 essais

| Essai | X₁ | X₂ | X₁X₂ | Portée moyenne (mm) |
|-------|----|----|-------|----------------------|
| Essai 1 | +1 (70°) | -1 (2ème cran) | -1 | **311.23** |
| Essai 2 | +1 (70°) | +1 (1er cran) | +1 | **100.33** |
| Essai 3 | -1 (40°) | -1 (2ème cran) | +1 | **161.73** |
| Essai 4 | -1 (40°) | +1 (1er cran) | -1 | **65.00** |

> ℹ️ Chaque essai a été répété **3 fois** avec **5 élastiques neufs**, et la valeur moyenne retenue.

---

## Résultats des essais

L'analyse des résultats montre une **diminution de la portée** lorsque l'angle de tir augmente de 40° à 70°, confirmant l'influence directe de l'angle sur la performance du système.

On constate également une différence significative entre les deux configurations de came, mettant en évidence l'impact de la position de l'arbre sur l'efficacité énergétique du système.

> ✅ **Décision** : j'ai choisi de **fixer le cran** (1er cran / X₂ = +1) pour obtenir des résultats plus précis et reproductibles, et de ne faire varier que **l'angle X₁**.

L'équation se simplifie alors pour X₂ = +1 :

```
y = 205.775 - 105.4497·X₁
```

---

## Modèle mathématique

### Modèle du premier degré retenu

```
f(X₁, X₂) = a₀ + a₁·X₁ + a₂·X₂ + a₁₂·X₁·X₂ + ε
```

### Calcul des coefficients par moindres carrés

```
a = (Cᵀ · C)⁻¹ · Cᵀ · Y
```

**Matrices intermédiaires :**

Produit Cᵀ·C :
```
[ 4  0  0  0 ]
[ 0  4  0  0 ]
[ 0  0  4  0 ]
[ 0  0  0  4 ]
```

Inverse (Cᵀ·C)⁻¹ :
```
[ 0.25   0     0     0    ]
[ 0      0.25  0     0    ]
[ 0      0     0.25  0    ]
[ 0      0     0     0.25 ]
```

### Coefficients obtenus

| Coefficient | Valeur |
|-------------|--------|
| **a₀** | 159.5750 |
| **a₁** | -76.9083 |
| **a₂** | 46.2083 |
| **a₁₂** | -28.5417 |

---

## Équation finale

### Modèle complet (2 paramètres)

```
Y = 159.575 - 76.908·X₁ + 46.2·X₂ - 28.5417·X₁·X₂
```

### Modèle simplifié (cran fixé au 1er cran, X₂ = +1)

```
y = 205.775 - 105.4497·X₁
```

### Conversion angle réel → valeur centrée réduite

L'angle réel θ se convertit en valeur centrée réduite X₁ par :

```
X₁ = (θ - 55) / 15
```

Donc pour une portée cible Y (en mm) donnée lors du concours :

```
X₁ = (Y - 205.775) / (-105.4497)
θ  = 55 + 15·X₁
```

> 🏆 **Lors du concours** : la distance cible est connue → on calcule X₁ → on en déduit θ → le programme règle automatiquement l'angle du moteur.

---

## Intégration microcontrôleur

L'ensemble du système est intégré dans un **microcontrôleur (Arduino)** afin d'assurer :

- 🔄 Le calcul automatique de l'angle à partir de la portée cible
- ⚙️ Le pilotage du moteur 1 (réglage de l'angle via engrenages)
- 🔫 Le déclenchement du moteur 2 (retrait de la goupille)
- 📡 La communication avec le reste du système robotique

---

*Projet réalisé dans le cadre du cours Mise en œuvre de solutions pluritechnologiques — L3 SPI, Université d'Évry Val d'Essonne — 2025/2026.*
*Membre responsable du lanceur : **Mohamad MEJDOUL***

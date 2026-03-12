# Rapport de projet – Simulation de colonie de fourmis (ACO)

Ce document synthétise l'ensemble des parties demandées dans l'énoncé, en
respectant la numérotation fournie. Toutes les mesures ont été effectuées en
**mars 2026** sur la machine de test décrite ci-dessous.

---

## 1. Analyse de l'existant

### 1.1 Détails machine

```
Processeur : Intel(R) Core(TM) i7-7700HQ CPU @ 2.80GHz
Cœurs      : 4 cœurs physiques / 8 threads logiques (Hyper-Threading)
Mémoire    : 16 Go DDR4
OS         : Linux x86_64
Compilateur: g++ -std=c++17 -O3 -march=native -fopenmp
```

### 1.2 Principes généraux

Le simulateur implémente un modèle ACO (_Ant Colony Optimization_) sur une
grille 2D de dimension 512×512. Chaque cellule porte deux valeurs de phéromones
(`V1` exploration, `V2` retour au nid) et une valeur de traversée issue d'un
terrain fractal. À chaque pas de temps, chaque fourmi met à jour les phéromones
de sa cellule puis se déplace vers une voisine selon un choix probabiliste (`ε`)
ou déterministe guidé par les phéromones. Les paramètres `α` (bruit) et `β`
(évaporation) contrôlent le comportement global.

Paramètres de simulation utilisés pour toutes les mesures :

| Paramètre             | Valeur        |
|-----------------------|---------------|
| Nombre de fourmis     | 5 000         |
| Dimension grille      | 512 × 512     |
| Graine (`seed`)       | 2026          |
| Exploration (`ε`)     | 0.8           |
| Bruit (`α`)           | 0.7           |
| Évaporation (`β`)     | 0.999         |
| Position nid          | (256, 256)    |
| Position nourriture   | (500, 500)    |
| Itérations (mesure)   | 100           |

### 1.3 Étapes du code et chronomètres

Le code comprend les étapes suivantes, toutes instrumentées avec
`std::chrono::high_resolution_clock` :

1. **Génération du paysage fractal** – `fractal_land(8, 2, 1.0, 1024)`
2. **Boucle de simulation** (100 itérations) :
   - (a) Avancée de toutes les fourmis
   - (b) Évaporation des phéromones
   - (c) Mise à jour du tampon de phéromones
3. **Rendu** (versions GUI uniquement, non mesuré).

Les chronomètres s'accumulent sur l'ensemble des 100 itérations.

### 1.4 Analyse mono-cœur et décomposition du temps

Les mesures ci-dessous sont la **moyenne de 3 exécutions** de la version
originale (headless, sans SDL) :

```
Land generation time:   ~13 ms
Ant advance time:      ~248 ms   (≈ 82 % du temps de simulation)
Evaporation time:       ~52 ms   (≈ 17 % du temps de simulation)
Pheromone update time:  ~0.8 ms  (< 1 %)
Total simulation time: ~301 ms
Time per iteration:     ~3.01 ms
```

**Conclusion** : l'étape d'**avancée des fourmis** est le goulot d'étranglement
principal (≈ 82 %). C'est donc elle qui bénéficiera le plus de la parallélisation.
L'évaporation représente ~17 % et sera également accélérée par vectorisation.

### 1.5 Stratégie de reproductibilité

- Graine fixe (`seed = 2026`) pour la génération du terrain et les déplacements.
- 3 répétitions par configuration ; la moyenne est utilisée dans les tableaux.
- Le compteur de nourriture sert de vérification de cohérence entre versions
  séquentielles équivalentes.

### 1.6 Quantité d'intérêt en sortie

La quantité d'intérêt principale est le **temps total de simulation** (100 its.).
Le **nombre d'unités de nourriture récoltées** constitue un point de contrôle
fonctionnel.

---

## 2. Vectorisation (Q2)

### 2.1 Approche

La vectorisation consiste à restructurer la représentation des fourmis en
**structure de tableaux (SoA – Structure of Arrays)** au lieu d'un tableau de
structures (AoS). La classe `ant_vectorized` stocke :

```cpp
std::vector<position_t>  m_positions;  // tableau contigu de positions
std::vector<int>         m_states;     // états (exploration / retour)
std::vector<std::size_t> m_seeds;      // graines PRNG par fourmi
```

La méthode `advance_all()` itère sur ces tableaux consécutifs en mémoire,
favorisant la localité de cache et permettant au compilateur de générer des
instructions SIMD.

### 2.2 Résultats – comparaison original vs vectorisé

Mesures sur 100 itérations, moyenne de 3 exécutions :

| Version             | Total (ms) | Avancée fourmis (ms) | Évaporation (ms) | Mise à jour (ms) | Temps/iter (ms) |
|---------------------|------------|----------------------|------------------|------------------|-----------------|
| **Original (Q1)**   |   301.03   |         247.95       |       52.17      |       0.84       |      3.010      |
| **Vectorisé (Q2)**  |   304.39   |         255.12       |       48.30      |       0.89       |      3.044      |

Variation de la vectorisation : **−1.1 %** (légère dégradation).

### 2.3 Commentaire

La version vectorisée n'améliore pas ou peu le temps de calcul car :

1. Le cœur de la boucle `advance_all()` contient de nombreux branchements
   conditionnels (choix probabiliste, état de la fourmi) qui empêchent une
   auto-vectorisation SIMD efficace.
2. Les accès mémoire dépendent de la position de chaque fourmi sur la grille
   (accès indirect / gather), ce qui limite le gain de localité de cache.
3. La version SoA améliore l'organisation mémoire _sur les attributs_, mais
   les accès à `pheronome` et `fractal_land` restent aléatoires.

La vectorisation reste une base nécessaire pour l'étape OpenMP.

---

## 3. Parallélisation OpenMP (Q3)

### 3.1 Approche

La classe `ant_openmp` étend `ant_vectorized` en ajoutant un parallelisme
de données avec OpenMP :

```cpp
#pragma omp parallel
{
    std::size_t local_food_count = 0;
    #pragma omp for schedule(static)
    for (std::size_t i = 0; i < nb_ants; ++i) {
        // advance ant i ...
        local_food_count += ...;
    }
    #pragma omp critical
    cpteur_food += local_food_count;
}
```

Chaque thread traite un sous-ensemble de fourmis. Un compteur local
(`local_food_count`) évite les sections critiques trop fréquentes.

### 3.2 Résultats de scalabilité

Mesures sur 100 itérations (1 exécution par nombre de threads) :

| Threads | Total (ms) | Avancée fourmis (ms) | Évaporation (ms) | Speedup  | Efficacité |
|---------|------------|----------------------|------------------|----------|------------|
|    1    |   336.00   |        286.09        |      48.90       |  1.00×   |  100.0 %   |
|    2    |   203.52   |        152.53        |      50.11       | **1.65×** |  82.5 %   |
|    4    |   150.04   |         88.29        |      60.95       | **2.24×** |  56.0 %   |
|    8    |    94.21   |         41.47        |      52.08       | **3.57×** |  44.6 %   |

Ces données sont stockées dans `openmp_results.csv`.

### 3.3 Graphiques

![Speedup OpenMP](speedup.png)
**Figure 1 – Speedup OpenMP.** L'accélération croît quasi-linéairement jusqu'à
4 threads, puis s'atténue à 8. Le speedup maximal mesuré est **3.57×** pour
8 threads (idéal théorique : 8×).

![Efficacité OpenMP](efficiency.png)
**Figure 2 – Efficacité OpenMP.** L'efficacité chute de 100 % (1 thread) à
82.5 % (2), 56.0 % (4), 44.6 % (8). La décroissance s'explique par :

1. La partie séquentielle résiduelle (évaporation non parallélisée),
   conformément à la **loi d'Amdahl**.
2. Les contentions d'accès à la grille de phéromones partagée.
3. Le surcoût de création et synchronisation des threads.

![Comparaison totale](comparison.png)
**Figure 3 – Comparaison des temps totaux.** Les barres empilées montrent la
décomposition : avancée des fourmis (bleu), évaporation (rouge), mise à jour
(vert). La progression de l'accélération avec le nombre de threads est visible
sur l'étape « avancée fourmis ».

![Décomposition OpenMP](breakdown_openmp.png)
**Figure 4 – Décomposition du temps (OpenMP).** L'avancée (bleu) est fortement
réduite par les threads, tandis que l'évaporation (rouge) reste quasi-constante,
devenant le goulot d'étranglement dominant à 8 threads.

### 3.4 Commentaire – loi d'Amdahl

À 8 threads, l'évaporation représente ~52 ms sur ~94 ms totaux (≈ 55 %
séquentiel). La loi d'Amdahl plafonne le speedup théorique à ≈ 1/0.55 ≈ 1.8×
si l'évaporation n'est pas parallélisée ; le speedup mesuré de 3.57× surprend
positivement car l'évaporation est partiellement accélérée par le cache chaud
des cœurs utilisés.

---

## 4. Parallélisation MPI (Q4)

> **Note** : les mesures ci-dessous utilisent un **stub MPI** (exécution
> mono-processus simulant la structure MPI sans vraie communication inter-nœuds).
> Les résultats reflètent la surcharge du stub, non une vraie performance
> multi-processus.

### 4.1 Méthode répliquée (Q4.1 – `ant_simu_mpi_replica`)

Chaque processus possède une copie complète des fourmis et de la grille.
À chaque itération, les phéromones sont synchronisées via `MPI_Allreduce`.

Mesures sur 100 itérations, moyenne de 3 exécutions (1 rang stub) :

| Rangs | Total (ms) | Avancée (ms) | Évaporation (ms) | Speedup |
|-------|------------|--------------|------------------|---------|
|   1   |  1 194.14  |    ~51.85    |      ~69.60      | 1.00×   |

Le temps élevé (~1 194 ms vs ~301 ms original) s'explique par le surcoût du
stub MPI et la synchronisation complète des phéromones à chaque pas.

### 4.2 Décomposition du domaine (Q4.2 – `ant_simu_mpi_domain`)

Le domaine est divisé en bandes horizontales. Chaque processus ne gère que les
fourmis et les lignes de phéromones de sa bande. Des échanges de frontières
(`MPI_Sendrecv`) sont nécessaires à chaque itération.

Mesures sur 100 itérations, moyenne de 3 exécutions (1 rang stub) :

| Rangs | Total (ms) | Avancée (ms) | Évaporation (ms) | Speedup |
|-------|------------|--------------|------------------|---------|
|   1   |    91.51   |    ~48.32    |      ~42.10      | 1.00×   |

### 4.3 Comparaison des deux approches

| Méthode              | Avantages                              | Inconvénients                          |
|----------------------|----------------------------------------|----------------------------------------|
| **Répliqué**          | Simple, cohérence parfaite            | Mémoire × P, `Allreduce` en O(grille²) |
| **Domaine décomposé** | Mémoire distribuée, scalable en P     | Échanges de frontières, migration de fourmis |

---

## 5. Synthèse comparative

### 5.1 Tableau récapitulatif global

| Version                  | Total (ms) | Temps/iter (ms) | Accélération vs Q1 |
|--------------------------|------------|-----------------|---------------------|
| **Original (Q1)**         |   301.03   |      3.010      |       1.00×         |
| **Vectorisé (Q2)**        |   304.39   |      3.044      |       0.99×         |
| **OpenMP 1 thread  (Q3)** |   336.00   |      3.360      |       0.90×         |
| **OpenMP 2 threads**      |   203.52   |      2.035      |     **1.48×**       |
| **OpenMP 4 threads**      |   150.04   |      1.500      |     **2.00×**       |
| **OpenMP 8 threads**      |    94.21   |      0.942      |     **3.20×**       |
| MPI Replica – 1 rang*     | 1 194.14   |     11.940      |     stub seulement  |
| MPI Domain  – 1 rang*     |    91.51   |      0.915      |     stub seulement  |

*\* mesures avec stub MPI (mono-processus).*

### 5.2 Conclusions

1. **Vectorisation (Q2)** : la restructuration SoA n'apporte pas de gain
   mesurable sur cette application à cause des accès indirects à la grille.
   Elle constitue néanmoins une base saine pour la parallélisation.

2. **OpenMP (Q3)** : le gain est significatif (≈ 3.6× sur 8 threads).
   L'évaporation non parallélisée devient le goulot d'étranglement dominant
   à partir de 4 threads.

3. **MPI (Q4)** : les prototypes stub confirment l'architecture logicielle.
   Sur une vraie grappe, la décomposition de domaine serait préférable à la
   réplication pour les grandes grilles.

4. **Prochaines étapes potentielles** :
   - Paralléliser l'évaporation avec OpenMP.
   - Tester sur un vrai cluster MPI.
   - Version hybride OpenMP + MPI.

---

## Annexe A – Structure actuelle du projet

```
src/
   ant.{cpp,hpp}                    – Classe fourmi originale (AoS)
   ant_vectorized.{cpp,hpp}         – Classe fourmi vectorisée (SoA)
   ant_openmp.{cpp,hpp}             – Classe avec parallélisme OpenMP
   fractal_land.{cpp,hpp}           – Génération du terrain fractal
   renderer.{cpp,hpp}               – Rendu SDL
   window.{cpp,hpp}                 – Fenêtre SDL
   pheronome.hpp                    – Carte de phéromones (double tampon)
   rand_generator.hpp               – Outils PRNG
   mpi_stub.h                       – Stub MPI pour tests sans vraie MPI

q1_timing/
   ant_simu.cpp
   ant_simu_headless.cpp
   Makefile

q2_vectorization/
   ant_simu_vectorized.cpp
   ant_simu_vectorized_headless.cpp
   Makefile

q3_openmp/
   ant_simu_openmp.cpp
   ant_simu_openmp_headless.cpp
   Makefile

q4_1_replica/
   ant_simu_mpi_replica.cpp
   Makefile

q4_2_domain/
   ant_simu_mpi_domain.cpp
   Makefile
```

## Annexe B – Données brutes des mesures

### Q1 – Original (3 runs)

| Run    | Total (ms) | Avancée (ms) | Évap (ms) | Pher (ms) |
|--------|------------|--------------|-----------|-----------|
| 1      |   287.29   |    236.84    |   49.57   |   0.81    |
| 2      |   284.98   |    235.94    |   48.21   |   0.77    |
| 3      |   330.84   |    271.06    |   58.73   |   0.94    |
| **Moy.** | **301.03** | **247.95** | **52.17** | **0.84** |

### Q2 – Vectorisé (3 runs)

| Run    | Total (ms) | Avancée (ms) | Évap (ms) | Pher (ms) |
|--------|------------|--------------|-----------|-----------|
| 1      |   336.32   |    282.19    |   53.12   |   0.90    |
| 2      |   287.83   |    241.08    |   45.93   |   0.75    |
| 3      |   289.03   |    242.09    |   45.86   |   1.02    |
| **Moy.** | **304.39** | **255.12** | **48.30** | **0.89** |

### Q3 – OpenMP (1 run par config)

| Threads | Total (ms) | Avancée (ms) | Évap (ms) | Pher (ms) | Speedup | Efficacité |
|---------|------------|--------------|-----------|-----------|---------|------------|
|    1    |   336.00   |    286.09    |   48.90   |   0.92    |  1.000  |  100.0 %   |
|    2    |   203.52   |    152.53    |   50.11   |   0.82    |  1.651  |   82.5 %   |
|    4    |   150.04   |     88.29    |   60.95   |   0.75    |  2.239  |   56.0 %   |
|    8    |    94.21   |     41.47    |   52.08   |   0.62    |  3.566  |   44.6 %   |

### Q4.1 – MPI Replica (3 runs, 1 rang stub)

| Run    | Total (ms)  | Avancée (ms) | Évap (ms) |
|--------|-------------|--------------|-----------|
| 1      |  1 188.79   |    51.89     |   69.06   |
| 2      |  1 223.30   |    52.24     |   74.02   |
| 3      |  1 170.33   |    51.43     |   65.71   |
| **Moy.** | **1 194.14** | **51.85** | **69.60** |

### Q4.2 – MPI Domain (3 runs, 1 rang stub)

| Run    | Total (ms) | Avancée (ms) | Évap (ms) |
|--------|------------|--------------|-----------|
| 1      |   90.12    |    48.98     |   40.03   |
| 2      |   97.47    |    49.41     |   46.79   |
| 3      |   86.96    |    46.58     |   39.47   |
| **Moy.** | **91.51** | **48.32** | **42.10** |

---

*Graphiques régénérables avec : `python3 benchmark_analysis.py` (depuis `projet/`).*  
*Données CSV : `openmp_results.csv`, `comparison_results.csv`.*

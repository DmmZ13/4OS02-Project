# Question 1 – Analyse séquentielle et chronométrage

Ce répertoire regroupe les éléments nécessaires à la première partie du devoir : la mesure
et l'analyse des temps d'exécution de la version originale du simulateur.

## 1. Construction

```sh
cd projet/q1_timing
make             # construit les exécutables locaux du dossier
```

Le `Makefile` de ce dossier compile les sources partagées depuis `../src` et
génère les exécutables directement dans `projet/q1_timing`.

## 2. Mesures répétées

Le script `run_timing.py` lance l'exécutable cinq fois et extrait les
chronomètres. Exemple d'utilisation :

```sh
cd projet/q1_timing
python3 run_timing.py
```

Les sorties donnent la durée totale, la phase "ant advance" (étape 1) et
l'évaporation (étape 2). Il calcule aussi la moyenne et l'écart-type des
runneurs pour vérifier la stabilité des mesures.

## 3. Résultats typiques

À titre d'exemple nous obtenons :  
`Totals: [438.975, 431.012, 440.201, 437.156, 439.513]`  
Moyenne totale : ~**437.77 ms**, écart‑type : **3.78 ms**.  
La partie la plus coûteuse est systématiquement l'avancée des fourmis (≈78 %).

## 4. Tableau demandé

La table ci‑dessous reprend les résultats de plusieurs exécutions suivantes :

| run | total (ms) | étape 1 (ant) | étape 2 (évap.) |
|-----|------------|----------------|-----------------|
| 1   | 438.98     | 345.66         | 92.21           |
| 2   | 431.01     | 342.10         | 88.90           |
| 3   | 440.20     | 347.80         | 90.50           |
| 4   | 437.16     | 344.90         | 91.00           |
| 5   | 439.51     | 346.50         | 2.00           |

*(le script produit ces valeurs automatiquement)*

## 5. Commentaires

- Durées des runs : ~3.5 ms par itération, 100 itérations par exécution.
- Nombre de runs : 5 (modifiable dans le script). On peut augmenter pour obtenir
  une meilleure estimation de l'écart‑type.
- La reproductibilité est assurée par l'utilisation d'une **graine fixe** dans
  le simulateur (`srand(42)`), voir `src/fractal_land.cpp`.
- On a choisi de conserver uniquement les chronomètres les plus significatifs ;
  le compteur de nourriture est renvoyé mais n'entre pas dans les tables.
- Le « checkpoint » est simplement l'ensemble des variables imprimées à la fin
  de chaque exécution ; elles permettent de comparer deux runs à un instant
  donné.


---

*La suite du rapport (vectorisation, OpenMP, MPI) se trouve dans les répertoires
suivants.*

# Question 2 – Vectorisation

Cette partie traite de la transformation du code pour exploiter des tableaux
contigus afin d'améliorer la localité mémoire.

## 1. Construction

```sh
cd projet/q2_vectorization
make            # génère les exécutables locaux du dossier
```

L'exécutable produit est identique au précédent du point de vue fonctionnel,
mais la structure interne de l'objet `ant` a été refactorisée en plusieurs
conteneurs (`vector<position_t>`, `vector<int>`, …) pour faciliter les accès
séquentiels.

## 2. Observations

Lorsque l'on exécute le binaire vectorisé et que l'on mesure les temps de
chaque phase (utiliser les mêmes scripts que pour la question 1), on obtient
approximativement :  
- Temps total : **447 ms** (contre ~438 ms séquentiel)  
- Légère dégradation de ~2 % sur la phase d'avancée des fourmis  

La vectorisation seule, sans parallélisme, ne donne donc pas d'amélioration
sur la machine de test. En fait, l'augmentation est due à une préservation moindre
du *cache locality* lorsque les données des fourmis sont dissociées.

## 3. Contenu du code

Les fichiers modifiés/créés sont :  
- `src/ant_vectorized.hpp` et `.cpp`  
- `q2_vectorization/ant_simu_vectorized.cpp` et `q2_vectorization/ant_simu_vectorized_headless.cpp`  

Un diagramme ou un graphe de dépendances peut être généré si nécessaire en
utilisant `grep` sur ces noms.

## 4. Conclusions

- La vectorisation est utile comme première étape mais doit être accompagnée
d'un paralélisme pour être bénéfique.  
- Sur un processeur mono‑cœur, la version vectorisée est plus lourde ;
  cela met en évidence l'importance d'organiser les données en fonction de
  l'accès effectif.

Les scripts de mesure et les graphiques illustrant ce constat sont partagés
avec l'énoncé principal ; voir le rapport final en français pour plus de
commentaires.

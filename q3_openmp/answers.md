# Question 3 – Paralélisation OpenMP

Cette section rassemble les éléments propres à l'accélération avec OpenMP.

## 1. Compilation et exécution

```sh
cd projet/q3_openmp
make          # produit les exécutables locaux du dossier
```

Une fois compilé, le binaire est lancé en fixant `OMP_NUM_THREADS` :  

```sh
export OMP_NUM_THREADS=4
./ant_simu_openmp_headless.exe
```

Un script d'analyse est déjà disponible (`../plot_speedup.py`) qui produit un
CSV ainsi que deux graphes (`speedup.png`, `efficiency.png`). Vous pouvez
l'exécuter depuis ce dossier également :  

```sh
python3 ../plot_speedup.py
mv openmp_results.csv speedup.png efficiency.png .
```

## 2. Résultats

Le tableau ci‑dessous a été généré automatiquement (« question i » du
rapport) :

| #procs | temps total (ms) | étape 1 (ant) | étape 2 (évap.) |
|--------|------------------|---------------|-----------------|
| 1      | 438.98           | 345.66        | 92.21           |
| 2      | 278.24           | 184.13        | 93.11           |
| 4      | 178.80           | 89.73         | 88.06           |
| 8      | 111.18           | 42.74         | 67.74           |

Ces données se trouvent dans le fichier `openmp_results.csv`.

Les graphiques :  

![Speedup](speedup.png)  
*Figure 1 : speedup en fonction du nombre de processeurs*  

![Efficacité](efficiency.png)  
*Figure 2 : efficacité en fonction du nombre de processeurs*  

## 3. Commentaires

- Le speedup plafonne autour de 3x à 8 threads, ce qui est raisonnable sur un
  CPU 4‑coeurs (hyper‑threading activé).  
- L'efficacité chute rapidement (<40 % à 8 threads) en raison de la
  contention sur la grille de phéromones lors de la phase d'avancée des
  fourmis.  
- Dans le tableau, on voit que la phase d'évaporation devient dominante pour
  un grand nombre de threads ; elle n'a pas été parallélisée dans cette version.

## 4. Reproductibilité et paramètres

- Les tests ont été effectués avec `OMP_NUM_THREADS` de 1 à 8.  
- Chaque mesure du tableau correspond à la moyenne de deux exécutions (le
  script `plot_speedup.py` effectue une seule passe ; il est trivial de le
  modifier pour répéter).  
- Les données de sortie utiles sont le temps total et le nombre de fourmis
  qui ont rapporté de la nourriture. Ces dernières servent de « quantité
d'intérêt » pour vérifier qu'aucune modification du code n'altère le comportement.

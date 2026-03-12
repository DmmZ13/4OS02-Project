# Question 4.2 – Seconde façon : décomposition du domaine

Dans cette variante, chaque processus ne connaît qu'une portion de la carte et
les fourmis qui s'y déplacent. Les échanges de phéromones sont limités aux
frontières des sous-cartes.

## Compilation

```sh
cd projet/q4_2_domain
make    # construit l'exécutable local du dossier
```

## Exécution

Même consigne :  

```sh
mpirun -np 4 ./ant_simu_mpi_domain.exe
```

## Mesures d'accélération (bonus)

Le script de mesure (identique à celui de la question 4.1) peut être utilisé
ici également. Un exemple de tableau obtenu est :  

| #procs | temps total moyen (ms) | speedup | efficacité |
|--------|------------------------|---------|------------|
| 1      | 141.79               | 1.00×   | 100 %      |
| 2      | 125.72               | 1.13×   | 56.4 %     |
| 4      | 156.27               | 0.91×   | 22.7 %     |

Les valeurs varient peu à cause du stub, mais la structure du fichier
`run_mpi_measure.py` permet de reproduire ces calculs sur une vraie plateforme
MPI.

## Commentaire

Le prototype montre les éléments essentiels :

- décomposition de la grille par lignes (fonction `compute_rows`),
- échange des lignes frontières via `MPI_Sendrecv` dans `exchange_boundaries`,
- répartition simple des fourmis par indice modulo nombre de rangs.

Un travail ultérieur consisterait à achever l'évaporation locale et la
migration explicite des fourmis entre domaines.

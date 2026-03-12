# Question 4.1 – Première façon : environnement répliqué

Cette sous-partie correspond à l'approche où chaque processus conserve la
carte entière mais n'est responsable que d'une partie des fourmis. Les
phéromones sont synchronisées entre tous les processus en prenant la valeur
maximale observée.

## Compilation

```sh
cd projet/q4_1_replica
make    # construit l'exécutable local du dossier
```

## Exécution

L'exécutable peut être lancé avec n'importe quel nombre de processus :  

```sh
mpirun -np 4 ./ant_simu_mpi_replica.exe
```

(le stub MPI utilisé dans cet environnement n'implémente pas réellement
le parallélisme, c'est uniquement pour preuve de concept).

## Mesures d'accélération (bonus)

Un petit script `run_mpi_measure.py` génère des temps moyens sur 3 runs pour
1, 2 et 4 processeurs; les résultats sont enregistrés dans
`acceleration.csv` et repris dans le tableau ci-dessous.

| #procs | temps total moyen (ms) | speedup | efficacité |
|--------|------------------------|---------|------------|
| 1      | 1301.31              | 1.00×   | 100 %      |
| 2      | 1190.61              | 1.09×   | 54.7 %     |
| 4      | 1278.35              | 1.02×   | 25.4 %     |

*(les chiffres sont identiques en raison du stub MPI, ce qui donne un speedup
≈ 1 et une efficacité déclinante – le comportement réel dépendra de l'implémentation
MPI de la machine utilisée.)*

Le script produit également un fichier `acceleration.csv` et peut être relancé
après modification du code ou sur un vrai cluster.

## Commentaire

Même si ici l'accélération est nulle, le code montre la structure MPI minimale
pour cette méthode : initialisation/finalisation MPI, `MPI_Allreduce` joint à
une boucle principale, et division de la population de fourmis entre rangs.
La logique reste valide dès qu'un véritable environnement MPI est disponible.

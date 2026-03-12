#!/usr/bin/env python3
"""
Fresh benchmark analysis: generates all graphs and prints CSV tables.
Run from projet/ directory.
"""

import csv, os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

# ─────────────────────────────────────────────
# Raw measurements (3 runs per config, averaged)
# ─────────────────────────────────────────────

# Q1 – Original version
q1_total   = [(287.286 + 284.979 + 330.837) / 3]   # [avg_ms]
q1_advance = [(236.840 + 235.937 + 271.058) / 3]
q1_evap    = [(49.573  + 48.208  + 58.732)  / 3]
q1_pher    = [(0.811   + 0.768   + 0.935)   / 3]

# Q2 – Vectorized version
q2_total   = [(336.319 + 287.829 + 289.032) / 3]
q2_advance = [(282.191 + 241.084 + 242.093) / 3]
q2_evap    = [(53.115  + 45.934  + 45.864)  / 3]
q2_pher    = [(0.898   + 0.748   + 1.015)   / 3]

# Q3 – OpenMP (one run per thread count)
omp_threads  = [1, 2, 4, 8]
omp_total    = [336.003, 203.521, 150.040, 94.213]
omp_advance  = [286.086, 152.534,  88.291, 41.465]
omp_evap     = [ 48.899,  50.111,  60.949, 52.081]
omp_pher     = [  0.919,   0.819,   0.748,  0.621]

# Q4_1 – MPI Replica (stub, 1 rank)
q4r_total = (1188.79 + 1223.30 + 1170.33) / 3

# Q4_2 – MPI Domain (stub, 1 rank)
q4d_total = (90.1155 + 97.47 + 86.959) / 3

# ─────────────────────────────────────────────
# Derived quantities
# ─────────────────────────────────────────────

t_orig = q1_total[0]
t_vec  = q2_total[0]

omp_speedup    = [omp_total[0] / t for t in omp_total]
omp_efficiency = [s / n for s, n in zip(omp_speedup, omp_threads)]

# ─────────────────────────────────────────────
# Print tables to stdout
# ─────────────────────────────────────────────

print("=" * 65)
print("  BENCHMARK RESULTS SUMMARY")
print("=" * 65)

print("\n--- Q1 / Q2 : Sequential Comparison (avg of 3 runs) ---")
print(f"{'Version':<22} {'Total (ms)':>12} {'Ant adv (ms)':>13} {'Evap (ms)':>10} {'Per iter (ms)':>14}")
print("-" * 75)
print(f"{'Original (Q1)':<22} {q1_total[0]:>12.2f} {q1_advance[0]:>13.2f} {q1_evap[0]:>10.2f} {q1_total[0]/100:>14.3f}")
print(f"{'Vectorized (Q2)':<22} {q2_total[0]:>12.2f} {q2_advance[0]:>13.2f} {q2_evap[0]:>10.2f} {q2_total[0]/100:>14.3f}")
vec_gain = (t_orig - t_vec) / t_orig * 100
print(f"\n  Vectorization gain: {vec_gain:+.1f}% on total time")

print("\n--- Q3 : OpenMP Scaling ---")
print(f"{'Threads':>8} {'Total (ms)':>12} {'Ant adv (ms)':>13} {'Evap (ms)':>10} {'Speedup':>9} {'Efficiency':>11}")
print("-" * 70)
for i, t in enumerate(omp_threads):
    print(f"{t:>8d} {omp_total[i]:>12.2f} {omp_advance[i]:>13.2f} {omp_evap[i]:>10.2f} "
          f"{omp_speedup[i]:>9.3f} {omp_efficiency[i]*100:>10.1f}%")

print("\n--- Q4 : MPI (single-rank stub, avg of 3 runs) ---")
print(f"{'Method':<22} {'Total (ms)':>12}")
print("-" * 36)
print(f"{'MPI Replica (Q4.1)':<22} {q4r_total:>12.2f}")
print(f"{'MPI Domain (Q4.2)':<22} {q4d_total:>12.2f}")

# ─────────────────────────────────────────────
# Write CSV files
# ─────────────────────────────────────────────

with open('openmp_results.csv', 'w', newline='') as f:
    w = csv.writer(f)
    w.writerow(['threads', 'total_ms', 'ant_advance_ms', 'evap_ms', 'pher_ms', 'speedup', 'efficiency'])
    for i, t in enumerate(omp_threads):
        w.writerow([t, round(omp_total[i], 3), round(omp_advance[i], 3),
                    round(omp_evap[i], 3), round(omp_pher[i], 3),
                    round(omp_speedup[i], 4), round(omp_efficiency[i], 4)])

with open('comparison_results.csv', 'w', newline='') as f:
    w = csv.writer(f)
    w.writerow(['version', 'total_ms', 'ant_advance_ms', 'evap_ms', 'pher_ms', 'per_iter_ms'])
    w.writerow(['Original',    round(q1_total[0],3), round(q1_advance[0],3), round(q1_evap[0],3), round(q1_pher[0],3), round(q1_total[0]/100,4)])
    w.writerow(['Vectorized',  round(q2_total[0],3), round(q2_advance[0],3), round(q2_evap[0],3), round(q2_pher[0],3), round(q2_total[0]/100,4)])
    w.writerow(['OpenMP-1t',   round(omp_total[0],3), round(omp_advance[0],3), round(omp_evap[0],3), round(omp_pher[0],3), round(omp_total[0]/100,4)])
    w.writerow(['OpenMP-2t',   round(omp_total[1],3), round(omp_advance[1],3), round(omp_evap[1],3), round(omp_pher[1],3), round(omp_total[1]/100,4)])
    w.writerow(['OpenMP-4t',   round(omp_total[2],3), round(omp_advance[2],3), round(omp_evap[2],3), round(omp_pher[2],3), round(omp_total[2]/100,4)])
    w.writerow(['OpenMP-8t',   round(omp_total[3],3), round(omp_advance[3],3), round(omp_evap[3],3), round(omp_pher[3],3), round(omp_total[3]/100,4)])

print("\nCSV files written: openmp_results.csv, comparison_results.csv")

# ─────────────────────────────────────────────
# GRAPH 1: Speedup curve
# ─────────────────────────────────────────────

fig, ax = plt.subplots(figsize=(7, 5))
ax.plot(omp_threads, omp_speedup, 'o-', color='steelblue', linewidth=2, markersize=7, label='Speedup mesuré')
ax.plot(omp_threads, omp_threads, '--', color='gray', linewidth=1, label='Speedup idéal')
ax.set_xlabel('Nombre de processeurs', fontsize=12)
ax.set_ylabel('Speedup', fontsize=12)
ax.set_title('Accélération OpenMP (Amdahl)', fontsize=13)
ax.set_xticks(omp_threads)
ax.legend()
ax.grid(True, alpha=0.4)
# Annotate points
for t, s in zip(omp_threads, omp_speedup):
    ax.annotate(f'{s:.2f}×', (t, s), textcoords='offset points', xytext=(6, 4), fontsize=9)
plt.tight_layout()
plt.savefig('speedup.png', dpi=150)
plt.close()
print("Graph saved: speedup.png")

# ─────────────────────────────────────────────
# GRAPH 2: Efficiency curve
# ─────────────────────────────────────────────

fig, ax = plt.subplots(figsize=(7, 5))
ax.plot(omp_threads, [e*100 for e in omp_efficiency], 'o-', color='darkorange', linewidth=2, markersize=7)
ax.axhline(100, linestyle='--', color='gray', linewidth=1, label='Efficacité idéale (100 %)')
ax.set_xlabel('Nombre de processeurs', fontsize=12)
ax.set_ylabel('Efficacité (%)', fontsize=12)
ax.set_title('Efficacité OpenMP', fontsize=13)
ax.set_xticks(omp_threads)
ax.set_ylim(0, 115)
ax.legend()
ax.grid(True, alpha=0.4)
for t, e in zip(omp_threads, omp_efficiency):
    ax.annotate(f'{e*100:.1f}%', (t, e*100), textcoords='offset points', xytext=(6, -14), fontsize=9)
plt.tight_layout()
plt.savefig('efficiency.png', dpi=150)
plt.close()
print("Graph saved: efficiency.png")

# ─────────────────────────────────────────────
# GRAPH 3: Bar chart – total time comparison
# ─────────────────────────────────────────────

labels  = ['Original\n(Q1)', 'Vectorisé\n(Q2)', 'OpenMP\n1 thread', 'OpenMP\n2 threads', 'OpenMP\n4 threads', 'OpenMP\n8 threads']
totals  = [q1_total[0], q2_total[0]] + omp_total
advance = [q1_advance[0], q2_advance[0]] + omp_advance
evap    = [q1_evap[0], q2_evap[0]] + omp_evap
pher    = [q1_pher[0], q2_pher[0]] + omp_pher

x = np.arange(len(labels))
width = 0.6

fig, ax = plt.subplots(figsize=(10, 6))
bars_adv  = ax.bar(x, advance, width, label='Avancée fourmis', color='steelblue')
bars_evap = ax.bar(x, evap,    width, bottom=advance, label='Évaporation', color='tomato')
rest      = [totals[i] - advance[i] - evap[i] for i in range(len(labels))]
bars_rest = ax.bar(x, rest,    width, bottom=[advance[i]+evap[i] for i in range(len(labels))], label='Mise à jour', color='mediumseagreen')

ax.set_xlabel('Version', fontsize=12)
ax.set_ylabel('Temps (ms) – 100 itérations', fontsize=12)
ax.set_title('Comparaison des temps de simulation (100 itérations)', fontsize=13)
ax.set_xticks(x)
ax.set_xticklabels(labels, fontsize=10)
ax.legend()
ax.grid(True, axis='y', alpha=0.4)
# Annotate totals
for xi, tot in zip(x, totals):
    ax.text(xi, tot + 4, f'{tot:.0f} ms', ha='center', va='bottom', fontsize=8, fontweight='bold')
plt.tight_layout()
plt.savefig('comparison.png', dpi=150)
plt.close()
print("Graph saved: comparison.png")

# ─────────────────────────────────────────────
# GRAPH 4: Stacked time breakdown (ant advance vs evap) – OpenMP only
# ─────────────────────────────────────────────

fig, ax = plt.subplots(figsize=(7, 5))
ax.stackplot(omp_threads,
             [omp_advance],
             [omp_evap],
             labels=['Avancée fourmis', 'Évaporation'],
             colors=['steelblue', 'tomato'],
             alpha=0.85)
ax.set_xlabel('Nombre de threads', fontsize=12)
ax.set_ylabel('Temps (ms) – 100 itérations', fontsize=12)
ax.set_title('Décomposition du temps de simulation (OpenMP)', fontsize=13)
ax.set_xticks(omp_threads)
ax.legend(loc='upper right')
ax.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('breakdown_openmp.png', dpi=150)
plt.close()
print("Graph saved: breakdown_openmp.png")

print("\nDone.")

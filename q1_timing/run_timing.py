import subprocess, re, statistics

def get_times():
    proc = subprocess.run(['./ant_simu_headless.exe'], cwd='/home/diego/4OS02/4OS02/projet/q1_timing', capture_output=True, text=True)
    out = proc.stdout
    total = float(re.search(r'Total simulation time:\s*([0-9\.]+)', out).group(1))
    ant = float(re.search(r'Ant advance time:\s*([0-9\.]+)', out).group(1))
    evap = float(re.search(r'Evaporation time:\s*([0-9\.]+)', out).group(1))
    return total, ant, evap

if __name__ == '__main__':
    runs = []
    for i in range(5):
        runs.append(get_times())
    totals = [r[0] for r in runs]
    print('Totals:', totals)
    print('Mean total:', statistics.mean(totals))
    print('Stddev total:', statistics.stdev(totals))
    # print table
    print('Detailed runs (total,ant,evap):')
    for r in runs:
        print(r)

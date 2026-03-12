import subprocess, re, statistics, csv

program = './ant_simu_mpi_replica.exe'
procs_list = [1,2,4]
runs = 3
results = []
for p in procs_list:
    times = []
    for i in range(runs):
        proc = subprocess.run(['mpirun','-np',str(p),program], capture_output=True, text=True)
        out = proc.stdout
        m = re.search(r'Total sim time: *([0-9\.]+)', out)
        if m:
            times.append(float(m.group(1)))
    avg = statistics.mean(times) if times else 0
    results.append((p, avg))

with open('acceleration.csv','w',newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['procs','avg_total'])
    writer.writerows(results)

print('Results:')
for p,avg in results:
    print(p,avg)

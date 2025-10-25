# PERF
- Uses Hardware Performance Counters to get information for program execution
### Metrics
- cache-misses: Num unsuccessful cache access attempts
- cache-reference: Num access attempts
- L1-dcache-load-misses / L1-dcache-loads: misses / loads only L1 (Flush+Reload)
- LLC-load-misses / LLC-store-misses: misses in Last Level Cache (Prime+Probe)
- bus-cycles / mem-loads / mem-stores: indirect, because more transactions
### Detection of SCA
- anomalies in metrics (high num of misses)
- restrict HPC access
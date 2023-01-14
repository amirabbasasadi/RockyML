#!/usr/bin/env bash

# number of threads
n_threads=4
n_threads_per_process=4
# number of MPI processes
n_procs=1

for benchmark_dir in benchmark_* ; do
    (cd "${benchmark_dir}/build" &&
    cmake .. -DCMAKE_CXX_COMPILER=mpic++ -DCMAKE_TOOLCHAIN_FILE=../../../../../vcpkg/scripts/buildsystems/vcpkg.cmake &&
    make -j${n_threads} app &&
    mpirun --use-hwthread-cpus -np ${n_procs} --map-by node:PE=${n_threads_per_process} ./app &&
    mkdir -p ../result &&
    lscpu > ../result/cpu_info.txt &&
    for p in `seq 0 $(expr $n_procs - 1)`; do cp "proc_${p}_loss.csv" ../result/; done &&
    python3 ../../../tools/plot_logs.py proc_0_loss.csv)
done

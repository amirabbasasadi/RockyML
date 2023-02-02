#!/usr/bin/env bash

# number of threads
n_threads=4
# number of MPI processes
n_procs=1

for benchmark_dir in benchmark_* ; do
    rm -rf "${benchmark_dir}/build" && 
    mkdir "${benchmark_dir}/build" &&
    (cd "${benchmark_dir}/build" &&
    cmake .. -DCMAKE_CXX_COMPILER=mpic++ -DCMAKE_TOOLCHAIN_FILE=../../../../../vcpkg/scripts/buildsystems/vcpkg.cmake &&
    make -j${n_threads} app &&
    (for ((n_threads_per_process = 1 ; n_threads_per_process <= $n_threads ; n_threads_per_process=n_threads_per_process*2)); do
        mpirun --use-hwthread-cpus -np ${n_procs} --map-by node:PE=${n_threads_per_process} ./app ${n_threads_per_process}
    done) && 
    mkdir -p ../result &&
    lscpu > ../result/cpu_info.txt
    # for p in `seq 0 $(expr $n_procs - 1)`; do cp "proc_${p}_loss.csv" ../result/; done &&
    # python3 ../../../tools/plot_logs.py proc_0_loss.csv
    )
done

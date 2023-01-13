for benchmark_dir in benchmark_* ; do
    (cd "${benchmark_dir}/build" &&
    cmake .. -DCMAKE_CXX_COMPILER=mpic++ -DCMAKE_TOOLCHAIN_FILE=../../../../../vcpkg/scripts/buildsystems/vcpkg.cmake &&
    make -j4 app &&
    mpirun --use-hwthread-cpus -np 1 --map-by node:PE=4 ./app &&
    python3 ../../../tools/plot_logs.py proc_0_loss.csv)
done

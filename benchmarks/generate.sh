for benchmark_dir in benchmark_* ; do
    (cd "${benchmark_dir}/build" &&
    cmake .. -DCMAKE_CXX_COMPILER=mpic++ -DCMAKE_TOOLCHAIN_FILE=../../../../../vcpkg/scripts/buildsystems/vcpkg.cmake &&
    make -j4 app &&
    mpirun -np 1 ./app &&
    python3 ../../../tools/plot_logs.py objective.csv)
done

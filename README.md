# UltraFastMapper

A faithful C++17 baseline implementation of **Diagonal CGRA Scheduling** from
J. Lee and T. E. Carlson, *"Ultra-Fast CGRA Scheduling to Enable Run Time
Programmable CGRAs"*, DAC 2021 (Algorithm 1). This repository is the software
baseline for future hardware-acceleration research (Vitis HLS porting,
bottleneck profiling, and microarchitectural optimization).

## Build & Run

```bash
    cmake -S . -B build
    cmake --build build
    ./build/ultrafast_mapper

To build and run the focused loop-recurrence tests:

    cmake -S . -B build
    cmake --build build
    ctest --test-dir build --output-on-failure

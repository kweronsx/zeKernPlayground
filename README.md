# Intro

This repo is a playground for testing level zero kernel scheduling behavior.

The repo provides a CMake function addKernelTest that makes profiling examples easier.

# Building

```console
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

# Gathering profiling info

This cleans the directory with profiling data produced by ze\_tracer / unitrace,
and then runs the profiling commands.

```console
cmake --build ./build --target reKernTimelines
```

For each kernel test added via addKernelTest, there is a subdirectory created
in the main build directory in kernelTimelines, called after the name of the kernel test.
That is where all the profiling data ends up.

To only gather profiling data (no cleaning), you can run one of the following:

## Gathering profiling data using ze\_tracer only

```console
cmake --build ./build --target ztKernTimelines
```

## Gathering profiling data using unitrace only

```console
cmake --build ./build --target utKernTimelines
```

These commands will not erase prior contents, so the files will stack up over time.

## Build
```
sh ./build.sh
```

## Test
```
sh ./test.sh
```
## Benchmark
```
./build/gen_ppm -w 2000 -h 1500 -o benchmark_2000x1500.ppm
./build/gen_ppm -w 4000 -h 3000 -o benchmark_4000x3000.ppm
./build/pipeline -i benchmark_2000x1500.ppm -o output.ppm
./build/pipeline -i benchmark_4000x3000.ppm -o output.ppm
```


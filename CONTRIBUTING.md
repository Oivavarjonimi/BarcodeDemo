# Contributing

Thanks for your interest in improving BarcodeDemo!

## Development environment
- Linux (Ubuntu 22.04+ recommended) or WSL2
- C++20 compiler (GCC 11+)
- CMake 3.27+
- Qt 6 (Widgets, Multimedia, Network)

See `requirements.txt` / README for install commands.

## Build
```bash
cmake -S . -B build
cmake --build build -j
./build/BarcodeDemo

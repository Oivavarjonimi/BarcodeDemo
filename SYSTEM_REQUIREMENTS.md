# System Requirements (BarcodeDemo)

This document describes the system-level dependencies needed to build and run **BarcodeDemo** on Linux (Ubuntu) and WSL2.

## Supported Platforms

- Ubuntu 22.04+ (recommended)
- Ubuntu 24.04+
- WSL2 (Ubuntu) – supported, but **PC webcam access may be limited**

## Toolchain

- **CMake**: 3.27+
- **Compiler**: GCC / G++ 11+ (C++20)
- **Build**: Ninja or Make

Verify versions:
```bash
cmake --version
g++ --version

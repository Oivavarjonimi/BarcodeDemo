# BarcodeDemo

**BarcodeDemo** is a Qt 6–based C++ demo application for reading **QR codes and barcodes** (EAN / Code128) from images, PC webcams, and mobile phone cameras.

The application supports **runtime-selectable decoding backends**:
- **ZXing-cpp**
- **ZBar**

It is designed as a **clear, extensible demo** showing modern Qt UI, camera handling, 
and pluggable decoding logic.

---

## Features

- Read barcodes from:
  - PC webcam
  - Mobile phone camera (via DroidCam MJPEG stream)
  - Image files (PNG/JPEG)
- Live preview with manual or automatic capture
- Decode formats:
  - QR Code
  - EAN-13 / EAN-8
  - Code128
- Runtime decoder backend selection:
  - ZXing
  - ZBar
- Settings dialog:
  - Decoder backend selection
  - Phone camera IP & port configuration
- Copy decoded data to clipboard
- Save captured images and decoded results
- Debug-friendly architecture and logging

---

## Requirements

### Main dependencies
- Linux (Ubuntu 22.04+ or WSL2)
- C++20 compiler (GCC 11+)
- CMake ≥ 3.27
- Qt 6 (Widgets, Multimedia, Network)
- ZXing-cpp (included as submodule)
- ZBar (optional backend)

---

## Installation (Ubuntu / WSL2)

### 1. System Dependencies
```bash
sudo apt update && sudo apt install -y \
  cmake gcc g++ make \
  qt6-base-dev qt6-base-dev-tools \
  qt6-multimedia-dev qt6-network-dev qt6-tools-dev \
  libzbar-dev libxkbcommon-dev libgl1-mesa-dev \
  v4l-utils ffmpeg
```

### 2. DroidCam Linux Client (Optional)
To use DroidCam as a virtual webcam device:
```bash
cd /tmp
wget -O droidcam_latest.zip [https://files.dev47apps.net/linux/droidcam_2.1.3.zip](https://files.dev47apps.net/linux/droidcam_2.1.3.zip)
unzip droidcam_latest.zip -d droidcam
cd droidcam && sudo ./install-client
```

---

## Build

```bash
# Clone with submodules
git clone --recurse-submodules <repo-url>
cd BarcodeDemo

# Configure and build
cmake -S . -B build
cmake --build build -j$(nproc)
```

Run the application:
```bash
./build/BarcodeDemo
```

---

## Using a Phone Camera (DroidCam)

The application fetches the **MJPEG stream** directly from the DroidCam app.

### Setup Android
1. Install **DroidCam** from the Play Store.
2. Connect phone and PC to the **same Wi-Fi**.
3. Open DroidCam on the phone to see the **IP address** and **Port** (e.g., `192.168.1.50:4747`).

### Configure in BarcodeDemo
1. Open **Settings → Phone camera…**
2. Enter the **IP address** and **Port**.
3. Press **Snap** to start the stream.
4. Press **Snap** again to capture and decode.

---

## Project Structure

```
BarcodeDemo/
    src/
        MainWindow.* # UI and application logic
        CameraController.* # PC webcam handling
        IDecoder.h        # Decoder interface
        ZXingDecoder.* # ZXing backend
        ZBarDecoder.* # ZBar backend
    extern/
        zxing-cpp/        # ZXing-cpp submodule
    CMakeLists.txt
    README.md
```

---

## License

**MIT License**

Copyright (c) 2026 Kimmo Lepoaho 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
Tässä on **valmis, siisti ja ammattimainen `README.md`** suoraan projektiisi.
Se on kirjoitettu niin, että **arvioija / kollega / opettaja** ymmärtää heti mitä demo tekee ja miten se ajetaan.

Voit kopioida tämän sellaisenaan tiedostoon `README.md`.

---

````md
# BarcodeDemo

**BarcodeDemo** is a Qt 6–based C++ demo application for reading **QR codes and barcodes**
(EAN / Code128) from images, PC webcams, and mobile phone cameras.

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

## Screenshots

*(Optional – add screenshots here if desired)*

---

## Requirements

See `requirements.txt` for a full list.

### Main dependencies
- Linux (Ubuntu 22.04+ or WSL2)
- C++20 compiler (GCC 11+)
- CMake ? 3.27
- Qt 6:
  - Qt6 Widgets
  - Qt6 Multimedia
  - Qt6 Network
- ZXing-cpp (included as submodule)
- ZBar (optional backend)

---

## Installation (Ubuntu / WSL2)

```bash
sudo apt update && sudo apt install -y \
  cmake gcc g++ make \
  qt6-base-dev qt6-base-dev-tools \
  qt6-multimedia-dev qt6-network-dev qt6-tools-dev \
  libzbar-dev \
  libxkbcommon-dev libxkbcommon-x11-dev \
  libgl1-mesa-dev \
  v4l-utils ffmpeg
````

---

## Build

```bash
git clone <your-repo-url>
cd BarcodeDemo

cmake -S . -B build
cmake --build build -j
```

Run:

```bash
./build/BarcodeDemo
```

---

## Using a Phone Camera (DroidCam)

The application supports **mobile phone cameras** using DroidCam in MJPEG mode.

### Setup

1. Install DroidCam on your phone
2. Start DroidCam in **PC Webcam / WiFi (MJPEG)** mode
3. Note the IP address and port (e.g. `192.168.100.171:4747`)

### Configure in BarcodeDemo

* Open **Settings ? Phone camera…**
* Enter IP address and port
* Press **Snap** to start live preview
* Press **Snap** again to freeze the frame and decode

---

## Decoder Backend Selection

You can switch decoder engines at runtime:

```
Settings ? Decoder backend ? ZXing / ZBar
```

This is useful for:

* Comparing decoding accuracy
* Performance testing
* Demonstrating modular architecture

---

## Project Structure

```
BarcodeDemo/
	src/
        MainWindow.*        # UI and application logic
        CameraController.* # PC webcam handling
        IDecoder.h         # Decoder interface
        ZXingDecoder.*     # ZXing backend
        ZBarDecoder.*      # ZBar backend
        BarcodeDecoder.*  # Shared decode helpers
    extern/
        zxing-cpp/         # ZXing-cpp submodule
    build/                 # Build output
    requirements.txt
    CMakeLists.txt
    README.md
```

---

## Design Notes

* UI logic is separated from decoding logic via `IDecoder`
* Decoder backend is selected at runtime (Strategy pattern)
* Phone camera is implemented as MJPEG-over-HTTP stream
* Application settings are stored using `QSettings`
* Designed for easy extension:

  * New decoders
  * New image sources
  * Auto-decode modes

---

## Possible Extensions

* Automatic live decoding with stability detection
* Region-of-interest (ROI) selection
* Decode history filtering and export
* Plugin-based decoder loading
* Cross-platform packaging (AppImage / Windows)

---

## License

This project is intended as a **demo / educational example**.

ZXing-cpp and ZBar are licensed under their respective licenses.

```


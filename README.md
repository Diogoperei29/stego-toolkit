[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Build System](https://img.shields.io/badge/Build-CMake-orange.svg)](https://cmake.org/) [![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard) [![CI](https://img.shields.io/github/actions/workflow/status/Diogoperei29/stego-toolkit/ci.yml?branch=main)](https://github.com/Diogoperei29/stego-toolkit/actions/workflows/ci.yml)

# Steganography Toolkit

A C++ steganography toolkit for hiding and extracting data inside media files.  
This is v1.0, focused on **image-based steganography** with **AES encryption**.

---

## Features (v1.0)
- Hide and extract arbitrary data inside **BMP/PNG images**.
- **LSB (Least Significant Bit)** embedding method.
- **AES-256 encryption** with password protection (PBKDF2-HMAC-SHA256 key derivation).
- Modular design → scalable to audio, video, and advanced algorithms.

---

## Dependencies
- C++17 or later
- CMake 3.15+
- [OpenSSL](https://www.openssl.org/) (for AES encryption + PBKDF2)
- [cxxopts](https://github.com/jarro2783/cxxopts) (CLI parsing) – fetched automatically via CMake FetchContent
- [stb_image / stb_image_write](https://github.com/nothings/stb) – fetched automatically via CMake FetchContent

Notes:
- Third-party libraries are downloaded at configure time using CMake FetchContent.
- The stb headers are treated as SYSTEM includes to suppress some third‑party warnings.

---

## Build Instructions
```bash
# Clone repository
git clone https://github.com/Diogoperei29/stego-toolkit.git
cd stego-toolkit

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .
```

---

## Run Instructions:
```bash
# Embed
./stegtool --command embed -i cover.png -d secret.txt -o stego.png -p "mypassword"

# Extract
./stegtool --command extract -i stego.png -o recovered.txt -p "mypassword"

# Help
./stegtool --help
```

---

## Project Structure

```
/stegtool
├── src/
│   ├── main.cpp                 # Entry point
│   ├── CLI.h/.cpp               # CLI parsing (cxxopts)
│   ├── StegoHandler.h           # Abstract base class
│   ├── ImageStegoHandler.h/.cpp # LSB embed/extract
│   ├── CryptoModule.h/.cpp      # AES-256-CBC + PBKDF2-HMAC-SHA256
│   ├── Utils.h/.cpp             # stb-based image load/save
├── CMakeLists.txt
├── NOTICE                       # Third-party notices
├── LICENSE
└── README.md
```

---

## How It Works (High Level)
- Encrypt data with AES-256-CBC; store payload as [salt | iv | ciphertext]
- Embed encrypted payload into image pixel LSBs
- Extract payload from LSBs and decrypt with the provided password

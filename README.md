# Steganography Toolkit

A C++ steganography toolkit for hiding and extracting data inside media files.  
This is v1.0, focused on **image-based steganography** with **AES encryption**.

---

## Features (v1.0)
- Hide and extract arbitrary data inside **BMP/PNG images**.
- **LSB (Least Significant Bit)** embedding method.
- **AES-256 encryption** with password protection.
- Modular design → scalable to audio, video, and advanced algorithms.

---

## Dependencies
- C++17 or later
- CMake 3.15+
- [OpenSSL](https://www.openssl.org/) (for AES encryption + PBKDF2)
- [stb_image / stb_image_write](https://github.com/nothings/stb) (header-only image library, included in `src/`)
---

## Build Instructions
```bash
# Clone repository
git clone https://github.com/Diogoperei29/stego-toolkit.git
cd Steganography-Toolkit

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make
```

## Project Structure

```
/stegtool
├── src/
│   ├── main.cpp                   # CLI entry point
│   ├── StegoHandler.h             # Abstract base class
│   ├── ImageStegoHandler.h/.cpp   # Image LSB implementation
│   ├── CryptoModule.h/.cpp        # AES encryption/decryption
│   ├── Utils.h/.cpp               # Helper functions
├── CMakeLists.txt
├── NOTICE
├── LICENSE
└── README.md
```

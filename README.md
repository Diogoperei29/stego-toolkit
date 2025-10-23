[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Build System](https://img.shields.io/badge/Build-CMake-orange.svg)](https://cmake.org/) [![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard) [![CI](https://img.shields.io/github/actions/workflow/status/Diogoperei29/stego-toolkit/ci.yml?branch=main)](https://github.com/Diogoperei29/stego-toolkit/actions/workflows/ci.yml)

# Steganography Toolkit

A modern C++ toolkit for hiding and extracting data within images using steganography techniques combined with strong encryption.

## Features

- **Image Steganography** - Hide data inside PNG/BMP/JPEG images
- **Strong Encryption** - AES-256-CBC with PBKDF2-HMAC-SHA256 key derivation (10,000 iterations)
- **Standard Compliance** - OpenSSL-compatible encryption format
- **Modular Architecture** - Extensible design supporting multiple steganography algorithms (planned)
- **Cross-Platform** - Windows, Linux
- **External Dependencies** - Libraries fetched automatically via CMake (excluding OpenSSL)

---

## Quick Start

### Prerequisites

**Linux:**
```bash
sudo apt install build-essential cmake libssl-dev
```

**Windows:**
- Install [Visual Studio 2017+](https://visualstudio.microsoft.com/) or [MinGW-w64](https://www.mingw-w64.org/)
- Install [CMake](https://cmake.org/download/)
- Install [OpenSSL](https://slproweb.com/products/Win32OpenSSL.html)

### Build

```bash
# Clone repository
git clone https://github.com/Diogoperei29/stego-toolkit.git
cd stego-toolkit

# Configure and build
mkdir build && cd build
cmake ..
cmake --build .
```

> [!NOTE]  
> To use this anywhere, the build folder to your PATH (might automate this later somehow).
> - Windows: add the absolute path to build to your user PATH, then restart your terminal.
> - Linux: add export PATH="/absolute/path/to/project/build:$PATH" to your shell profile (e.g., ~/.bashrc), then reload it.

### Usage

**Embed data into an image:**
```bash
stegtool embed -i cover.png -d secret.txt -o stego.png -p mypassword
```

**Extract hidden data:**
```bash
stegtool extract -i stego.png -o recovered.txt -p mypassword
```

**Get help:**
```bash
stegtool --help
stegtool -h
stegtool --version
```

---

## How It Works

### Encryption Layer
1. User provides a password and data file
2. Random salt (16 bytes) is generated
3. Key derived from password + salt using PBKDF2-HMAC-SHA256 (10,000 iterations)
4. Random IV (16 bytes) is generated
5. Data encrypted with AES-256-CBC
6. Output format: `[salt | IV | ciphertext]`

### Embedding Layer
The encrypted payload is embedded into the image using the selected algorithm. The algorithm modifies pixel values in a way that is imperceptible to the human eye while storing the data securely.

### Extraction Layer
1. Load stego image and extract embedded data
2. Decrypt using password (derives same key from stored salt)
3. Save recovered plaintext

**Security Model:**
- **Password is the only secret** - Without it, data cannot be decrypted
- **Salt prevents rainbow tables** - Each encryption uses unique random salt
- **IV prevents pattern analysis** - Identical plaintexts encrypt differently
- **Standard format** - Compatible with OpenSSL and other standard tools

---

## Project Structure

```
stegtool/
├── src/
│   ├── main.cpp
│   ├── core/                         # Application logic
│   │   └── CLI.h/.cpp                # Command-line interface
│   ├── utils/                        # Utility modules
│   │   ├── ErrorHandler.h/.cpp       # Result<T> error handling system
│   │   ├── CryptoModule.h/.cpp       # AES-256-CBC encryption
│   │   └── ImageIO.h/.cpp            # Image loading/saving (stb library)
│   └── algorithms/                   # Steganography algorithms
│       ├── StegoHandler.h            # Abstract base class
│       └── lsb/                      # LSB implementation
│           ├── LSBStegoHandler.h
│           └── LSBStegoHandler.cpp
├── tests/
│   └── test_all.cpp                  # Unit tests (Google Test)
├── CMakeLists.txt                    # Build configuration
├── LICENSE                           # Apache 2.0 license
├── THIRD-PARTY                       # Third-party attribution
└── README.md
```


## Dependencies

The project uses the following libraries, automatically fetched via CMake FetchContent:

| Library | Purpose | License |
|---------|---------|---------|
| [OpenSSL](https://www.openssl.org/) | AES-256-CBC encryption, PBKDF2 | Apache 2.0 |
| [cxxopts](https://github.com/jarro2783/cxxopts) | Command-line parsing | MIT |
| [stb](https://github.com/nothings/stb) | Image loading/saving | MIT/Public Domain |
| [Google Test](https://github.com/google/googletest) | Unit testing framework | BSD-3-Clause |

**System Requirements:**
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16 or later
- OpenSSL development libraries (system-installed)

---

## CLI Reference

### Commands

**`embed`** - Hide data inside an image
```bash
stegtool embed -i <cover_image> -d <data_file> -o <output_image> -p <password>

Options:
  -i, --input     Input cover image (PNG/BMP/JPEG)
  -d, --data      Data file to hide
  -o, --output    Output stego image
  -p, --password  Password for encryption
```

**`extract`** - Extract hidden data from an image
```bash
stegtool extract -i <stego_image> -o <output_file> -p <password>

Options:
  -i, --input     Input stego image
  -o, --output    Output file for extracted data
  -p, --password  Password for decryption
```
> [!WARNING]  
> If you ommit output file or use the same name, the program will ask to overwrite the file and wait for additional user input.

### Global Options
- `-h, --help` - Display help message
- `-v, --version` - Display version information

---

## License

This project is licensed under the Apache License 2.0 - see [LICENSE](LICENSE) file.

Third-party library licenses are documented in [THIRD-PARTY](THIRD-PARTY).


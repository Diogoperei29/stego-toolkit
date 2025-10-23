# Test Fixtures

This directory contains pre-generated test data for steganography testing.

## Image Files

### Grayscale Images (Optimal for LSB Testing)
- **`tiny_gray.png`** (32×32) - ~383 bytes capacity
- **`small_gray.png`** (100×100) - ~3.7 KB capacity
- **`medium_gray.png`** (512×512) - ~96 KB capacity
- **`large_gray.png`** (1024×768) - ~289 KB capacity
- **`huge_gray.png`** (2048×2048) - ~1.5 MB capacity

### Format Variations
- **`medium_gray.bmp`** - BMP format test
- **`medium_gray.jpg`** - JPEG format test (lossy - not recommended for LSB)

### Special Patterns
- **`gradient_gray.png`** - Gradient pattern (good for visual artifact detection)
- **`checkerboard.png`** - Alternating pattern (edge case testing)

### Color Images
- **`small_rgb.png`** (100×100, RGB) - ~11.2 KB capacity
- **`medium_rgb.png`** (512×512, RGB) - ~288 KB capacity
- **`rgba_test.png`** (256×256, RGBA) - 4 channels

## Text & Data Files

- **`small.txt`** - Small text file (~100 bytes)
- **`medium.txt`** - Medium text file (~800 bytes)
- **`large.txt`** - Large text file (~14 KB)
- **`huge_1mb.txt`** - 1 MB file (tests capacity overflow)
- **`unicode.txt`** - UTF-8 with emoji and special characters
- **`empty.txt`** - Empty file (edge case)
- **`single_byte.bin`** - Single byte file (minimal data)
- **`binary_data.bin`** - Binary data (1024 bytes, all byte values 0-255)

## Regenerating Fixtures

Run the Python script to regenerate image fixtures:

```bash
cd tests/fixtures
python generate_fixtures.py
```

## Why Gray-128?

Most images use **gray value 128** because:
- Binary: `10000000` → LSB flip gives `10000001` (129)
- Barely visible change (1/255 brightness)
- Easy to verify: `pixel & 1` extracts the embedded bit
- Mid-tone allows testing both 0→1 and 1→0 flips

## Capacity Formula

For LSB steganography:
```
Capacity (bytes) = (width × height × channels - 32) / 8
```

The 32 bits are reserved for the size header.
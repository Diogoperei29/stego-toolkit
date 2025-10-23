import numpy as np
from PIL import Image
import os

# ensure script executes where file is
script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)

# Create grayscale gradient image.
def create_grayscale_gradient(width, height, filename):
    img = np.zeros((height, width), dtype=np.uint8)
    for y in range(height):
        img[y, :] = int((y / height) * 255)
    Image.fromarray(img, mode='L').save(filename)
    print(f"Created {filename}")

# Create medium-gray (128) image for LSB testing.
def create_gray_128(width, height, filename, format='PNG'):
    img = np.full((height, width), 128, dtype=np.uint8)
    Image.fromarray(img, mode='L').save(filename, format=format)
    print(f"Created {filename}")

# Create checkerboard pattern.
def create_alternating_pattern(width, height, filename):
    img = np.zeros((height, width), dtype=np.uint8)
    for y in range(height):
        for x in range(width):
            img[y, x] = 255 if (x + y) % 2 == 0 else 0
    Image.fromarray(img, mode='L').save(filename)
    print(f"Created {filename}")

# Create RGB image.
def create_rgb_image(width, height, filename, color=(128, 128, 128)):
    img = np.zeros((height, width, 3), dtype=np.uint8)
    img[:, :] = color
    Image.fromarray(img, mode='RGB').save(filename)
    print(f"Created {filename}")

# Create RGBA image.
def create_rgba_image(width, height, filename):
    img = np.zeros((height, width, 4), dtype=np.uint8)
    img[:, :] = [128, 128, 128, 255]
    Image.fromarray(img, mode='RGBA').save(filename)
    print(f"Created {filename}")

if __name__ == "__main__":
    print("Generating test fixtures...")
    
    create_gray_128(32, 32, "tiny_gray.png")
    create_gray_128(100, 100, "small_gray.png")
    create_gray_128(512, 512, "medium_gray.png", 'PNG')
    create_gray_128(512, 512, "medium_gray.bmp", 'BMP')
    create_gray_128(512, 512, "medium_gray.jpg", 'JPEG')
    create_gray_128(1024, 768, "large_gray.png")
    create_gray_128(2048, 2048, "huge_gray.png")
    
    create_grayscale_gradient(512, 512, "gradient_gray.png")
    create_alternating_pattern(256, 256, "checkerboard.png")
    
    create_rgb_image(512, 512, "medium_rgb.png")
    create_rgb_image(100, 100, "small_rgb.png", color=(64, 128, 192))
    create_rgba_image(256, 256, "rgba_test.png")
    
    print("Done")

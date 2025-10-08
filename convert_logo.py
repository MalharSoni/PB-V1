#!/usr/bin/env python3
"""
CTRC Logo Converter for LVGL 5.3 (VEX V5 Brain)
Converts logo_100 x 100.png to RGB565 C array format
"""

from PIL import Image
import os

# Input and output paths
input_file = '/Users/malharsoni/Downloads/logo_100 x 100.png'
output_file = '/Users/malharsoni/Downloads/HalloweenQualCode/logo_data.txt'

print("=== CTRC Logo Converter ===")
print(f"Input: {input_file}")
print(f"Output: {output_file}")

# Load image
try:
    img = Image.open(input_file)
    print(f"✓ Loaded image: {img.size[0]}x{img.size[1]} pixels")
except Exception as e:
    print(f"✗ Error loading image: {e}")
    exit(1)

# Convert to RGB (handle transparency)
if img.mode == 'RGBA':
    # Create white background for transparency
    background = Image.new('RGB', img.size, (255, 255, 255))
    background.paste(img, mask=img.split()[3])  # Use alpha channel as mask
    img = background
    print("✓ Converted RGBA to RGB (white background)")
elif img.mode != 'RGB':
    img = img.convert('RGB')
    print(f"✓ Converted {img.mode} to RGB")

width, height = img.size

# Verify size
if width != 100 or height != 100:
    print(f"⚠ Warning: Expected 100x100, got {width}x{height}")

# Convert to RGB565 format
print("Converting to RGB565...")
rgb565_data = []
for y in range(height):
    for x in range(width):
        r, g, b = img.getpixel((x, y))

        # Convert 8-bit RGB (0-255) to 5-6-5 bit format
        r5 = (r >> 3) & 0x1F  # 5 bits for red
        g6 = (g >> 2) & 0x3F  # 6 bits for green
        b5 = (b >> 3) & 0x1F  # 5 bits for blue

        # Combine into 16-bit RGB565 value
        rgb565 = (r5 << 11) | (g6 << 5) | b5

        # Store as little-endian bytes (low byte first, then high byte)
        rgb565_data.append(rgb565 & 0xFF)
        rgb565_data.append((rgb565 >> 8) & 0xFF)

total_bytes = len(rgb565_data)
print(f"✓ Converted {width}x{height} = {width*height} pixels")
print(f"✓ Generated {total_bytes} bytes ({total_bytes/1024:.1f} KB)")

# Write C array to file
print(f"Writing to {output_file}...")
with open(output_file, 'w') as f:
    f.write("// CTRC Logo - 100x100 pixels - RGB565 format\n")
    f.write(f"// Generated from: {os.path.basename(input_file)}\n")
    f.write(f"// Size: {total_bytes} bytes ({total_bytes/1024:.1f} KB)\n\n")

    f.write("static const uint8_t logo_ctrc_100x100_map[] = {\n")

    # Write bytes in rows of 12 for readability
    for i in range(0, len(rgb565_data), 12):
        row = rgb565_data[i:i+12]
        hex_values = ', '.join(f'0x{b:02x}' for b in row)
        f.write(f'    {hex_values},\n')

    f.write('};\n\n')

    # Write LVGL 5.3 image descriptor
    f.write("// LVGL 5.3 Image Descriptor\n")
    f.write("const lv_img_dsc_t logo_ctrc_100x100 = {\n")
    f.write("    .header = {\n")
    f.write("        .cf = LV_IMG_CF_TRUE_COLOR,\n")
    f.write("        .always_zero = 0,\n")
    f.write("        .reserved = 0,\n")
    f.write(f"        .w = {width},\n")
    f.write(f"        .h = {height},\n")
    f.write("    },\n")
    f.write("    .data_size = sizeof(logo_ctrc_100x100_map),\n")
    f.write("    .data = logo_ctrc_100x100_map,\n")
    f.write("};\n")

print(f"✓ Successfully wrote C array to {output_file}")
print("\n=== Next Steps ===")
print("1. Copy the contents of logo_data.txt")
print("2. Replace the contents of include/logo.h")
print("3. Build and upload to robot")

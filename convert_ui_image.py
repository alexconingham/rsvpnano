#!/usr/bin/env python3
import struct
from PIL import Image
import numpy as np

# Target colors (hex to RGB)
TARGET_COLORS = {
    'background': (0xdf, 0xf6, 0xf5),  # dff6f5 - light cyan
    'dark': (0x30, 0x2c, 0x2e),        # 302c2e - dark gray
    'accent': (0xa0, 0x93, 0x8e),      # a0938e - medium gray
}

def rgb_to_hsv(r, g, b):
    """Convert RGB to HSV for better color matching"""
    r, g, b = r / 255.0, g / 255.0, b / 255.0
    mx = max(r, g, b)
    mn = min(r, g, b)
    df = mx - mn
    
    if mx == mn:
        h = 0
    elif mx == r:
        h = (60 * ((g - b) / df) + 360) % 360
    elif mx == g:
        h = (60 * ((b - r) / df) + 120) % 360
    else:
        h = (60 * ((r - g) / df) + 240) % 360
    
    s = 0 if mx == 0 else (df / mx)
    v = mx
    return h, s, v

def find_closest_color(rgb):
    """Find closest target color based on brightness/value"""
    r, g, b = rgb
    
    # Convert to grayscale to determine brightness
    brightness = (r * 0.299 + g * 0.587 + b * 0.114) / 255.0
    
    if brightness > 0.8:
        return TARGET_COLORS['background']
    elif brightness < 0.3:
        return TARGET_COLORS['dark']
    else:
        return TARGET_COLORS['accent']

def rgb565_to_bytes(r, g, b):
    """Convert RGB888 to RGB565 little-endian bytes"""
    # Convert to 5-6-5 format
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    
    # Pack into 16-bit RGB565
    rgb565 = (r5 << 11) | (g6 << 5) | b5
    
    # Return as little-endian bytes
    return bytes([rgb565 & 0xFF, (rgb565 >> 8) & 0xFF])

# Load image
img = Image.open('ui_blank.png')
img = img.convert('RGB')
width, height = img.size

print(f"Image size: {width}x{height}")

# Convert pixels
pixels = img.load()
rgb565_bytes = bytearray()

for y in range(height):
    for x in range(width):
        r, g, b = pixels[x, y]
        target_rgb = find_closest_color((r, g, b))
        rgb565_bytes.extend(rgb565_to_bytes(*target_rgb))

# Generate C++ header
header = f'''#pragma once

#include <Arduino.h>

// Generated from ui_blank.png: {width}x{height} RGB565
constexpr size_t UI_BLANK_SIZE = {len(rgb565_bytes)};

constexpr uint8_t ui_blank_rgb565[] = {{
'''

# Add bytes in rows of 16 (8 pixels per row)
for i in range(0, len(rgb565_bytes), 16):
    chunk = rgb565_bytes[i:i+16]
    hex_str = ','.join(f'0x{b:02x}' for b in chunk)
    header += f'    {hex_str},\n'

header += '''};
'''

# Write header file
with open('../src/display/ui_blank.h', 'w') as f:
    f.write(header)

print(f"Generated ui_blank.h with {len(rgb565_bytes)} bytes")
print(f"Colors used:")
print(f"  Background (light): {TARGET_COLORS['background']}")
print(f"  Dark elements: {TARGET_COLORS['dark']}")
print(f"  Accents: {TARGET_COLORS['accent']}")

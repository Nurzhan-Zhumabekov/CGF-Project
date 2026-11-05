#!/usr/bin/env python3
"""Generate our radial RGBA ring profile with Python's standard library only.

Usage: python tools/gen_ring_texture.py
The committed PNG is used at runtime; Python is not needed to build or run the app.
"""
import math
from pathlib import Path
import struct
import zlib


def smoothstep(a, b, x):
    t = max(0.0, min(1.0, (x - a) / (b - a)))
    return t * t * (3.0 - 2.0 * t)


def chunk(kind, data):
    return struct.pack('!I', len(data)) + kind + data + struct.pack('!I', zlib.crc32(kind + data))


def main():
    width, height = 1024, 8
    row = bytearray()
    for i in range(width):
        u = i / (width - 1)
        bands = 0.5 + 0.25 * math.sin(u * 190) + 0.15 * math.sin(u * 470) + 0.1 * math.sin(u * 1070)
        density = 0.3 if u < 0.22 else 0.85 if u < 0.66 else 0.65
        # A transparent Cassini-like gap and soft inner/outer edges.
        gap = smoothstep(0.645, 0.66, u) * (1.0 - smoothstep(0.70, 0.715, u))
        edge = smoothstep(0.0, 0.025, u) * (1.0 - smoothstep(0.97, 1.0, u))
        alpha = density * (0.65 + 0.35 * bands) * (1.0 - gap) * edge
        brightness = 0.7 + 0.3 * bands
        row.extend(int(c * brightness) for c in (230, 210, 172))
        row.append(round(255 * alpha))
    pixels = (b'\x00' + bytes(row)) * height
    png = b'\x89PNG\r\n\x1a\n'
    png += chunk(b'IHDR', struct.pack('!2I5B', width, height, 8, 6, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(pixels, 9)) + chunk(b'IEND', b'')
    path = Path(__file__).resolve().parents[1] / 'assets' / 'textures' / 'saturn_rings.png'
    path.write_bytes(png)
    print('wrote', path)


if __name__ == '__main__':
    main()

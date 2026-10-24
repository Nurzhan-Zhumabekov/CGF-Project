#!/usr/bin/env python3
"""Generates the placeholder planet textures and star skybox (own procedural art, no third-party assets).

Usage: python3 tools/gen_textures.py   (needs numpy and Pillow)
Real photographic textures can replace any of these files; keep the same file names.
"""
import os
import numpy as np
from PIL import Image

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "textures")
W, H = 1024, 512


def hash3(ix, iy, iz, seed):
    h = (ix * 73856093) ^ (iy * 19349663) ^ (iz * 83492791) ^ (seed * 2654435761)
    h &= 0xFFFFFFFF
    h = ((h ^ (h >> 13)) * 1274126177) & 0xFFFFFFFF
    return ((h ^ (h >> 16)) & 0xFFFF) / 65535.0


def noise3(x, y, z, seed=0):
    xi, yi, zi = np.floor(x).astype(np.int64), np.floor(y).astype(np.int64), np.floor(z).astype(np.int64)
    fx, fy, fz = x - xi, y - yi, z - zi
    fx, fy, fz = (f * f * (3 - 2 * f) for f in (fx, fy, fz))
    out = 0
    for dx in (0, 1):
        for dy in (0, 1):
            for dz in (0, 1):
                w = (fx if dx else 1 - fx) * (fy if dy else 1 - fy) * (fz if dz else 1 - fz)
                out = out + w * hash3(xi + dx, yi + dy, zi + dz, seed)
    return out


def fbm(p, freq, octaves=5, seed=0):
    x, y, z = p
    total, amp, norm = 0, 1.0, 0.0
    for o in range(octaves):
        total = total + amp * noise3(x * freq, y * freq, z * freq, seed + o)
        norm += amp
        freq *= 2.0
        amp *= 0.5
    return total / norm


def sphere_dirs():
    lon = (np.arange(W) + 0.5) / W * 2 * np.pi
    lat = np.pi / 2 - (np.arange(H) + 0.5) / H * np.pi
    lon, lat = np.meshgrid(lon, lat)
    return (np.cos(lat) * np.cos(lon), np.sin(lat), -np.cos(lat) * np.sin(lon)), lat


def lerp(a, b, t):
    a, b = np.array(a, dtype=np.float32), np.array(b, dtype=np.float32)
    return a + (b - a) * t[..., None]


def save(name, rgb):
    img = (np.clip(rgb, 0, 1) * 255).astype(np.uint8)
    Image.fromarray(img).save(os.path.join(OUT, name + ".png"), optimize=True)
    print("wrote", name)


def main():
    os.makedirs(OUT, exist_ok=True)
    p, lat = sphere_dirs()
    n1 = fbm(p, 3.0, seed=1)
    n2 = fbm(p, 9.0, 4, seed=7)
    ridge = 1 - np.abs(2 * fbm(p, 6.0, 4, seed=3) - 1)

    save("sun", lerp((0.85, 0.28, 0.04), (1.0, 0.92, 0.45), np.clip((fbm(p, 5.0, 5, 11) - 0.3) * 2.2, 0, 1)))
    save("mercury", lerp((0.28, 0.27, 0.26), (0.66, 0.63, 0.6), 0.6 * n1 + 0.4 * ridge ** 3))
    warp = fbm(p, 2.0, 3, 5)
    save("venus", lerp((0.72, 0.5, 0.2), (0.95, 0.85, 0.55), np.clip(0.5 + 0.5 * np.sin(lat * 9 + warp * 6) * 0.6 + n2 * 0.2, 0, 1)))

    land = fbm(p, 2.2, 5, 21)
    is_land = land > 0.53
    ocean = lerp((0.02, 0.1, 0.35), (0.06, 0.3, 0.55), np.clip((land - 0.3) * 3, 0, 1))
    green = lerp((0.13, 0.38, 0.12), (0.45, 0.4, 0.2), np.clip((n2 - 0.35) * 2.5, 0, 1))
    desert = lerp((0.7, 0.58, 0.35), green, np.clip(1.2 - np.abs(lat) * 2.2, 0, 1))
    earth = np.where(is_land[..., None], desert, ocean)
    ice = np.clip((np.abs(lat) - 1.2) * 8, 0, 1)
    save("earth", earth * (1 - ice[..., None]) + ice[..., None] * np.array([0.93, 0.95, 1.0], np.float32))
    lights = (is_land & (fbm(p, 24.0, 3, 33) > 0.58) & (np.abs(lat) < 1.1)).astype(np.float32)
    save("earth_night", lights[..., None] * np.array([1.0, 0.8, 0.4], np.float32) * (0.6 + 0.4 * n2[..., None]))

    save("mars", lerp((0.45, 0.17, 0.08), (0.8, 0.42, 0.22), 0.3 * n2 + 0.7 * n1) * (1 - 0.7 * np.clip((np.abs(lat) - 1.35) * 20, 0, 1))[..., None]
         + 0.7 * np.clip((np.abs(lat) - 1.35) * 20, 0, 1)[..., None])
    band = 0.5 + 0.5 * np.sin(lat * 14 + fbm(p, 4.0, 4, 41) * 5)
    jup = lerp((0.55, 0.38, 0.25), (0.93, 0.85, 0.72), band)
    spot = np.exp(-(((lat + 0.35) / 0.1) ** 2 + ((((np.arctan2(-p[2], p[0]) - 2.0) + np.pi) % (2 * np.pi) - np.pi) / 0.22) ** 2))
    save("jupiter", jup * (1 - spot[..., None]) + spot[..., None] * np.array([0.75, 0.3, 0.18], np.float32))
    save("saturn", lerp((0.7, 0.6, 0.38), (0.93, 0.85, 0.62), 0.5 + 0.5 * np.sin(lat * 18 + n1 * 2) * 0.7))
    save("uranus", lerp((0.45, 0.75, 0.8), (0.65, 0.88, 0.9), 0.5 + 0.3 * np.sin(lat * 6) + 0.2 * n1))
    save("neptune", lerp((0.1, 0.2, 0.65), (0.3, 0.5, 0.95), np.clip(0.5 + 0.35 * np.sin(lat * 10 + n1 * 3) + 0.2 * n2, 0, 1)))
    save("moon", lerp((0.3, 0.3, 0.3), (0.72, 0.72, 0.7), 0.55 * n1 + 0.45 * ridge ** 4))

    S = 512
    rng = np.random.default_rng(2024)
    s = np.linspace(-1, 1, S)
    sg, tg = np.meshgrid(s, s)
    faces = {
        "px": (np.ones_like(sg), -tg, -sg), "nx": (-np.ones_like(sg), -tg, sg),
        "py": (sg, np.ones_like(sg), tg), "ny": (sg, -np.ones_like(sg), -tg),
        "pz": (sg, -tg, np.ones_like(sg)), "nz": (-sg, -tg, -np.ones_like(sg))}
    for name, (x, y, z) in faces.items():
        r = np.sqrt(x * x + y * y + z * z)
        d = (x / r, y / r, z / r)
        neb = np.clip(fbm(d, 1.6, 5, 55) - 0.45, 0, 1)
        img = np.zeros((S, S, 3), np.float32)
        img += neb[..., None] * np.array([0.25, 0.1, 0.4], np.float32) * 1.6
        for _ in range(900):
            px_, py_ = rng.integers(1, S - 1, 2)
            b = rng.random() ** 3
            col = np.array([1.0, 0.95, 0.85]) * (0.35 + 0.65 * b) if rng.random() < 0.7 else np.array([0.7, 0.8, 1.0]) * (0.35 + 0.65 * b)
            img[py_, px_] = np.maximum(img[py_, px_], col)
            if b > 0.5:
                img[py_ - 1:py_ + 2, px_] = np.maximum(img[py_ - 1:py_ + 2, px_], col * 0.4)
                img[py_, px_ - 1:px_ + 2] = np.maximum(img[py_, px_ - 1:px_ + 2], col * 0.4)
        save("sky_" + name, img)


if __name__ == "__main__":
    main()

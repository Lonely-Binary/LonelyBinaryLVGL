#!/usr/bin/env python3
"""Contact sheet: every PNG of an example side by side, on a grey ground.
Reads only what render.cpp writes (8-bit RGB, filter 0), so no PIL needed.

    tools/render/sheet.py 20_Button        -> shots/20_Button.png
"""
import pathlib, struct, sys, zlib

HERE = pathlib.Path(__file__).resolve().parent


def read(p):
    b = p.read_bytes()
    i, idat, w, h = 8, b"", 0, 0
    while i < len(b):
        n, t = struct.unpack(">I4s", b[i:i + 8])
        d = b[i + 8:i + 8 + n]
        if t == b"IHDR":
            w, h = struct.unpack(">II", d[:8])
        elif t == b"IDAT":
            idat += d
        i += 12 + n
    raw = zlib.decompress(idat)
    return w, h, [raw[y * (w * 3 + 1) + 1:(y + 1) * (w * 3 + 1)] for y in range(h)]


def write(p, w, h, rows):
    def chunk(t, d):
        return struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d))
    raw = b"".join(b"\0" + bytes(r) for r in rows)
    p.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
                  + chunk(b"IDAT", zlib.compress(raw, 6)) + chunk(b"IEND", b""))


def main():
    for name in sys.argv[1:]:
        imgs = [read(p) for p in sorted((HERE / "shots" / name).glob("*.png"))]
        # One row per shot name order, but simply flow left to right, wrap at 1400 px.
        gap, maxw = 12, 1400
        x = y = gap
        rowh = 0
        place = []
        for w, h, px in imgs:
            if x + w + gap > maxw and x > gap:
                x, y, rowh = gap, y + rowh + gap, 0
            place.append((x, y, w, h, px))
            x += w + gap
            rowh = max(rowh, h)
        W = max(px_x + w for px_x, _, w, _, _ in place) + gap
        H = y + rowh + gap
        canvas = [bytearray(b"\x50\x50\x50" * W) for _ in range(H)]
        for x0, y0, w, h, px in place:
            for r in range(h):
                canvas[y0 + r][x0 * 3:(x0 + w) * 3] = px[r]
        out = HERE / "shots" / f"{name}.png"
        write(out, W, H, canvas)
        print(out)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
import sys, re, binascii
from PIL import Image
from io import BytesIO

# -------- settings --------
W, H = 128, 64
TAG = "sh1106_dump"   # the logger tag you used
# --------------------------

need_bytes = W * H // 8
need_hex = need_bytes * 2

text = sys.stdin.read()

# 1) Prefer lines from our tag; keep only their trailing hex blob
hex_parts = []
for line in text.splitlines():
    if TAG in line:
        m = re.search(r']: ([0-9A-Fa-f]+)\s*$', line)
        if m:
            s = m.group(1).strip()
            # sanity: even length and at least 2 hex chars
            if len(s) >= 2 and len(s) % 2 == 0:
                hex_parts.append(s)

hex_str = "".join(hex_parts)

# 2) Fallback: if that didn't give a full frame, brute-force all hex pairs in input
if len(hex_str) < need_hex:
    pairs = re.findall(r'[0-9A-Fa-f]{2}', text)
    hex_str = "".join(pairs)

# 3) If we got more than one frame (e.g., multiple dumps), take the last full frame
if len(hex_str) >= need_hex:
    hex_str = hex_str[-need_hex:]
else:
    raise SystemExit(f"Only {len(hex_str)//2} bytes found, need {need_bytes}.")

# 4) Convert to image (SSD1306/SH1106 page order)
buf = bytearray(binascii.unhexlify(hex_str))
if len(buf) != need_bytes:
    raise SystemExit(f"Byte count mismatch: {len(buf)} != {need_bytes}.")

img = Image.new("1", (W, H), 0)
px = img.load()
for y in range(H):
    row = (y // 8) * W
    bit = 1 << (y & 7)
    for x in range(W):
        px[x, y] = 255 if (buf[row + x] & bit) else 0

img.save("oled.png")

bio = BytesIO()
img.save(bio, format="png")
sys.stdout.buffer.write(bio.getvalue())

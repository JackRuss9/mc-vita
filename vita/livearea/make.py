#!/usr/bin/env python3
"""Your own pictures for the Vita's home screen (LiveArea).

    python3 vita/livearea/make.py ICON [BACKGROUND [START]]

ICON becomes the bubble (128x128), BACKGROUND the LiveArea's background
(840x500) and START the picture on its Start button (280x158; BACKGROUND
again when left out). Any picture format works; each is cropped to fill its
size and saved as the 8-bit (paletted) PNG the Vita needs, in livearea/ at
the top of the project, which ./build.sh then uses instead of the pictures
in vita/livearea/. Delete livearea/ to go back to those.

Needs Pillow (Ubuntu/WSL: sudo apt install python3-pil).
"""
import os
import sys

try:
    from PIL import Image, ImageOps
except ImportError:
    sys.exit("Pillow is missing: sudo apt install python3-pil")


def fit(path, size, out):
    im = Image.open(path).convert("RGB")
    im = ImageOps.fit(im, size, Image.LANCZOS)
    im.convert("P", palette=Image.ADAPTIVE, colors=256).save(out, optimize=True)
    print(f"{out}: {size[0]}x{size[1]} from {path}")


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    dest = os.path.join(root, "livearea")
    os.makedirs(dest, exist_ok=True)
    here = os.path.dirname(os.path.abspath(__file__))
    icon = sys.argv[1]
    bg = sys.argv[2] if len(sys.argv) > 2 else None
    start = sys.argv[3] if len(sys.argv) > 3 else bg
    fit(icon, (128, 128), os.path.join(dest, "icon0.png"))
    if bg:
        fit(bg, (840, 500), os.path.join(dest, "bg.png"))
        fit(start, (280, 158), os.path.join(dest, "startup.png"))
    # the pictures not given come from vita/livearea/
    for name in ("bg.png", "startup.png", "template.xml"):
        p = os.path.join(dest, name)
        if not os.path.exists(p):
            with open(os.path.join(here, name), "rb") as f, open(p, "wb") as o:
                o.write(f.read())
    print("Now run ./build.sh")


main()

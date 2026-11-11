#!/usr/bin/env python3
"""Your own pictures for the Vita's home screen (LiveArea), for your own builds.

    python3 vita/livearea/make.py ICON [BACKGROUND [START]] [--bg-x X] [--start-x X]

ICON becomes the bubble (128x128), BACKGROUND the LiveArea's background
(840x500) and START the picture on its Start button (280x158; BACKGROUND
again when left out). Any picture format works. The background and Start
pictures are cropped to fill their size; --bg-x / --start-x pick the part of
a wide picture that is kept (0 = left edge, 1 = right edge, default 0.5). An
icon that isn't about square is fitted inside the bubble on its own
background colour (the colour of its top-left corner) instead of cropped.

The results are the 8-bit (paletted) PNGs the Vita needs, written to
livearea/ at the top of the project. `./build.sh --my-art` uses them; a plain
`./build.sh` uses the placeholders in vita/livearea/, so a VPK you publish
never contains your pictures. livearea/ is in .gitignore: it is never
committed.

Needs Pillow (Ubuntu/WSL: sudo apt install python3-pil).
"""
import os
import sys

try:
    from PIL import Image, ImageOps
except ImportError:
    sys.exit("Pillow is missing: sudo apt install python3-pil")


def save(im, out, what):
    im.convert("P", palette=Image.ADAPTIVE, colors=256).save(out, optimize=True)
    print(f"{out}: {im.width}x{im.height} from {what}")


def fill(path, size, out, x=0.5):
    im = Image.open(path).convert("RGB")
    save(ImageOps.fit(im, size, Image.LANCZOS, centering=(x, 0.5)), out, path)


def icon(path, out, size=128):
    im = Image.open(path).convert("RGB")
    ratio = im.width / im.height
    if 0.87 <= ratio <= 1.15:
        save(ImageOps.fit(im, (size, size), Image.LANCZOS), out, path)
        return
    # fitted inside the round bubble (84% of its height or width) on the
    # picture's own background colour
    s = size * 0.84 / max(im.width, im.height)
    small = im.resize((round(im.width * s), round(im.height * s)), Image.LANCZOS)
    canvas = Image.new("RGB", (size, size), im.getpixel((1, 1)))
    canvas.paste(small, ((size - small.width) // 2, (size - small.height) // 2))
    save(canvas, out, path + " (fitted)")


def main():
    args = sys.argv[1:]
    opts = {"--bg-x": 0.5, "--start-x": 0.5}
    files = []
    i = 0
    while i < len(args):
        if args[i] in opts and i + 1 < len(args):
            opts[args[i]] = min(1.0, max(0.0, float(args[i + 1])))
            i += 2
        elif args[i].startswith("--"):
            sys.exit(__doc__)
        else:
            files.append(args[i])
            i += 1
    if not 1 <= len(files) <= 3:
        sys.exit(__doc__)
    root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    dest = os.path.join(root, "livearea")
    os.makedirs(dest, exist_ok=True)
    here = os.path.dirname(os.path.abspath(__file__))
    bg = files[1] if len(files) > 1 else None
    start = files[2] if len(files) > 2 else bg
    icon(files[0], os.path.join(dest, "icon0.png"))
    if bg:
        fill(bg, (840, 500), os.path.join(dest, "bg.png"), opts["--bg-x"])
        fill(start, (280, 158), os.path.join(dest, "startup.png"), opts["--start-x"])
    # the pictures not given come from vita/livearea/
    for name in ("bg.png", "startup.png", "template.xml"):
        p = os.path.join(dest, name)
        if not os.path.exists(p):
            with open(os.path.join(here, name), "rb") as f, open(p, "wb") as o:
                o.write(f.read())
    print("Now run ./build.sh --my-art")


main()

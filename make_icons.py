#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""2048n_m 图标生成器
用法:
  python make_icons.py --windows 2048n_m.ico      # 生成 Windows 图标
  python make_icons.py --android android/res      # 生成 Android mipmap 各密度图标
  python make_icons.py --all 2048n_m.ico android/res
依赖: Pillow
"""
import argparse
import os
import sys

from PIL import Image, ImageDraw, ImageFont

BG = (250, 248, 239)        # 米白
TEXT = (119, 110, 101)      # 深棕
TILES = [(238, 228, 218), (237, 224, 200), (242, 177, 121)]  # 2 / 4 / 8
TILE_TEXT_DARK = (119, 110, 101)


def font(size):
    for name in ("DejaVuSans-Bold.ttf", "arialbd.ttf", "msyhbd.ttc"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def rounded(draw, box, radius, fill):
    draw.rounded_rectangle(box, radius=radius, fill=fill)


def render(size):
    """渲染一张 size x size 的 2048 风格图标"""
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    # 米白圆角底
    rounded(d, (0, 0, size - 1, size - 1), int(size * 0.18), BG + (255,))

    # 顶部三个小方块 (2 / 4 / 8)
    tw = size * 0.17
    gap = size * 0.035
    total = tw * 3 + gap * 2
    x0 = (size - total) / 2
    y0 = size * 0.16
    radius = int(tw * 0.15)
    for i, color in enumerate(TILES):
        x = x0 + i * (tw + gap)
        rounded(d, (x, y0, x + tw, y0 + tw), radius, color + (255,))
        num = str(2 ** (i + 1))
        f = font(int(tw * (0.62 if len(num) == 1 else 0.5)))
        bbox = d.textbbox((0, 0), num, font=f)
        d.text((x + (tw - (bbox[2] - bbox[0])) / 2 - bbox[0],
                y0 + (tw - (bbox[3] - bbox[1])) / 2 - bbox[1]),
               num, font=f, fill=TILE_TEXT_DARK + (255,))

    # 大号 2048
    f = font(int(size * 0.34))
    text = "2048"
    bbox = d.textbbox((0, 0), text, font=f)
    d.text(((size - (bbox[2] - bbox[0])) / 2 - bbox[0],
            size * 0.62 - (bbox[3] - bbox[1]) / 2 - bbox[1]),
           text, font=f, fill=TEXT + (255,))
    return img


def gen_windows(out_path):
    sizes = [16, 24, 32, 48, 64, 128, 256]
    imgs = [render(s) for s in sizes]
    imgs[0].save(out_path, format="ICO", sizes=[(s, s) for s in sizes],
                 append_images=imgs[1:])
    print("Windows 图标:", out_path)


DENSITIES = {  # 密度: 边长(px)
    "mdpi": 48, "hdpi": 72, "xhdpi": 96, "xxhdpi": 144, "xxxhdpi": 192,
}


def gen_android(out_dir):
    for name, px in DENSITIES.items():
        d = os.path.join(out_dir, "mipmap-" + name)
        os.makedirs(d, exist_ok=True)
        render(px).convert("RGBA").save(os.path.join(d, "ic_launcher.png"))
    big = os.path.join(out_dir, "play_store_512.png")
    render(512).convert("RGBA").save(big)
    print("Android 图标:", out_dir)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--windows", metavar="ICO")
    ap.add_argument("--android", metavar="DIR")
    ap.add_argument("--all", nargs=2, metavar=("ICO", "DIR"))
    args = ap.parse_args()
    if not (args.windows or args.android or args.all):
        ap.print_help()
        sys.exit(1)
    if args.all:
        args.windows, args.android = args.all
    if args.windows:
        gen_windows(args.windows)
    if args.android:
        gen_android(args.android)


if __name__ == "__main__":
    main()

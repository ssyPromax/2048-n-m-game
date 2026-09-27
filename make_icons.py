#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""2048n_m 图标生成器（头像背景版）
背景: GitHub 头像, 高斯虚化 + 白色半透明蒙版
文字: 20 / 48 两行
用法:
  python make_icons.py --windows 2048n_m.ico
  python make_icons.py --android android/res
  python make_icons.py --all 2048n_m.ico android/res
可选: --avatar <路径或URL>  自定义头像（默认自动下载 GitHub 头像）
依赖: Pillow
"""
import argparse
import io
import os
import sys
import urllib.request

from PIL import Image, ImageDraw, ImageFilter, ImageFont

AVATAR_URL = "https://avatars.githubusercontent.com/u/177380592"
AVATAR_MIRROR = "https://gh-proxy.com/" + AVATAR_URL
TEXT = (74, 66, 58)           # 深棕文字
MASK_ALPHA = 110              # 白色蒙版不透明度 (0~255)


def font(size):
    for name in ("DejaVuSans-Bold.ttf", "arialbd.ttf", "msyhbd.ttc"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def load_avatar(src):
    """从本地文件或 URL 加载头像"""
    if src and os.path.exists(src):
        return Image.open(src).convert("RGB")
    for url in (src or AVATAR_URL, AVATAR_MIRROR):
        try:
            req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
            data = urllib.request.urlopen(req, timeout=20).read()
            img = Image.open(io.BytesIO(data)).convert("RGB")
            if img.width >= 64:
                return img
        except Exception as e:
            print("头像下载失败, 尝试镜像:", e, file=sys.stderr)
    raise SystemExit("无法获取头像, 请用 --avatar 指定本地图片")


def cover_square(img, size):
    """居中裁剪为正方形后缩放到 size"""
    w, h = img.size
    s = min(w, h)
    img = img.crop(((w - s) // 2, (h - s) // 2, (w + s) // 2, (h + s) // 2))
    return img.resize((size, size), Image.LANCZOS)


def render(size, avatar):
    """渲染一张 size x size 的图标"""
    # 头像背景 + 虚化
    bg = cover_square(avatar, size).filter(ImageFilter.GaussianBlur(size / 18.0))

    # 白色半透明蒙版
    mask = Image.new("RGBA", (size, size), (255, 255, 255, MASK_ALPHA))
    bg = Image.alpha_composite(bg.convert("RGBA"), mask)

    # 圆角裁剪
    out = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    corner = Image.new("L", (size, size), 0)
    cd = ImageDraw.Draw(corner)
    cd.rounded_rectangle((0, 0, size, size), radius=int(size * 0.18), fill=255)
    out.paste(bg, (0, 0), corner)
    d = ImageDraw.Draw(out)

    # 两行数字 20 / 48
    f = font(int(size * 0.36))
    lines = ["20", "48"]
    bboxes = [d.textbbox((0, 0), t, font=f) for t in lines]
    lh = max(b[3] - b[1] for b in bboxes)          # 行高
    total = lh * 2 + size * 0.02                   # 两行总高 + 紧凑行距
    y = (size - total) / 2
    for t, b in zip(lines, bboxes):
        tw = b[2] - b[0]
        d.text(((size - tw) / 2 - b[0], y - b[1]), t, font=f, fill=TEXT + (255,))
        y += lh + size * 0.02
    return out


def gen_windows(out_path, avatar):
    sizes = [16, 24, 32, 48, 64, 128, 256]
    imgs = [render(s, avatar) for s in sizes]
    imgs[0].save(out_path, format="ICO", sizes=[(s, s) for s in sizes],
                 append_images=imgs[1:])
    print("Windows 图标:", out_path)


DENSITIES = {  # 密度: 边长(px)
    "mdpi": 48, "hdpi": 72, "xhdpi": 96, "xxhdpi": 144, "xxxhdpi": 192,
}


def gen_android(out_dir, avatar):
    for name, px in DENSITIES.items():
        d = os.path.join(out_dir, "mipmap-" + name)
        os.makedirs(d, exist_ok=True)
        render(px, avatar).save(os.path.join(d, "ic_launcher.png"))
    render(512, avatar).save(os.path.join(out_dir, "play_store_512.png"))
    print("Android 图标:", out_dir)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--windows", metavar="ICO")
    ap.add_argument("--android", metavar="DIR")
    ap.add_argument("--all", nargs=2, metavar=("ICO", "DIR"))
    ap.add_argument("--avatar", metavar="PATH_OR_URL",
                    help="头像图片，默认自动下载 GitHub 头像")
    args = ap.parse_args()
    if not (args.windows or args.android or args.all):
        ap.print_help()
        sys.exit(1)
    if args.all:
        args.windows, args.android = args.all
    avatar = load_avatar(args.avatar)
    if args.windows:
        gen_windows(args.windows, avatar)
    if args.android:
        gen_android(args.android, avatar)


if __name__ == "__main__":
    main()

"""latenight oled scheme from palemoon: black backgrounds, white text and icons

python tools/oled_scheme.py  (rerun after editing style.qss or style_palemoon.qss)
writes res/skins/LateNight/style_oled.qss and res/skins/LateNight/oled/
"""
import os
import re
import shutil

SKIN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "res", "skins", "LateNight")
COLOR = re.compile(r"#[0-9a-fA-F]{3,8}\b|rgba?\([^)]*\)")
DECL = re.compile(r"([a-z-]+)(\s*:\s*)([^;{}]+)(;)")
HEX6 = re.compile(r"#[0-9a-fA-F]{6}\b|#[0-9a-fA-F]{3}\b")
# black outline copy drawn under icon labels like SYNC
OUTLINE_TEXT = re.compile(r'\s*<text\b[^>]*\bstroke="#0{3}(?:0{3}|0{2}1)?"[^>]*>.*?</text>', re.S)
ORANGE_TO_BLUE = {(0xb2, 0x4c, 0x12): (0x1e, 0x64, 0xc8),  # active
                  (0xb2, 0x42, 0x1c): (0x1e, 0x64, 0xc8),
                  (0x7d, 0x35, 0x0d): (0x15, 0x46, 0x8c)}  # dim, momentary
# grey active toggles (gui toggles, pfl, split cue) -> blue
GREY_TO_BLUE = {(0x77, 0x77, 0x77): (0x1e, 0x64, 0xc8),
                (0x66, 0x66, 0x66): (0x1e, 0x64, 0xc8),
                (0x55, 0x55, 0x55): (0x15, 0x46, 0x8c)}


def parse(c):
    """(r, g, b, alpha or None, kind), None if unsupported"""
    if c.startswith("#"):
        h = c[1:]
        if len(h) == 3:
            return tuple(int(x * 2, 16) for x in h) + (None, "hex")
        if len(h) == 6:
            return tuple(int(h[i:i + 2], 16) for i in (0, 2, 4)) + (None, "hex")
        if len(h) == 8:  # qt: #aarrggbb
            return tuple(int(h[i:i + 2], 16) for i in (2, 4, 6)) + (h[:2], "hex8")
        return None
    parts = [p.strip() for p in c[c.index("(") + 1:-1].split(",")]
    if len(parts) not in (3, 4) or not all(p.isdigit() for p in parts[:3]):
        return None
    return tuple(int(p) for p in parts[:3]) + (parts[3] if len(parts) == 4 else None, "rgb")


def fmt(rgb, alpha, kind):
    if kind == "hex8":
        return "#%s%02x%02x%02x" % ((alpha,) + rgb)
    if kind == "rgb" and alpha is not None:
        return "rgba(%d, %d, %d, %s)" % (rgb + (alpha,))
    return "#%02x%02x%02x" % rgb


def remap(prop, c):
    p = parse(c.lower())
    if p is None:
        return c
    r, g, b, alpha, kind = p
    spread, light = max(r, g, b) - min(r, g, b), (r + g + b) / 3
    if (r, g, b) in ORANGE_TO_BLUE and prop not in ("color", "selection-color"):
        return fmt(ORANGE_TO_BLUE[(r, g, b)], alpha, kind)
    if prop == "color" and (r, g, b) == (0, 0, 0):  # dark text on lit buttons
        return fmt((255, 255, 255), alpha, kind)
    if prop in ("color", "selection-color"):
        if spread <= 40 and light >= 0x70:  # greys and ivory -> white
            return fmt((255, 255, 255), alpha, kind)
    elif "background" in prop:
        if (r, g, b) in GREY_TO_BLUE:
            return fmt(GREY_TO_BLUE[(r, g, b)], alpha, kind)
        if spread <= 12 and light <= 0x22:
            return fmt((0, 0, 0), alpha, kind)
    elif "border" in prop or prop in ("outline", "gridline-color"):
        if spread <= 12 and light <= 0x10:  # keep visible separators
            return fmt((0, 0, 0), alpha, kind)
    return c


def convert(text):
    text = text.replace("LateNight/palemoon/", "LateNight/oled/")
    return DECL.sub(lambda m: m.group(1) + m.group(2)
                    + COLOR.sub(lambda c: remap(m.group(1), c.group(0)), m.group(3)) + m.group(4), text)


def remap_svg(c, brighten):
    h = c[1:] if len(c) == 7 else "".join(x * 2 for x in c[1:])
    r, g, b = (int(h[i:i + 2], 16) for i in (0, 2, 4))
    spread, light = max(r, g, b) - min(r, g, b), (r + g + b) // 3
    if spread > 40:  # accents
        return c
    if brighten and light >= 0x70:  # beige icons -> near white
        light = min(255, light + 90)
    return "#%02x%02x%02x" % (light, light, light)  # warm tint -> neutral


def make_images():
    src, dst = os.path.join(SKIN, "palemoon"), os.path.join(SKIN, "oled")
    shutil.rmtree(dst, ignore_errors=True)
    for root, _, files in os.walk(src):
        rel = os.path.relpath(root, src)
        os.makedirs(os.path.join(dst, rel), exist_ok=True)
        icons = rel.split(os.sep)[0] in ("buttons", "style", ".")  # not knobs, sliders
        for name in files:
            s, d = os.path.join(root, name), os.path.join(dst, rel, name)
            if not name.endswith(".svg"):
                shutil.copyfile(s, d)
                continue
            with open(s, encoding="utf-8", errors="surrogateescape", newline="") as f:
                text = f.read()
            if text.strip().endswith(".svg") and "<" not in text:  # git symlink without symlinks
                with open(os.path.join(root, text.strip()), encoding="utf-8", newline="") as f:
                    text = f.read()
            if icons:
                text = OUTLINE_TEXT.sub("", text)
            with open(d, "w", encoding="utf-8", errors="surrogateescape", newline="") as f:
                f.write(HEX6.sub(lambda m: remap_svg(m.group(0).lower(), icons), text))


def main():
    out = ["/* generated by tools/oled_scheme.py from style.qss + style_palemoon.qss, do not edit */\n"]
    for name in ("style.qss", "style_palemoon.qss"):  # palemoon cascade order
        with open(os.path.join(SKIN, name), encoding="utf-8") as f:
            out.append("\n/* ---- %s ---- */\n" % name + convert(f.read()))
    with open(os.path.join(SKIN, "style_oled.qss"), "w", encoding="utf-8", newline="\n") as f:
        f.write("".join(out))
    make_images()


if __name__ == "__main__":
    assert remap("color", "#c2b3a5") == "#ffffff"
    assert remap("color", "#444") == "#444"
    assert remap("background-color", "#19191a") == "#000000"
    assert remap("background-color", "rgba(15, 15, 15, 220)") == "rgba(0, 0, 0, 220)"
    assert remap("background-color", "#2c454f") == "#2c454f"
    assert remap("border", "#0c0c0c") == "#000000" and remap("border", "#333") == "#333"
    assert remap("background-color", "#b24c12") == "#1e64c8"
    assert remap("background-color", "#777") == "#1e64c8" and remap("color", "#000") == "#ffffff"
    assert remap_svg("#a89a8c", True) == "#f4f4f4"
    assert remap_svg("#66564d", True) == "#585858"  # leader crown
    assert remap_svg("#a3a4a2", False) == "#a3a3a3"
    assert remap_svg("#ffaa26", True) == "#ffaa26"
    assert OUTLINE_TEXT.sub("", '<g><text stroke="#000001">S</text><text fill="#fff">S</text></g>') \
        == '<g><text fill="#fff">S</text></g>'
    main()

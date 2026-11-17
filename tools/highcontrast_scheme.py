"""latenight "High Contrast (dark)" scheme from palemoon

black backgrounds, white text and icons, grey frames, blue for active buttons.
python tools/highcontrast_scheme.py  (rerun after editing style.qss or style_palemoon.qss)
writes res/skins/LateNight/style_highcontrast.qss and res/skins/LateNight/highcontrast/
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
# frame of the spinbox arrow buttons: black ring path
RING = re.compile(r'(<path [^>]*?d="[^"]*z *m[^"]*"[^>]*? fill=")#00000[01](")', re.I)
# palemoon separates dark greys with darker or lighter lines and bevels. on
# black these all vanish, so every line and frame gets the same visible grey
FRAME = (0x5a, 0x5a, 0x5a)
# bevel with black top and left (search box, sidebar, track table): only the
# bottom right corner would get a frame, so the lighter half goes black as well
HALF_BEVEL = re.compile(r"(border-top:[^;{}]*#000;\s*border-left:[^;{}]*#000;\s*border-bottom:[^;{}#]*)"
                        r"#[0-9a-f]+(;\s*border-right:[^;{}#]*)#[0-9a-f]+;", re.I)
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
    spread, light = max(r, g, b) - min(r, g, b), (r + g + b) // 3
    if (r, g, b) in ORANGE_TO_BLUE and prop not in ("color", "selection-color"):
        return fmt(ORANGE_TO_BLUE[(r, g, b)], alpha, kind)
    if prop == "color" and (r, g, b) == (0, 0, 0):  # dark text on lit buttons
        return fmt((255, 255, 255), alpha, kind)
    if prop in ("color", "selection-color"):
        if spread <= 40 and light >= 0x70:  # greys and ivory -> white
            return fmt((255, 255, 255), alpha, kind)
        if spread <= 40 and light >= 0x30:  # dim labels stay dimmer, but readable
            return fmt((light + 0x50,) * 3, alpha, kind)
    elif "background" in prop:
        if (r, g, b) in GREY_TO_BLUE:
            return fmt(GREY_TO_BLUE[(r, g, b)], alpha, kind)
        if spread <= 12 and light <= 0x22:
            return fmt((0, 0, 0), alpha, kind)
    elif "border" in prop or prop in ("outline", "gridline-color"):
        if spread <= 12 and 0 < light < FRAME[0]:
            return fmt(FRAME, alpha, kind)
    return c


def convert(text):
    text = text.replace("LateNight/palemoon/", "LateNight/highcontrast/")
    text = HALF_BEVEL.sub(lambda m: m.group(1) + "#000" + m.group(2) + "#000;", text)
    return DECL.sub(lambda m: m.group(1) + m.group(2)
                    + COLOR.sub(lambda c: remap(m.group(1), c.group(0)), m.group(3)) + m.group(4), text)


def kind_of(rel, name):
    """how the greys of an image are used"""
    top = rel.split(os.sep)[0]
    if top in ("knobs", "sliders"):
        return "control"
    if name.startswith("btn_embedded_"):
        return "frame"  # button, spinbox and header backgrounds
    if "xfader" in name:
        return "switch"
    return "icon"


def remap_svg(c, kind):
    h = c[1:] if len(c) == 7 else "".join(x * 2 for x in c[1:])
    r, g, b = (int(h[i:i + 2], 16) for i in (0, 2, 4))
    spread, light = max(r, g, b) - min(r, g, b), (r + g + b) // 3
    if spread > 40:  # accents, brighter on knob arcs and fader caps
        scale = min(1.35, 255 / max(r, g, b)) if kind == "control" else 1
        return "#%02x%02x%02x" % tuple(round(x * scale) for x in (r, g, b))
    if kind == "frame" and light < 0x40:  # outline and bevel of the button
        light = FRAME[0]
    elif kind == "switch" and 0x08 <= light < 0x30:  # track of the switch
        light = 0x40
    elif kind == "control" and 0x30 <= light:  # knob rims, scale marks
        light = min(255, light + 0x48)
    elif light >= 0x70:  # beige icons -> near white
        light = min(255, light + 90)
    return "#%02x%02x%02x" % (light, light, light)  # warm tint -> neutral


def make_images():
    src, dst = os.path.join(SKIN, "palemoon"), os.path.join(SKIN, "highcontrast")
    shutil.rmtree(dst, ignore_errors=True)
    for root, _, files in os.walk(src):
        rel = os.path.relpath(root, src)
        os.makedirs(os.path.join(dst, rel), exist_ok=True)
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
            kind = kind_of(rel, name)
            if kind != "control":
                text = OUTLINE_TEXT.sub("", text)
            elif name.startswith("slider_"):  # groove, black in palemoon
                text = text.replace('stroke="#000"', 'stroke="#%02x%02x%02x"' % FRAME)
            if "spinbox" in name:
                text = RING.sub(lambda m: m.group(1) + "#%02x%02x%02x" % FRAME + m.group(2), text)
            with open(d, "w", encoding="utf-8", errors="surrogateescape", newline="") as f:
                f.write(HEX6.sub(lambda m: remap_svg(m.group(0).lower(), kind), text))


def main():
    out = ["/* generated by tools/highcontrast_scheme.py from style.qss + style_palemoon.qss,"
           " do not edit */\n"]
    for name in ("style.qss", "style_palemoon.qss"):  # palemoon cascade order
        with open(os.path.join(SKIN, name), encoding="utf-8") as f:
            out.append("\n/* ---- %s ---- */\n" % name + convert(f.read()))
    with open(os.path.join(SKIN, "style_highcontrast.qss"), "w", encoding="utf-8", newline="\n") as f:
        f.write("".join(out))
    make_images()


if __name__ == "__main__":
    assert remap("color", "#c2b3a5") == "#ffffff"
    assert remap("color", "#444") == "#949494" and remap("color", "#161617") == "#161617"
    assert remap("background-color", "#19191a") == "#000000"
    assert remap("background-color", "rgba(15, 15, 15, 220)") == "rgba(0, 0, 0, 220)"
    assert remap("background-color", "#2c454f") == "#2c454f"
    # bevels become one frame color, explicit black and lighter lines stay
    assert remap("border", "#0c0c0c") == remap("border-top", "#333") == "#5a5a5a"
    assert remap("border", "#000") == "#000" and remap("border", "#585858") == "#5a5a5a"
    assert remap("border", "#fff") == "#fff" and remap("border", "#257b82") == "#257b82"
    assert "5a5a5a" not in convert("a {\n  border-top: 1px solid #000;\n  border-left: 1px solid #000;\n"
                                   "  border-bottom: 1px solid #1e1e1e;\n  border-right: 1px solid #222;\n}")
    assert "5a5a5a" in convert("a { border-top: 1px solid #000; border-bottom: 1px solid #333; }")  # separator
    assert remap("background-color", "#b24c12") == "#1e64c8"
    assert remap("background-color", "#777") == "#1e64c8" and remap("color", "#000") == "#ffffff"
    assert remap_svg("#a89a8c", "icon") == "#f4f4f4"
    assert remap_svg("#66564d", "icon") == "#585858"  # leader crown
    assert remap_svg("#ffaa26", "icon") == "#ffaa26"
    assert remap_svg("#000001", "frame") == "#5a5a5a" and remap_svg("#dcdcdc", "frame") == "#ffffff"
    assert remap_svg("#0f0f0f", "switch") == "#404040"
    assert remap_svg("#404040", "control") == "#888888" and remap_svg("#050505", "control") == "#050505"
    assert remap_svg("#b34e13", "control") == "#f2691a"
    assert OUTLINE_TEXT.sub("", '<g><text stroke="#000001">S</text><text fill="#fff">S</text></g>') \
        == '<g><text fill="#fff">S</text></g>'
    main()

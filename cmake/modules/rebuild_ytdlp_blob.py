"""Pack a shebang+zlib zipimport blob from yt-dlp + yt-dlp-ejs wheels.

Usage: python3 rebuild_ytdlp_blob.py <out_blob> <ytdlp.whl> <ejs.whl>
Reads the file list from stdin? No — args only. Keeps __main__.py from the
old AAR convention (version-agnostic entry calling yt_dlp.main()).
"""

import io
import sys
import zipfile

MAIN = (
    b"#!/usr/bin/env python3\n"
    b"\n# Execute with\n# $ python3 -m yt_dlp\n\nimport sys\n\n"
    b"if __package__ is None and not getattr(sys, 'frozen', False):\n"
    b"    # direct call of __main__.py\n"
    b"    import os.path\n"
    b"    path = os.path.realpath(os.path.abspath(__file__))\n"
    b"    sys.path.insert(0, os.path.dirname(os.path.dirname(path)))\n\n"
    b"import yt_dlp\n\nif __name__ == '__main__':\n    yt_dlp.main()\n"
)

out_blob, ytdlp_whl, ejs_whl = sys.argv[1], sys.argv[2], sys.argv[3]

buf = io.BytesIO()
buf.write(b"#!/usr/bin/env python3\n")
with zipfile.ZipFile(buf, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as out:
    out.writestr("__main__.py", MAIN)
    for whl in (ytdlp_whl, ejs_whl):
        with zipfile.ZipFile(whl) as zin:
            for name in zin.namelist():
                top = name.split("/")[0]
                if top in ("yt_dlp", "yt_dlp_ejs") and not name.endswith("/"):
                    out.writestr(name, zin.read(name))

with open(out_blob, "wb") as f:
    f.write(buf.getvalue())
print(len(buf.getvalue()))

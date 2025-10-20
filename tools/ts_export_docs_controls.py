import os

from pathlib import Path
from docutils import nodes
from docutils.utils import new_document
from docutils.parsers.rst import Parser
from docutils.frontend import get_default_settings

manual_dir = (
    Path(os.path.dirname(os.path.realpath(__file__))) / "../../manual"
).absolute()
controls_file_path = manual_dir / "source/chapters/appendix/mixxx_controls.rst"
include_path = manual_dir / "source"


def parse_rst(file_path: Path, include_path: Path):
    with open(file_path, "r", encoding="utf-8") as f:
        text = f.read()

    parser = Parser()
    settings = get_default_settings(Parser)
    settings.file_insertion_enabled = True
    settings.input_encoding = "utf-8"
    settings.source_path = file_path
    settings.include_path = [include_path]

    document = new_document(str(file_path), settings)
    parser.parse(text, document)

    return document


doc = parse_rst(controls_file_path, include_path)

for node in doc.findall(nodes.title):
    print("Titel:", node.astext())

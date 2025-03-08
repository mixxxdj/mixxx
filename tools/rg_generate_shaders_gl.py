#!/usr/bin/env python3
"""
Converts a fragment or vertex shader file into a GL shader file.

You can use it like this:

    $ ./rg_generate_shaders_gl.py --cmake-output \\
        ../src/rendergraph/shaders/generated_shaders_gl.cmake \\
        ../src/rendergraph/shaders/*.{vert,frag}
"""
import argparse
import logging
import os
import pathlib
import shutil
import subprocess
import tempfile
import typing


def find_executable(
    executable_name: str, additional_paths: typing.Optional[list[str]] = None
) -> pathlib.Path:
    """Find an executable by name in $PATH and in the additional paths."""
    if executable_path := shutil.which(executable_name):
        return pathlib.Path(executable_path)

    if additional_paths:
        if executable_path := shutil.which(
            executable_name, path=os.pathsep.join(additional_paths)
        ):
            return pathlib.Path(executable_path)

    raise OSError(f"Executable {executable_name!r} not found!")


QSB_EXECUTABLE = find_executable(
    "qsb",
    additional_paths=[
        "/usr/lib/qt6/bin",
        "/lib/qt6/bin",
        "/usr/local/lib/qt6/bin",
    ],
)


def parse_shader(input_filepath: pathlib.Path) -> typing.Iterator[str]:
    """Parse a Fragment/Vertex shader file and yield lines for a GL file."""
    with tempfile.NamedTemporaryFile() as fp:
        subprocess.check_call(
            [
                QSB_EXECUTABLE,
                "--glsl",
                "120",
                "--output",
                fp.name,
                input_filepath,
            ]
        )
        output = subprocess.check_output(
            [QSB_EXECUTABLE, "--dump", fp.name],
            encoding="utf-8",
            universal_newlines=True,
        )

    comment_added = False
    ok = False
    in_shader_block = 0
    buffered_blank_line = False
    for line in output.splitlines():
        if in_shader_block == 2:
            if line.startswith("**"):
                ok = True
                break
            if not comment_added and not line.startswith("#"):
                yield "//// GENERATED - EDITS WILL BE OVERWRITTEN"
                comment_added = True
            if line:
                if buffered_blank_line:
                    yield ""
                    buffered_blank_line = False
                yield line
            else:
                buffered_blank_line = True
        elif in_shader_block == 1:
            if line.rstrip() == "Contents:":
                in_shader_block = 2
        else:
            if line.startswith("Shader") and line.rstrip().endswith(
                ": GLSL 120 [Standard]"
            ):
                in_shader_block = 1
    if not ok:
        print(f"qsb output: {output}")
        raise EOFError("end of file reached before end marker reached")


def get_paths(paths: list[pathlib.Path]) -> typing.Iterator[pathlib.Path]:
    for path in paths:
        if path.is_dir():
            yield from path.glob("*.vert")
            yield from path.glob("*.frag")
        else:
            yield path


def main(argv: typing.Optional[list[str]] = None) -> int:
    logging.basicConfig(level=logging.DEBUG, format="%(message)s")

    logger = logging.getLogger(__name__)

    description, _, epilog = __doc__.strip().partition("\n\n")
    parser = argparse.ArgumentParser(
        description=description,
        epilog=epilog,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "file",
        nargs="+",
        type=pathlib.Path,
        help="Input files (.vert, .frag) or directory",
    )
    parser.add_argument(
        "--cmake-output",
        type=argparse.FileType("w"),
        required=True,
        help="CMake Output files (.cmake)",
    )
    args = parser.parse_args(argv)

    generated_shaders: list[pathlib.Path] = []

    for file in sorted(get_paths(args.file)):
        logger.info("Reading file: %s", file)
        try:
            lines = list(parse_shader(file))
        except EOFError as err:
            logger.error("Failed to parse %s: %s", file, err)
            continue

        output_file = file.with_suffix(f"{file.suffix}.gl")
        logger.info("Writing file: %s", output_file)
        with output_file.open("w") as fp:
            for line in lines:
                fp.write(f"{line}\n")

        generated_shaders.append(output_file)

    args.cmake_output.write("set(\n")
    args.cmake_output.write("  generated_shaders_gl\n")
    for generated_file in generated_shaders:
        args.cmake_output.write(f"  {generated_file.name}\n")
    args.cmake_output.write(")\n")
    logger.info("Generated %d shader files.", len(generated_shaders))

    return 0


if __name__ == "__main__":
    main()

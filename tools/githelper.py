import itertools
import re
import logging
import subprocess
import typing


Line = typing.NamedTuple(
    "Line",
    [("sourcefile", str), ("number", int), ("text", str), ("added", bool)],
)
FileLines = typing.NamedTuple(
    "FileLines",
    [("filename", str), ("lines", typing.Sequence[typing.Tuple[int, int]])],
)


def get_toplevel_path() -> str:
    logger = logging.getLogger(__name__)

    cmd = ["git", "rev-parse", "--show-toplevel"]
    logger.debug("Executing: %r", cmd)
    return subprocess.check_output(cmd, text=True).strip()


def get_changed_lines(
    from_ref=None, filter_lines=None, include_files=None
) -> typing.Iterable[Line]:
    """Inspect `git diff` output, yields changed lines."""

    logger = logging.getLogger(__name__)

    cmd = ["git", "diff", "--unified=0", from_ref if from_ref else "HEAD"]
    if include_files:
        cmd.extend(["--", *include_files])
    logger.debug("Executing: %r", cmd)
    proc = subprocess.run(cmd, capture_output=True)
    proc.check_returncode()
    current_file = None
    lines_left = 0
    for line in proc.stdout.decode(errors="replace").splitlines():
        match_file = re.match(r"^\+\+\+ b/(.*)$", line)
        if match_file:
            current_file = match_file.group(1)
            lines_left = 0
            continue

        match_lineno = re.match(
            r"^@@ -(\d+(?:,\d+)?) \+([0-9]+(?:,[0-9]+)?) @@", line
        )
        if match_lineno:
            start_removed, _, length_removed = match_lineno.group(1).partition(
                ","
            )
            start_added, _, length_added = match_lineno.group(2).partition(",")
            lineno_removed = int(start_removed)
            lineno_added = int(start_added)
            lines_left = int(length_removed) if length_removed else 1
            lines_left += int(length_added) if length_added else 1
            continue

        if lines_left and line:
            lines_left -= 1

            assert line[0] in ("+", "-")
            if line.startswith("+"):
                lineno_added += 1
                current_lineno = lineno_added
            else:
                lineno_removed += 1
                current_lineno = lineno_removed

            lineobj = Line(
                sourcefile=current_file,
                number=current_lineno - 1,
                text=line[1:],
                added=line.startswith("+"),
            )

            if filter_lines is None or filter_lines(lineobj):
                yield lineobj


def get_changed_lines_grouped(
    from_ref=None, filter_lines=None, include_files=None
) -> typing.Iterable[FileLines]:
    """Inspect `git diff` output, yields changed lines grouped by file."""

    lines = get_changed_lines(from_ref, filter_lines, include_files)
    for filename, file_lines in itertools.groupby(
        lines, key=lambda line: line.sourcefile
    ):
        grouped_linenumbers = []
        start_linenumber = None
        last_linenumber = None
        for line in file_lines:
            if None not in (start_linenumber, last_linenumber):
                if line.number != last_linenumber + 1:
                    grouped_linenumbers.append(
                        (start_linenumber, last_linenumber)
                    )

            if start_linenumber is None:
                start_linenumber = line.number
            last_linenumber = line.number

        if None not in (start_linenumber, last_linenumber):
            grouped_linenumbers.append((start_linenumber, last_linenumber))

        if grouped_linenumbers:
            yield FileLines(filename, grouped_linenumbers)

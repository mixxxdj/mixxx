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
    from_ref=None, to_ref=None, filter_lines=None, include_files=None
) -> typing.Iterable[Line]:
    """Inspect `git diff-index` output, yields changed lines."""

    logger = logging.getLogger(__name__)

    if to_ref:
        changeset = "{}...{}".format(from_ref if from_ref else "HEAD", to_ref)
    else:
        changeset = from_ref if from_ref else "HEAD"

    # We're using the pre-commit framework which stashes all unstaged changes
    # before running the pre-commit hooks, so we don't need to add `--cached`
    # here. Also, if we run 2 hooks that modify the files, the second hook
    # should work on the diff that includes the unstaged changes made by the
    # first hook, not the original diff.
    cmd = ["git", "diff", "--patch", "--unified=0", changeset]
    if include_files:
        cmd.extend(["--", *include_files])
    logger.debug("Executing: %r", cmd)
    proc = subprocess.run(cmd, capture_output=True)
    proc.check_returncode()
    current_file = None
    hunk_lines_left = 0
    for line in proc.stdout.decode(errors="replace").splitlines():
        match_file = re.match(r"^\+\+\+ b/(.*)$", line)
        if match_file:
            # Current line contains a diff filename
            assert hunk_lines_left == 0
            current_file = match_file.group(1)
            continue

        match_lineno = re.match(
            r"^@@ -(\d+(?:,\d+)?) \+([0-9]+(?:,[0-9]+)?) @@", line
        )
        if match_lineno:
            # Current line contains a hunk header
            assert current_file is not None
            assert hunk_lines_left == 0
            start_removed, _, length_removed = match_lineno.group(1).partition(
                ","
            )
            start_added, _, length_added = match_lineno.group(2).partition(",")
            lineno_removed = int(start_removed)
            lineno_added = int(start_added)
            hunk_lines_left = int(length_removed) if length_removed else 1
            hunk_lines_left += int(length_added) if length_added else 1
            continue

        if hunk_lines_left and line:
            # Current line contains an added/removed line
            hunk_lines_left -= 1

            if line.startswith("+"):
                lineno_added += 1
                current_lineno = lineno_added
            elif line.startswith("-"):
                lineno_removed += 1
                current_lineno = lineno_removed
            else:
                assert line[0] == "\\"
                # This case can happen if the last line is lacking a newline at
                # the EOF, e.g. if the diff looks like this:
                #     -This is the last line.
                #     \ No newline at end of file
                #     +This is the last line.
                continue

            lineobj = Line(
                sourcefile=current_file,
                number=current_lineno - 1,
                text=line[1:],
                added=line.startswith("+"),
            )

            if filter_lines is None or filter_lines(lineobj):
                yield lineobj

        # If we reach this part, the line does not contain a diff filename or a
        # hunk header and does not belog to a hunk. This means that this line
        # will be ignored implicitly.

    # Make sure we really parsed all lines from the last hunk
    assert hunk_lines_left == 0


def get_changed_lines_grouped(
    from_ref=None, to_ref=None, filter_lines=None, include_files=None
) -> typing.Iterable[FileLines]:
    """Inspect `git diff-index` output, yields lines grouped by file."""

    lines = get_changed_lines(from_ref, to_ref, filter_lines, include_files)
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
                    start_linenumber = None

            if start_linenumber is None:
                start_linenumber = line.number
            last_linenumber = line.number

        if None not in (start_linenumber, last_linenumber):
            grouped_linenumbers.append((start_linenumber, last_linenumber))

        if grouped_linenumbers:
            yield FileLines(filename, grouped_linenumbers)

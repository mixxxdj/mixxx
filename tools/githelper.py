import os
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


def get_moved_files(
    changeset, include_files=None
) -> typing.Iterable[typing.Tuple[str, str]]:
    """
    Inspect `git diff` output to find moved/renamed files.

    Yields tuples in the form (old_file, new_file).

    If include_files is set, this only yields results where at least one of the
    two file names is in the list of file names to include.
    """

    logger = logging.getLogger(__name__)

    cmd = ["git", "diff", "--raw", "-z", changeset]
    logger.debug("Executing: %r", cmd)
    proc = subprocess.run(cmd, capture_output=True)
    proc.check_returncode()
    diff_output = proc.stdout.decode(errors="replace")
    for line in diff_output.lstrip(":").split(":"):
        change, _, files = line.partition("\0")
        _, _, changetype = change.rpartition(" ")
        if changetype.startswith("R"):
            # A file was renamed
            old_file, sep, new_file = files.rstrip("\0").partition("\0")
            assert sep == "\0"

            move = (old_file, new_file)
            if include_files and not any(x in include_files for x in move):
                continue

            yield move


def get_changed_lines(
    from_ref=None, to_ref=None, filter_lines=None, include_files=None
) -> typing.Iterable[Line]:
    """Inspect `git diff` output, yields changed lines."""

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
        # If you're committing inside a git worktree, it's possible that files
        # from *outside* the current git repository (i.e. in the original git
        # repository). Therefore we need to filter our all files outside of the
        # current repository before passing them to git diff.
        toplevel_path = get_toplevel_path()
        include_files = {
            path
            for path in include_files
            if os.path.commonprefix((os.path.abspath(path), toplevel_path))
            == toplevel_path
        }
        if not include_files:
            # No files to check
            return

        # If files were moved, it's possible that only the new filename is in
        # the list of included files. For example, this is the case when the
        # script is used by pre-commit. When calling `git diff` after a rename
        # with a path argument where only the new file is listed, git treats
        # the file as newly added and the whole file shows up as added lines.
        #
        # This leads to false positives because the lines were not actually
        # changed. Hence, we check if any of the files were renamed, and make
        # sure that both the old and new filename is included in the initial
        # `git diff` call.
        moved_files = get_moved_files(changeset, include_files=include_files)
        include_files_with_moved = include_files.union(
            itertools.chain.from_iterable(moved_files)
        )
        cmd.extend(["--", *include_files_with_moved])
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
                if include_files and lineobj.sourcefile not in include_files:
                    continue
                yield lineobj

        # If we reach this part, the line does not contain a diff filename or a
        # hunk header and does not belong to a hunk. This means that this line
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

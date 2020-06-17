#!/usr/bin/env python
"""Wrapper script for `git-clang-format` to avoid race conditions when
merging/rebasing CPP files with merge conflicts."""
from collections import namedtuple
from os import environ as env
from subprocess import check_output
from typing import List, Iterable
import argparse
import enum
import logging
import sys

logger = logging.getLogger(__name__)


class GitState(enum.Enum):
    """
    Excerpt from https://git-scm.com/docs/git-status

    For paths with merge conflicts, X and Y show the modification states
    of each side of the merge. For paths that do not have merge conflicts,
    X shows the status of the index, and Y shows the status of the work
    tree. For untracked paths, XY are ??. Other status codes can be
    interpreted as follows:
    M = modified
    A = added
    D = deleted
    R = renamed
    C = copied
    U = updated but unmerged
    """

    ADDED = "A"
    COPIED = "C"
    DELETED = "D"
    MODIFIED = "M"
    RENAMED = "R"
    UNMODIFIED = " "
    UNTRACKED = "?"
    UPDATED_BUT_UNMERGED = "U"


GitEdit = namedtuple("GitEdit", ["x", "y", "path"])


def get_git_status() -> Iterable[GitEdit]:
    """Inspect `git status` output, return GitEdits generator."""

    command = ["git", "status", "--porcelain", "--untracked-files=no"]
    logger.debug("Executing `%s`", " ".join(command))
    git_status_output = check_output(args=command, text=True)

    edits = [
        edit for edit in git_status_output.rstrip().split("\n") if edit != ""
    ]
    logger.debug(
        "Found %d path(s) with state change in GIT status output", len(edits)
    )

    if edits:
        logger.debug("Pending GIT edits:\n%s", "\n".join(edits))

    return (
        git_edit
        for git_edit in (
            GitEdit(edit[0:1], edit[1:2], edit[3:]) for edit in edits
        )
        if git_edit.x in (GitState.MODIFIED.value, GitState.ADDED.value)
    )


def run_clang_format(file_list: List[str]) -> bool:
    """Execute git-clang-format for provided `file_list`.

    Return True if `git-clang-format` has errors or has modified any file,
    i.e. if it failed the formatting test.
    """
    if not file_list:
        logger.info("No file to be formatted, exiting")
        return

    command = ["git-clang-format"]

    src_commit = env.get("PRE_COMMIT_FROM_REF") or env.get("PRE_COMMIT_SOURCE")

    if src_commit:
        command.append(src_commit)

    command.append("--")
    command.extend(file_list)

    logger.info("Executing `%s`", " ".join(command))
    out = check_output(command, text=True)
    logger.debug(out)

    return False if out == "clang-format did not modify any files\n" else True


def main():
    # Setup CLI argument parser
    logging.basicConfig(
        format="[%(levelname)s] %(message)s", level=logging.DEBUG
    )
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "files", nargs="*", help="Restrict formatting to these files"
    )
    args = parser.parse_args()
    logger.debug("Wrapper called with `files` arguments: %s", args.files)

    # Inspect GIT status output and filter paths that must be clang-formatted
    paths_to_be_clang_formatted = [
        status.path for status in get_git_status() if status.path in args.files
    ]
    logger.debug(
        "Found %d staged path(s) to be clang-formatted",
        len(paths_to_be_clang_formatted),
    )

    if paths_to_be_clang_formatted:
        logger.debug("Paths:\n%s", "\n".join(paths_to_be_clang_formatted))

        if run_clang_format(paths_to_be_clang_formatted):
            return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())

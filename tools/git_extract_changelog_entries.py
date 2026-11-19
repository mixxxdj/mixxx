#!/usr/bin/env python3

"""
Git Extract Changelog Entries

This script iterates through Git commits and extracts a changelog line for all
merged pull requests in the given range to stdout, skipping entries that are
already present in CHANGELOG.md.

Usage:
    python script.py --start_commit <start_commit> \
    [--end_commit <end_commit>] [--repo_path /path/to/your/repo]

Dependencies:
    - GitPython library
"""

import argparse
import os
import re
import sys
import git


def extract_pull_requests(repo_path, start_commit, end_commit):
    try:
        repo = git.Repo(repo_path, search_parent_directories=True)
    except (git.exc.InvalidGitRepositoryError, git.exc.NoSuchPathError):
        print(
            f"Error: '{repo_path}' is not a valid Git repository.",
            file=sys.stderr,
        )
        sys.exit(1)

    pr_pattern = re.compile(r"Merge pull request #(\d+)")
    sync_branch_pattern = re.compile(r"sync-branch-\S+")

    # Read existing CHANGELOG.md contents to skip duplicate PR entries
    existing_changelog = ""
    if repo.bare or not repo.working_tree_dir:
        print(
            "Notice: Repository is bare (no working tree).",
            file=sys.stderr,
        )
        sys.exit(1)

    changelog_path = os.path.join(repo.working_tree_dir, "CHANGELOG.md")
    if not os.path.exists(changelog_path):
        print(
            f"Error: '{changelog_path}' not found.",
            file=sys.stderr,
        )
        sys.exit(1)

    try:
        with open(changelog_path, "r", encoding="utf-8") as f:
            existing_changelog = f.read()
    except OSError as e:
        print(
            f"Warning: Could not read {changelog_path}: {e}",
            file=sys.stderr,
        )

    try:
        commits = list(repo.iter_commits(rev=f"{start_commit}..{end_commit}"))
    except git.exc.GitCommandError as e:
        err_msg = e.stderr.strip() if e.stderr else str(e)
        print(
            f"Error: Failed to iterate commits ({err_msg})",
            file=sys.stderr,
        )
        sys.exit(1)

    for commit in commits:
        summary = commit.summary

        # Skip sync branch merges
        if sync_branch_pattern.search(summary):
            continue

        match = pr_pattern.search(summary)
        if match:
            pr_number = match.group(1)
            pr_tag = f"[#{pr_number}]"

            # Skip if the PR reference is already present in CHANGELOG.md
            if pr_tag in existing_changelog:
                continue

            commit_message_lines = commit.message.split("\n")
            print(
                f"* {commit_message_lines[2]} "
                + f"{pr_tag}"
                + f"(https://github.com/mixxxdj/mixxx/pull/{pr_number})"
            )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Git Extract Changelog Entries"
    )
    parser.add_argument(
        "--start_commit", required=True, help="Starting commit SHA"
    )
    parser.add_argument(
        "--end_commit", default="HEAD", help="Ending commit SHA (default HEAD)"
    )
    parser.add_argument(
        "--repo_path",
        default=".",
        help="Path to the Git repository (default: current directory)",
    )

    args = parser.parse_args()
    extract_pull_requests(args.repo_path, args.start_commit, args.end_commit)

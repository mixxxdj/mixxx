#!/usr/bin/env python3

"""
Git Extract Changelog Entries

This script iterates through Git commits and extracts a changelog line for all
merged pull request in the given range to stdout

Usage:
    python script.py --start_commit <start_commit> \
    [--end_commit <end_commit>] [--repo_path /path/to/your/repo]

Dependencies:
    - GitPython library
"""

import git
import re
import argparse


def extract_pull_requests(repo_path, start_commit, end_commit):
    repo = git.Repo(repo_path)
    pr_pattern = re.compile(r"Merge pull request #(\d+)")

    for commit in repo.iter_commits(rev=f"{start_commit}..{end_commit}"):
        match = pr_pattern.search(commit.summary)
        if match:
            pr_number = match.group(1)
            commit_message_lines = commit.message.split("\n")
            print(
                f"* {commit_message_lines[2]} "
                + f"[#{pr_number}]"
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

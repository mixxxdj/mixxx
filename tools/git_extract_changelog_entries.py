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
import git


def extract_pull_requests(repo_path, start_commit, end_commit):
    repo = git.Repo(repo_path)
    pr_pattern = re.compile(r"Merge pull request #(\d+)")
    sync_branch_pattern = re.compile(r"mixxxdj\/sync-branch-[\d.]+-to-\S+")

    # Read existing CHANGELOG.md contents to skip duplicate PR entries
    changelog_path = os.path.join(repo.working_tree_dir, "CHANGELOG.md")
    existing_changelog = ""
    if os.path.exists(changelog_path):
        with open(changelog_path, "r", encoding="utf-8") as f:
            existing_changelog = f.read()

    for commit in repo.iter_commits(rev=f"{start_commit}..{end_commit}"):
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

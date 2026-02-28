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
import requests
import re
import os
import argparse
import collections

r = requests.Session()


def get_category_key(category):
    return category if category != "Uncategorized" else None


def get_milestone_key(milestone):
    return milestone.get("title") if milestone else None


def get_milestone_title(milestone):
    return (
        f"[{milestone.get('title')}]({milestone.get('url')})"
        if milestone
        else "Unreleased"
    )


class Milestone(dict):
    def __init__(self):
        super().__init__()
        self.start = None

    def offset(self, threshold, lines):
        for k, v in self.items():
            if v >= threshold:
                self[k] += lines
        if self.start >= threshold:
            self.start += lines


class Changelog:
    def __init__(self):
        self.keys = collections.defaultdict(Milestone)
        self.start = None
        self.lines = []
        self.pr_numbers = set()

    def offset(self, threshold, lines=1):
        for v in self.keys.values():
            v.offset(threshold, lines)
        if self.start >= threshold:
            self.start += lines

    def read(self, f):
        current_milestone = None
        current_category = None
        pr_pattern = re.compile(r"\[#(\d+)\]")
        version = re.compile(r"\[(\d\.\d\.\d)\]")
        self.pr_numbers.clear()

        for nth, line in enumerate(f.readlines()):
            self.lines.append(line)
            match = pr_pattern.search(line)
            if line.startswith("## "):
                if not current_milestone:
                    self.start = nth - 1
                if current_category:
                    self.keys[current_milestone][
                        get_category_key(current_category)
                    ] = (nth - 1)
                    current_category = None
                match = version.search(line)
                if match:
                    current_milestone = match.group(1)
                elif line.strip().endswith("Unreleased"):
                    current_milestone = None
                else:
                    current_milestone = line[2:].strip()
                self.keys[current_milestone].start = nth + 1
            elif line.startswith("### "):
                if current_category:
                    self.keys[current_milestone][
                        get_category_key(current_category)
                    ] = (nth - 1)
                current_category = line[3:].strip()
            elif match:
                self.pr_numbers.add(match.group(1))

    def add(self, milestone, category, item):
        key = get_milestone_key(milestone)
        if key not in self.keys:
            self.lines.insert(
                self.start, f"\n## {get_milestone_title(milestone)}\n"
            )
            self.offset(self.start)
            self.keys[key].start = self.start
        if category not in self.keys[key]:
            self.lines.insert(
                self.keys[key].start,
                (
                    f"\n### {category}\n\n"
                    if category
                    else "\n### Uncategorized\n\n"
                ),
            )
            self.offset(self.keys[key].start)
            self.keys[key][category] = self.keys[key].start
            if key is None:
                self.keys[key].start += 1
        self.lines.insert(self.keys[key][category], item)
        self.offset(self.keys[key][category])

    def write(self, f):
        f.writelines(self.lines)


def fetch_info(owner, repo, pull_number):
    query = """query GetProjectItems($owner: String!, $repo: String!, $pull_number: Int!) {
        repository(owner: $owner, name: $repo) {
            pullRequest(number: $pull_number) {
                projectItems(first: 10) {
                    nodes {
                        fieldValueByName(name: "Category") {
                            ... on ProjectV2ItemFieldSingleSelectValue {
                                name
                                field {
                                ... on ProjectV2SingleSelectField {
                                    name
                                }
                            }
                        }
                    }
                }
            }
            milestone {
                title
                url
            }
            labels(first: 20) {
                nodes {
                    name
                }
            }
        }
    }
    }"""
    response = r.post(
        "https://api.github.com/graphql",
        json=dict(
            query=query,
            variables=dict(owner=owner, repo=repo, pull_number=pull_number),
        ),
        headers=dict(Authorization=f"Bearer {os.getenv('GITHUB_TOKEN')}"),
    )
    data = (
        response.json()
        .get("data", {})
        .get("repository", {})
        .get("pullRequest", {})
    )
    category = None
    labels = []
    nodes = data.get("projectItems", {}).get("nodes", [])
    if nodes:
        category = nodes[0].get("fieldValueByName", {}).get("name")
    milestone = data.get("milestone")
    nodes = data.get("labels")
    if nodes:
        labels = [
            node.get("name")
            for node in nodes.get("nodes", [])
            if node.get("name")
        ]
    return category, milestone, labels


changelog = Changelog()


def extract_pull_requests(repo_path, end_commit, file):
    repo = git.Repo(repo_path)
    start_commit = next(repo.iter_commits(paths=file, max_count=1))

    pr_pattern = re.compile(r"\[#(\d+)\]")
    with open(file) as f:
        changelog.read(f)

    pr_pattern = re.compile(r"Merge pull request #(\d+)")

    for commit in repo.iter_commits(rev=f"{start_commit}..{end_commit}"):
        match = pr_pattern.search(commit.summary)
        if match:
            pr_number = match.group(1)
            commit_message_lines = commit.message.split("\n")

            if pr_number in changelog.pr_numbers:
                continue

            category, milestone, labels = fetch_info(
                owner="mixxxdj", repo="mixxx", pull_number=int(pr_number)
            )
            if "sync-branches" in labels or "changelog" in labels:
                continue
            changelog.add(
                milestone,
                category,
                f"* {commit_message_lines[2]} [#{pr_number}](https://github.com/mixxxdj/mixxx/pull/{pr_number})\n",
            )

    with open(file, "w") as f:
        changelog.write(f)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Git Extract Changelog Entries"
    )
    parser.add_argument(
        "--end_commit", default="HEAD", help="Ending commit SHA (default HEAD)"
    )
    parser.add_argument(
        "--repo_path",
        default=".",
        help="Path to the Git repository (default: current directory)",
    )
    parser.add_argument(
        "--file",
        default=".",
        help="Path to the changelog file (default: CHANGELOG.md)",
    )

    args = parser.parse_args()
    extract_pull_requests(args.repo_path, args.end_commit, args.file)

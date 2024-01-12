import git
import re

repo_path = "."

repo = git.Repo(repo_path)

# Starting commit SHA
start_commit = "2.4"

# Regular expression pattern to extract PR number
pr_pattern = re.compile(r"Merge pull request #(\d+)")

# Iterate through commits
for commit in repo.iter_commits(rev=f"{start_commit}..upstream/2.4"):
    match = pr_pattern.search(commit.summary)
    if match:
        pr_number = match.group(1)
        commit_message_lines = commit.message.split("\n")
        print(
            f"* {commit_message_lines[2]} "
            + f"[#{pr_number}]"
            + f"(https://github.com/mixxxdj/mixxx/pull/{pr_number})"
        )

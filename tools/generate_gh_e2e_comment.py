#!/usr/bin/env python3
"""Build and update a PR comment with E2E test results.

Called from .github/workflows/build.yml in the e2e-comment job.
Uses the `gh` CLI (pre-installed on GitHub Actions runners) for all API calls.

Environment variables:
  GITHUB_REPOSITORY  - owner/repo (required)
  GITHUB_RUN_ID      - workflow run ID (required)
  GITHUB_SERVER_URL  - GitHub server URL (required)
  GH_TOKEN           - GitHub API token (required,
                       auto-set from ${{ github.token }})
  PR_NUMBER          - pull request number (required)
  COMMENT_ID         - existing comment ID to update (optional)
  RESULTS_DIR        - path to downloaded artifacts (default: results)

When NO_GITHUB=1 is set, the script runs in offline mode: it reads results
from RESULTS_DIR, prints the generated comment body to stdout, and skips all
GitHub API calls (using local .mkv files for video URLs).
"""

import enum
import json
import os
import pathlib
import re
import subprocess
from typing import NamedTuple

TITLE_MARKER = "<!-- mixxx-e2e-results -->"

SLUG_NAMES = {
    "ubuntu-jammy": "Ubuntu 26.04",
    "macos-macosintel": "macOS 15 x64",
    "windows-win64": "Windows Server 2025 VS2026 x64",
    "windows-winarm": "Windows 11 VS2026 ARM64",
}


class OutcomeCategory(enum.Enum):
    PASSED = "passed"
    FAILED = "failed"
    ERROR = "error"
    EXPECTED_FAILURE = "expected failure"
    UNEXPECTED_PASS = "unexpected pass"
    FLAKY = "flaky"
    SKIPPED = "skipped"
    OTHER = "other"

    def display(self, count):
        if self == OutcomeCategory.PASSED:
            return f"{count} passed"
        if self == OutcomeCategory.FAILED:
            return f"{count} failed"
        if self == OutcomeCategory.ERROR:
            return f"{count} error{'' if count == 1 else 's'}"
        if self == OutcomeCategory.FLAKY:
            return f"{count} flaky"
        if self == OutcomeCategory.SKIPPED:
            return f"{count} skipped"
        return f"{count} {self.value}"


class JobRef(NamedTuple):
    """Identity of a workflow job, enough to derive its URLs."""

    id: int
    attempt: int


# ---------------------------------------------------------------------------
# GitHub API helpers
# ---------------------------------------------------------------------------


def gh_api(endpoint, method="GET", fields=None):
    cmd = ["gh", "api", endpoint]
    cmd += ["-H", "Accept: application/vnd.github+json"]
    cmd += ["-H", "X-GitHub-Api-Version: 2022-11-28"]
    if method != "GET":
        cmd += ["-X", method]
    if fields:
        for k, v in fields.items():
            cmd += ["-f", f"{k}={v}"]
    result = subprocess.run(cmd, capture_output=True, text=True, check=True)
    if result.stdout.strip():
        return json.loads(result.stdout)
    return None


def gh_api_list(endpoint, key):
    """Collect paginated results from a list endpoint."""
    items = []
    page = 1
    while True:
        data = gh_api(f"{endpoint}?per_page=100&page={page}")
        items.extend(data[key])
        if len(data[key]) < 100:
            break
        page += 1
    return items


# ---------------------------------------------------------------------------
# Helpers mirroring the original inline JS
# ---------------------------------------------------------------------------


def status_emoji(outcome):
    return {
        "PASSED": "\u2705",
        "UNEXPECTED PASS": "\u2705\u2753",
        "FAILED": "\u274c",
        "ERROR": "\U0001f4a3",
        "SKIPPED": "\u23ed\ufe0f",
        "EXPECTED FAILURE": "\U0001f527",
        "FLAKY": "\u26a0\ufe0f",
    }.get(outcome.upper(), f"\u2753({outcome})")


def scenario_label(result, repo):
    name = result["name"]
    outcome = result.get("outcome", "")
    gh_issue = result.get("gh_issue")
    if outcome in ("expected failure", "unexpected pass"):
        if gh_issue:
            name += (
                f" ([#{gh_issue}]"
                f"(https://github.com/{repo}/issues/{gh_issue}))"
            )
        else:
            name += " (missing GH issue!)"
    return name


def timecode(result):
    start_ms = result.get("start_ms")
    if start_ms is None:
        return ""
    offset = start_ms / 1000.0
    m = int(offset // 60)
    s = int(offset % 60)
    return f"{m}:{s:02d}"


def collapse_errors(attempts):
    pieces = []
    for a in attempts:
        outcome = a.get("outcome", "")
        if outcome not in ("failed", "error", "expected failure"):
            continue
        e = (a.get("error") or "").split("\n")[0]
        if not e:
            continue
        for raw in e.split(" / "):
            p = raw.strip().replace("|", "\\|")
            if p:
                pieces.append(p)
    counts = {}
    order = []
    for p in pieces:
        if p not in counts:
            order.append(p)
        counts[p] = counts.get(p, 0) + 1
    return " / ".join(
        f"{p} [x{counts[p]}]" if counts[p] > 1 else p for p in order
    )


def format_duration(seconds):
    seconds = int(seconds)
    m, s = divmod(seconds, 60)
    h, m = divmod(m, 60)
    if h:
        return f"{h}:{m:02d}:{s:02d}"
    return f"{m}:{s:02d}"


def section_took(attempts):
    """Wall time from the earliest start to the latest end of the attempts.

    Returns None when any timestamp is missing so callers can omit the
    duration instead of printing a misleading value.
    """
    if not attempts:
        return None
    starts = [a.get("start_time") for a in attempts]
    ends = [a.get("end_time") for a in attempts]
    if any(s is None for s in starts) or any(e is None for e in ends):
        return None
    return max(ends) - min(starts)


def counts_suffix(counts, took=None):
    expected_total = (
        counts[OutcomeCategory.EXPECTED_FAILURE]
        + counts[OutcomeCategory.UNEXPECTED_PASS]
    )
    parts = [
        cat.display(counts[cat])
        for cat in (
            OutcomeCategory.PASSED,
            OutcomeCategory.FAILED,
            OutcomeCategory.ERROR,
            OutcomeCategory.FLAKY,
            OutcomeCategory.SKIPPED,
        )
        if counts[cat]
    ]
    if expected_total:
        parts.append(
            f"{expected_total} expected failure"
            f"{'s' if expected_total > 1 else ''}"
        )
    if took is not None:
        parts.append(f"took {format_duration(took)}")
    return f" ({', '.join(parts)})" if parts else ""


def section_mark(counts):
    if counts[OutcomeCategory.FAILED] or counts[OutcomeCategory.ERROR]:
        return ":x:"
    return ":white_check_mark:"


def summarize_counts(scenarios_by_name):
    """Tally the final outcome per scenario (last attempt by start time).

    `scenarios_by_name` must hold the scenarios of a single feature;
    scenario names are only unique within a feature, so the same name may
    legitimately occur in several features (e.g. when summing the feature
    counts of a whole platform).
    """
    counts = {cat: 0 for cat in OutcomeCategory}
    for name, attempts in scenarios_by_name.items():
        sorted_a = sorted(attempts, key=lambda a: a.get("start_time") or 0)
        last_a = sorted_a[-1]
        if not last_a:
            continue
        try:
            cat = OutcomeCategory(last_a.get("outcome", ""))
        except ValueError:
            cat = OutcomeCategory.OTHER
        counts[cat] += 1
    return counts, counts_suffix(counts)


# ---------------------------------------------------------------------------
# Data fetching
# ---------------------------------------------------------------------------


def fetch_video_urls(repo, run_id, server_url):
    artifacts = gh_api_list(
        f"/repos/{repo}/actions/runs/{run_id}/artifacts", "artifacts"
    )
    urls = {}
    for a in artifacts:
        name = a.get("name", "")
        if name.endswith(".mkv"):
            basename = name[:-4]
            urls[basename] = (
                f"{server_url}/{repo}/actions/runs"
                f"/{run_id}/artifacts/{a['id']}"
            )
    return urls


def fetch_job_ids(repo, run_id):
    """Map each testable platform name to the e2e job that ran it.

    The job name reported by the API is prefixed with the caller when the
    workflow is called as a reusable workflow (e.g. "build / Test e2e (macOS
    15 x64)"), so the prefix is optional in the pattern.
    """
    jobs = gh_api_list(f"/repos/{repo}/actions/runs/{run_id}/jobs", "jobs")
    pattern = re.compile(r"(?:.* / )?Test e2e \((.+)\)")
    ids = {}
    for job in jobs:
        m = pattern.fullmatch(job.get("name", "") or "")
        if m:
            ids[m.group(1)] = JobRef(
                id=job["id"], attempt=job.get("run_attempt") or 1
            )
    return ids


def job_links(job, repo, run_id, server_url):
    """Return the (job page, job summary) URLs for a job."""
    run_url = f"{server_url}/{repo}/actions/runs/{run_id}"
    return (
        f"{run_url}/job/{job.id}",
        f"{run_url}/attempts/{job.attempt}#summary-{job.id}",
    )


# ---------------------------------------------------------------------------
# Result parsing
# ---------------------------------------------------------------------------


def discover_local_video_urls(results_dir):
    """Walk results_dir for .mkv files and return {stem: file_url}."""
    urls = {}
    rdir = pathlib.Path(results_dir)
    if not rdir.is_dir():
        return urls
    for entry in rdir.iterdir():
        if entry.is_dir():
            for f in entry.iterdir():
                if f.name.endswith(".mkv"):
                    urls[f.name[:-4]] = f.resolve().as_uri()
    return urls


def read_results(results_dir, video_urls):
    """Parse JSON result files and augment with slug + video_url.

    For each JSON file, the file stem (basename without .json) is used to
    look up the corresponding video URL from video_urls.
    """
    all_results = []
    rdir = pathlib.Path(results_dir)
    if not rdir.is_dir():
        return all_results

    for entry in sorted(rdir.iterdir()):
        if not entry.is_dir():
            continue
        dirname = entry.name
        slug = dirname
        if dirname.startswith("mixxx-ui-test-"):
            slug = dirname[len("mixxx-ui-test-") :]

        for jf in sorted(entry.iterdir()):
            if not jf.name.endswith(".json"):
                continue
            try:
                with open(jf, "r") as f:
                    results = json.load(f)
            except OSError:
                continue

            video_basename = jf.name.replace(".json", "")
            video_url = video_urls.get(video_basename)

            for r in results:
                r["slug"] = slug
                r["video_url"] = video_url
                all_results.append(r)

    return all_results


# ---------------------------------------------------------------------------
# Comment body generation
# ---------------------------------------------------------------------------


def build_comment_body(all_results, job_ids, repo, run_id, server_url):
    if not all_results:
        return f"{TITLE_MARKER}\n# E2E Test\n\n**No test report found**\n"

    # Group by slug -> feature -> scenario name
    by_slug = {}
    for r in all_results:
        slug = r.get("slug", "unknown")
        by_slug.setdefault(slug, {}).setdefault(
            r.get("feature", "unknown"), {}
        ).setdefault(r.get("name", "unknown"), []).append(r)

    # Totals across all results (scenario names are only unique per feature)
    unique_scenarios = {(r.get("feature"), r.get("name")) for r in all_results}
    unique_slugs = {r.get("slug") for r in all_results if r.get("slug")}

    body_lines = [
        TITLE_MARKER,
        "# E2E Test",
        "",
        (
            f"**{len(unique_scenarios)} scenarios"
            f" across {len(unique_slugs)} platform"
            f"{'s' if len(unique_slugs) > 1 else ''}**"
        ),
        "",
    ]

    for slug in sorted(by_slug.keys()):
        by_feature = by_slug[slug]
        base_slug = re.sub(r"-e2e$", "", slug)
        slug_name = SLUG_NAMES.get(base_slug, slug)
        job = job_ids.get(slug_name)
        job_url = summary_url = None
        if job is not None:
            job_url, summary_url = job_links(job, repo, run_id, server_url)

        # Per-platform stats (sum of the feature stats, last attempt per
        # scenario) plus total wall time across the platform's scenarios
        plat_counts = {cat: 0 for cat in OutcomeCategory}
        for scenarios_by_name in by_feature.values():
            counts, _ = summarize_counts(scenarios_by_name)
            for cat, n in counts.items():
                plat_counts[cat] += n
        plat_took = section_took(
            [
                a
                for feat_dict in by_feature.values()
                for att_list in feat_dict.values()
                for a in att_list
            ]
        )
        stat_suffix = counts_suffix(plat_counts, plat_took)
        title_text = f"{section_mark(plat_counts)} {slug_name}{stat_suffix}"
        title_html = (
            f'<a href="{job_url}">{title_text}</a>' if job_url else title_text
        )

        feature_lines = [
            "<details>",
            f"<summary><strong>{title_html}</strong></summary>",
            "",
        ]

        has_timecode = any(
            r.get("start_ms") is not None
            for feat_dict in by_feature.values()
            for att_list in feat_dict.values()
            for r in att_list
        )

        for feature in sorted(by_feature.keys()):
            scenarios_by_name = by_feature[feature]
            feat_took = section_took(
                [
                    a
                    for att_list in scenarios_by_name.values()
                    for a in att_list
                ]
            )
            feat_counts = summarize_counts(scenarios_by_name)[0]
            feat_suffix = counts_suffix(feat_counts, feat_took)
            if job_url:
                feat_summary = (
                    f"<strong>├─ {section_mark(feat_counts)}"
                    f'<a href="{job_url}">{feature}</a>{feat_suffix}</strong>'
                )
            else:
                feat_summary = (
                    f"<strong>├─ {section_mark(feat_counts)}"
                    f"{feature}{feat_suffix}</strong>"
                )
            feature_lines.append("<details>")
            feature_lines.append(f"<summary>{feat_summary}</summary>")
            feature_lines.append("")

            cols = ["scenario", "status", "error"]
            if has_timecode:
                cols.insert(2, "timecode")

            table = []
            for name in sorted(
                scenarios_by_name.keys(),
                key=lambda n: min(
                    a.get("start_time") or 0 for a in scenarios_by_name[n]
                ),
            ):
                attempts = sorted(
                    scenarios_by_name[name],
                    key=lambda a: a.get("start_time") or 0,
                )
                values = {
                    "scenario": scenario_label(attempts[0], repo),
                    "status": " / ".join(
                        status_emoji(a.get("outcome", "")) for a in attempts
                    ),
                    "error": collapse_errors(attempts),
                }
                if has_timecode:
                    values["timecode"] = " / ".join(
                        timecode(a) for a in attempts
                    )
                table.append(values)

            feature_lines.append(
                "| " + " | ".join(c.title() for c in cols) + " |"
            )
            feature_lines.append("|" + "---|" * len(cols))
            feature_lines.extend(
                [
                    "| " + " | ".join(row[c] for c in cols) + " |"
                    for row in table
                ]
            )

            feature_lines.append("")

            # Video link (from the latest attempt)
            all_attempts = [
                a for attempts in scenarios_by_name.values() for a in attempts
            ]
            if all_attempts:
                latest = max(
                    all_attempts, key=lambda a: a.get("start_time") or 0
                )
                video_url = latest.get("video_url")
                parts = []
                if summary_url:
                    parts.append(f"[Job summary]({summary_url})")
                if job_url:
                    parts.append(f"[Job logs]({job_url})")
                if video_url:
                    parts.append(f"[{feature} video]({video_url})")
                else:
                    parts.append("*No video available*")
                feature_lines.append(" \u00b7 ".join(parts))
                feature_lines.append("")

            feature_lines.append("</details>")
            feature_lines.append("")

        feature_lines.append("</details>")
        feature_lines.append("")
        body_lines.extend(feature_lines)

    return "\n".join(body_lines)


def upsert_comment(repo, pr_number, comment_id, body):
    if comment_id:
        gh_api(
            f"/repos/{repo}/issues/comments/{comment_id}",
            method="PATCH",
            fields={"body": body},
        )
    else:
        gh_api(
            f"/repos/{repo}/issues/{pr_number}/comments",
            method="POST",
            fields={"body": body},
        )


def main():
    repo = os.environ.get("GITHUB_REPOSITORY", "")
    run_id = os.environ.get("GITHUB_RUN_ID", "")
    server_url = os.environ.get("GITHUB_SERVER_URL", "https://github.com")
    pr_number = os.environ.get("PR_NUMBER", "")
    comment_id = os.environ.get("COMMENT_ID", "")
    results_dir = os.environ.get("RESULTS_DIR", "results")
    no_github = os.environ.get("NO_GITHUB", "") == "1"

    # Build video URL map (from API or local files)
    if no_github:
        video_urls = discover_local_video_urls(results_dir)
        job_ids = {}
    else:
        video_urls = fetch_video_urls(repo, run_id, server_url)
        job_ids = fetch_job_ids(repo, run_id)

    all_results = read_results(results_dir, video_urls)
    body = build_comment_body(all_results, job_ids, repo, run_id, server_url)

    if no_github:
        print(body)
        return

    upsert_comment(
        repo, pr_number, int(comment_id) if comment_id else None, body
    )


if __name__ == "__main__":
    main()

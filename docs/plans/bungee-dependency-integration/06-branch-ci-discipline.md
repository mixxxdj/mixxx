# 06 — Branch, Backport, and Per-Commit CI Discipline

This migration must preserve a clean, reviewable `feature/bungee` branch where every commit is independently buildable and CI-green.

## 6.1 Branch source of truth

Treat `feature/bungee` as the integration branch that will eventually be submitted/re-submitted for review.

Current local branch observed while writing this plan:

```text
feat/bungee-petrify-debug
```

Before implementation, confirm the actual branch names with:

```bash
git branch --show-current
git branch --list '*bungee*'
git remote -v
```

## 6.2 Backport/cherry-pick rule

Any Bungee-related commits made on the current working branch, experimental branches, or review-fix branches must be backported into `feature/bungee` before final validation.

Preferred workflow:

1. Start from a clean working tree.
2. Update the target base branch from upstream Mixxx.
3. Check out `feature/bungee`.
4. Rebase `feature/bungee` onto the agreed upstream base, unless maintainers prefer merge commits.
5. Cherry-pick Bungee-related commits from the current branch in logical order.
6. Resolve conflicts once, then immediately build/test.
7. If a cherry-picked commit is not independently green, fix it before continuing to the next commit.

Example commands, to be adapted after confirming branch names:

```bash
git fetch upstream
git switch feature/bungee
git rebase upstream/main

git log --oneline upstream/main..feat/bungee-petrify-debug
# cherry-pick only the commits that belong in feature/bungee
git cherry-pick <commit1>
# build/test this commit before cherry-picking <commit2>
git cherry-pick <commit2>
```

If many commits need to move, use `git cherry-pick A^..B` only after verifying the range contains no unrelated work.

## 6.3 Per-commit green requirement

Every commit on `feature/bungee` must be independently valid:

- configures from a clean build directory;
- builds with `-DBUNGEE=ON` when dependencies are available;
- builds with `-DBUNGEE=OFF`;
- does not dirty the source tree during configure/build;
- passes relevant unit tests;
- passes every required Mixxx CI job for that commit.

Do not leave intermediate commits that only pass after a later fixup commit. If needed, squash/fixup/reorder with interactive rebase before pushing.

## 6.4 CI policy

The target for `feature/bungee` is **100% passing CI on every commit**.

Practical enforcement options:

1. Push each commit individually and wait for CI before pushing the next commit.
2. Use a temporary branch per commit if GitHub only runs CI at branch heads.
3. Use interactive rebase plus local validation, then ask maintainers whether branch-head-only CI is sufficient or whether they want proof of per-commit CI.
4. If Mixxx has a merge queue or required-check policy, do not mark PR ready until all required checks are green.

For each pushed commit, record:

- commit SHA;
- CI run URL;
- result;
- any skipped/not-applicable jobs and why.

A simple tracking table can be added to the PR description:

| Commit | Purpose | Local validation | CI result |
| --- | --- | --- | --- |
| `<sha>` | `<summary>` | `<commands>` | `<CI URL>` |

## 6.5 Commit structuring rules

Each commit should be atomic and reviewable. Good boundaries:

1. Improve `FindBungee.cmake` without changing runtime behavior.
2. Add package/vcpkg discovery path while keeping old path or fallback.
3. Add ExternalProject fallback if approved.
4. Switch Mixxx linking to the dependency target.
5. Remove vendored source.
6. Update docs/tests/packaging.

Avoid commits that mix unrelated concerns, such as Bungee scaler logic changes plus dependency packaging changes.

## 6.6 Handling CI failures

If CI fails for a commit:

1. Identify whether the failure is caused by the commit, infrastructure, or unrelated upstream breakage.
2. If caused by the commit, amend/fixup that commit and force-push the corrected branch.
3. If infrastructure/unrelated, document evidence and rerun; do not claim the commit is green until rerun passes or maintainers explicitly waive it.
4. If a platform cannot support Bungee yet, make that explicit in the commit itself (`BUNGEE=OFF` for that platform or guarded logic), not as a later fix.

## 6.7 Exit criteria

Before opening/marking the PR ready:

- `feature/bungee` contains all Bungee-related commits from current/existing branches;
- no unrelated local branch work is included;
- every commit has been locally validated;
- every commit has either a passing CI record or an explicit maintainer-approved exception;
- the final branch head has 100% passing required CI checks.

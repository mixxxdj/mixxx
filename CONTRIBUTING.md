# Mixxx Contribution Guidelines

Thank you for contributing to [Mixxx](https://mixxx.org/)! Your work helps DJs all over the world! We are global, all volunteer team that works by consensus and we are excited to have you join us. This document specifies technical aspects of our workflow. For social aspects please refer to [CODE_OF_CONDUCT.md](https://github.com/mixxxdj/mixxx/blob/main/CODE_OF_CONDUCT.md) in this repository. We encourage you to introduce yourself on our [Zulip chat](https://mixxx.zulipchat.com/) before starting to contribute code to Mixxx.

Table of Contents

1. [Orientation](#orientation)
   1. [Git Repositories](#git-repositories)
2. [Git Workflow](#git-workflow)
   1. [All Contributors](#all-contributors)
   2. [Core Team](#core-team)

## Orientation

We have lots more helpful information for users and developers on the [Mixxx wiki](https://github.com/mixxxdj/mixxx/wiki) and elsewhere, including [build instructions](https://github.com/mixxxdj/mixxx/wiki#compile-mixxx-from-source-code).

### Important Guidelines and Policies

* [Git Workflow](#git-workflow)
* [Coding Guidelines](https://github.com/mixxxdj/mixxx/wiki/Coding-Guidelines) & [Setting up `pre-commit`](#pre-commit)
* [Minimum Requirements Policy](https://github.com/mixxxdj/mixxx/wiki/Coding-Guidelines)
* [Internationalization Workflow](https://github.com/mixxxdj/mixxx/wiki/Internationalization)
* [Release Checklist](https://github.com/mixxxdj/mixxx/wiki/Release-Checklist-2.5.0)

### Git Repositories

This repository contains the Mixxx source code, skins, controller mappings, and some helpful scripts. We have a few other Git repositories too:

* [mixxxdj/website](https://github.com/mixxxdj/website): content for the main [mixxx.org](https://mixxx.org/) website which is generated with the [Pelican static site generator](https://getpelican.com/)
* [mixxxdj/manual](https://github.com/mixxxdj/manual): content for the Mixxx manual, which uses [Sphinx](https://www.sphinx-doc.org/)
* [mixxxdj/vcpkg](https://github.com/mixxxdj/vcpkg): dependencies packaged for macOS and Windows

All of these are automatically built and deployed by [GitHub Actions](https://github.com/features/actions) whenever a change is committed. You are welcome to open a pull request in any of these repositories.

## Git Workflow

### `pre-commit`

* Install [pre-commit](https://pre-commit.com/#install) to automatically ensure that your commits comply with our code style for both C++ and JavaScript. This saves time reviewing so we don't have to point out nitpicky style issues. Once you have pre-commit installed on your computer, set it up in your local Git repository:

      cd /path/to/your/git/repo
      pre-commit install
      pre-commit install -t pre-push

  If you have a problems with a particular hook, you can use the `SKIP` environment variable to disable hooks:

      SKIP=clang-format,end-of-file-fixer git commit

  This can also be used to separate logic changes and autoformatting into two subsequent commits. Using the SKIP environment variable is preferable to using `git commit --no-verify` (which also disables the checks) because it won't prevent catching other, unrelated issues.

### All Contributors

* Each feature/bug fix should be done on its own Git branch so they can be reviewed and merged independently. Refer to [Using Git](https://github.com/mixxxdj/mixxx/wiki/using-git) for how to do this. Please ask for help on [Zulip](https://mixxx.zulipchat.com/) if you have questions about using Git after reading that page.
* Commits should be as small as they can while still building. The smaller the commit, the easier it is to review. It also makes it easier to revert if it is later identified as the source of a bug. If you have lots of changes that you need to commit, a [GUI Git client](https://git-scm.com/downloads/guis) can be helpful for picking out specific changes for multiple small commits.
* Every commit should build. This is important so [git bisect](https://git-scm.com/book/en/v2/Git-Tools-Debugging-with-Git#_binary_search) works.
* Commit messages should succinctly describe what is changed in that commit and why. Lines should wrap at 72 characters so they show fully in GitHub and other Git tools. For example, this is a good commit message:

      DlgPrefEffects: add QListWidget to set order of chains

      This order will soon be used by new ControlObjects to load them
      from controllers.

  This is not a good commit message:

      address comments from PR review

  Neither is this:

      fix a bug with quantize while the deck is playing and sync leader is enabled and an effect unit is on the deck while the user is turning an EQ knob

  Refer to [How to Write a Git Commit Message](https://chris.beams.io/posts/git-commit/) for more details.

* Generally, prefer merging over rebasing. Do not rebase unless you have discussed that with whoever is reviewing the pull request. When you rebase a branch with an open pull request, it is no longer possible to distinguish your latest changes from already reviewed parts, resulting in unnecessary extra work for the reviewer. Comments made directly to a single commit will be lost. Rebased commits are likely not tested and there is a risk that building fails in a later `git bisect` run. If you want to correct minor mistakes with a rebase within a few minutes of pushing commits, that is okay as long as no one has started reviewing those commits yet. A `git commit --amend` is possible at any time as long the commit has the limited scope of one topic.
* If you plan to rebase your branch before merge to eliminate forth and back commits or such, you can commit them with the --fixup option or manually add "fixup!" as the first word in the commit message. This prevents mergeing to upstream before the rebase.
* If you are helping with someone else's pull request that is not yet merged, open a pull request targeted at their fork. Leave a comment on the upstream pull request (which targets mixxxdj/mixxx) with a link to your pull request so other Mixxx contributors are aware of your changes.
* Low risk bug fixes should be targeted at the stable branch (e.g., `2.3`). However, bug fixes for the stable branches must have a direct impact on users. If you spot a minor bug reading the code or only want to clean up the code, target that at the `main` or beta branch.
* Controller mappings should be targeted at the stable branch unless they use features that are new in the beta or `main` branch.
* If you are making changes to the GUI with a pull request, please post before and after screenshots of the changes.
* Please help review other people's pull requests. When others review your pull requests, please return the favor. The continued progress of Mixxx depends on all of us working together. Even if you are not familiar with the area of the code being changed in a pull request, you can be helpful by building the branch, verifying that it works as described, and commenting with feedback about the user experience design.
* If you demonstrate good coding skills, help review pull requests, contribute major features, and show a commitment to Mixxx over time, we may invite you to the core team.

### Core Team

Mixxx core team members are contributors who have write access to the [upstream mixxxdj repositories](https://github.com/mixxxdj/) on GitHub, access to the Jenkins web interface for the build servers, and access to the private Zulip stream for the core team.

* Enable [two-factor authentication (2FA)](https://help.github.com/en/github/authenticating-to-github/securing-your-account-with-two-factor-authentication-2fa) for your GitHub account.
* _Never_ force push to an upstream repository (mixxxdj). If you encounter an error from Git saying you would need to force push, stop what you are doing and discuss the situation on Zulip.
* Only push directly to an upstream repository (mixxxdj) for trivial, uncontroversial changes like fixing a typo.
* All non-trivial contributions should be made with a pull request, just like any other contributor who does not have write access. Do not merge your own pull requests.
* You may merge someone else's pull request as the only reviewer if no other contributors have expressed concerns about the changes or said they want to review the code. Please do not merge pull requests immediately; allow at least a day or two for others to comment. Remember we are all volunteers and cannot respond to everything immediately.
* If there is disagreement about changes in a pull request, do not merge it until a consensus has been reached.
* Check CI to ensure builds work and tests pass before merging. If CI timed out, either manually restart it or build the branch and run the tests locally before merging.
* When you merge a pull request to a stable branch, merge the stable branch to the beta branch afterwards. If you merge a pull request to a beta branch, merge the beta branch to `main` afterwards. When backporting, cherry-pick or rebase rather than merge.
* Merge PRs using a merge, to keep the original commits valid. Keep the default commit message "Merge pull request ..." with the reference to the pull request. In case where the PR contains broken (non-building) commits, back-and-forth commits or commits without a meaningful commit message that are not worth keeping, ask the author to squash the commits before merge. Alternatively you may ask the contributor to check "Allow edits and access to secrets by maintainers". Then you can squash locally or use the `/softfix` comment to squash remotely. See [Softfix](https://github.com/daschuer/softfix/?tab=readme-ov-file#softfix-a-pull-request)
* Default to open; only post in the private Zulip stream for discussions that have a reason to be private. Most of the time, post to a public Zulip stream so anyone can participate in the discussion.
* When Mixxx participates in Google Summer of Code, you may volunteer as a mentor if you like.

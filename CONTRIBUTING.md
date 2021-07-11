# Mixxx Contribution Guidelines #
Thank you for contributing to [Mixxx](https://mixxx.org/)! Your work helps DJs all over the world! We are a global team that works by consensus and we are excited to have you join us.

This document describes how our community has agreed to collaborate together effectively. Reading and following these guidelines shows other collaborators that you respect their time and effort. In return, they will do their best to address your issue and help you get your contributiuons merged.

All Mixxx contributors, including the core team members, are volunteers. This means that Mixxx is made possible by their passion and commitment to the project on their free time. This is why it's so important to treat everyone with respect and understanding, and be patient if things sometimes move a bit slower than you would like. Don't forget to read and follow our [Code Of Conduct](https://github.com/mixxxdj/mixxx/blob/master/CODE_OF_CONDUCT.md), as it provides a framework for everyone to feel accepted, respected and safe in all their interactions within the mixxx community.

## Table of Contents ##
1. [Orientation](#Orientation)
   1. [Git Repositories](#Git-Repositories)
2. [Git Workflow](#Git-Workflow)
   1. [All Contributors](#All-Contributors)
   2. [Core Team](#Core-Team)

## Getting in touch with the community ##

We use [Zulip chat](https://mixxx.zulipchat.com/) to discuss and coordinate everything related to Mixxx development (and more!).

- We encourage you to say hi on the [introduce yourself channel](https://mixxx.zulipchat.com/#narrow/stream/109123-introduce-yourself) and share what you would like to contribute.
- If you need help with your contribution, the Mixxx codebase or anything else related to Mixxx development don't hesitate to ask on the [development help channel](https://mixxx.zulipchat.com/#narrow/stream/247620-development-help).
- The [development channel](https://mixxx.zulipchat.com/#narrow/stream/109171-development) is where day to day discussion about code, new features and releases happens.

We also have lots of additional information for users and developers on the [Mixxx wiki](https://github.com/mixxxdj/mixxx/wiki), make sure to check it out if there's some piece of information you can't find.

## How can I contribute? ##

There are lots of ways you can contribute to Mixxx. Even if you don't know how to program you can help making Mixxx accessible to more users:

 - [Help translate Mixxx to other languages](https://github.com/mixxxdj/mixxx/wiki/Internationalization)
 - Improve the Mixxx [Manual](https://github.com/mixxxdj/manual).

If you have some coding skills, or you are willing to learn them you can do even more:

 - [Add native support for new hardware controllers](https://github.com/mixxxdj/mixxx/wiki/Contributing%20mappings)
 - Fix bugs or program new features

We welcome programmers of all backgrounds and skill levels, and our community will do their best to help you with any problems you encounter.

## Orientation ##
We have lots more helpful information for users and developers on the [Mixxx wiki](https://mixxx.org/wiki/doku.php/start), including [build instructions](https://mixxx.org/wiki/doku.php/start#compile_mixxx_from_source_code).

### Git Repositories ###
This repository contains the Mixxx source code, skins, controller mappings, and some helpful scripts. We have a few other Git repositories too:
* [mixxxdj/website](https://github.com/mixxxdj/website): content for the main [mixxx.org](https://mixxx.org/) website which is generated with the [Cactus static site generator](https://github.com/eudicots/Cactus)
* [mixxxdj/manual](https://github.com/mixxxdj/manual): content for the Mixxx manual, which uses [Sphinx](https://www.sphinx-doc.org/)
* [mixxxdj/buildserver](https://github.com/mixxxdj/buildserver): scripts for generating our prebuilt dependencies for macOS and Windows

All of these are automatically built and deployed by our [Jenkins build servers](https://builds.renegadetech.mixxx.org/) whenever a change is committed. You are welcome to open a pull request in any of these repositories.

## Git Workflow ##
### All Contributors ###
* Each feature/bug fix should be done on its own Git branch so they can be reviewed and merged independently. Refer to [Using Git](https://mixxx.org/wiki/doku.php/using_git) for how to do this. Please ask for help on [Zulip](https://mixxx.zulipchat.com/) if you have questions about using Git after reading that page.
* Commits should be as small as they can while still building. The smaller the commit, the easier it is to review. It also makes it easier to revert if it is later identified as the source of a bug. If you have lots of changes that you need to commit, a [GUI Git client](https://git-scm.com/downloads/guis) can be helpful for picking out specific changes for multiple small commits.
* Every commit should build. This is important so [git bisect](https://git-scm.com/book/en/v2/Git-Tools-Debugging-with-Git#_binary_search) works.
* Commit messages should succinctly describe what is changed in that commit and why. Lines should wrap at 72 characters so they show fully in GitHub and other Git tools. For example, this is a good commit message:

    ```
    DlgPrefEffects: add QListWidget to set order of chains

    This order will soon be used by new ControlObjects to load them
    from controllers.
    ```

    This is not a good commit message:

    ```
    address comments from PR review
    ```

    Neither is this:

    ```
    fix a bug with quantize while the deck is playing and master sync is enabled and an effect unit is on the deck while the user is turning an EQ knob
    ```

    Refer to [How to Write a Git Commit Message](https://chris.beams.io/posts/git-commit/) for more details.

* Install [pre-commit](https://pre-commit.com/#install) to automatically ensure that your commits comply with our code style for both C++ and JavaScript. Note this is currently not working on Windows. This saves time reviewing so we don't have to point out nitpicky style issues. Once you have pre-commit installed on your computer, set it up in your local Git repository:

    ```
    cd /path/to/your/git/repo
    pre-commit install
    pre-commit install -t pre-push
    ```

    If you have a problems with a particular hook, you can use the SKIP environment variable to disable hooks:

    ```
    SKIP=clang-format,end-of-file-fixer git commit
    ```

    This can also be used to separate logic changes and autoformatting into two subsequent commits. Using the SKIP environment variable is preferable to using `git commit --no-verify` (which also disables the checks) because it won't prevent catching other, unrelated issues.

* Generally, prefer merging to rebasing. Do not rebase unless you have discussed that with whoever is reviewing the pull request. When you rebase a branch with an open pull request, prior review comments made inline in the code on GitHub lose their connection to that spot in the code. If you want to correct minor mistakes with a rebase or `git commit --amend` within a few minutes of pushing commits, that is okay as long as no one has started reviewing those commits yet.
* If you are helping with someone else's pull request that is not yet merged, open a pull request targeted at their fork. Leave a comment on the upstream pull request (which targets mixxxdj/mixxx) with a link to your pull request so other Mixxx contributors are aware of your changes.
* Low risk bug fixes should be targeted at the stable branch (currently 2.2). However, bug fixes for the stable branches must have a direct impact on users. If you spot a minor bug reading the code or only want to clean up the code, target that at the master or beta branch.
* Controller mappings should be targeted at the stable branch unless they use features that are new in the beta or master branch.
* If you are making changes to the GUI with a pull request, please post before and after screenshots of the changes.
* Please help review other people's pull requests. When others review your pull requests, please return the favor. The continued progress of Mixxx depends on all of us working together. Even if you are not familiar with the area of the code being changed in a pull request, you can be helpful by building the branch, verifying that it works as described, and commenting with feedback about the user experience design.
* If you demonstrate good coding skills, help review pull requests, contribute major features, and show a commitment to Mixxx over time, we may invite you to the core team.

### Core Team ###
Mixxx core team members are contributors who have write access to the [upstream mixxxdj repositories](https://github.com/mixxxdj/) on GitHub, access to the Jenkins web interface for the build servers, and access to the private Zulip stream for the core team.

* Enable [two-factor authentication (2FA)](https://help.github.com/en/github/authenticating-to-github/securing-your-account-with-two-factor-authentication-2fa) for your GitHub account.
* _Never_ force push to an upstream repository (mixxxdj). If you encounter an error from Git saying you would need to force push, stop what you are doing and discuss the situation on Zulip.
* Only push directly to an upstream repository (mixxxdj) for trivial, uncontroversial changes like fixing a typo.
* All non-trivial contributions should be made with a pull request, just like any other contributor who does not have write access. Do not merge your own pull requests.
* You may merge someone else's pull request as the only reviewer if no other contributors have expressed concerns about the changes or said they want to review the code. Please do not merge pull requests immediately; allow at least a day or two for others to comment. Remember we are all volunteers and cannot respond to everything immediately.
* If there is disagreement about changes in a pull request, do not merge it until a consensus has been reached.
* Check CI to ensure builds work and tests pass before merging. If CI timed out, either manually restart it or build the branch and run the tests locally before merging.
* When you merge a pull request to a stable branch, merge the stable branch to the beta branch afterwards. If you merge a pull request to a beta branch, merge the beta branch to master afterwards. When backporting, cherry-pick or rebase rather than merge.
* Default to open; only post in the private Zulip stream for discussions that have a reason to be private. Most of the time, post to a public Zulip stream so anyone can participate in the discussion.
* When Mixxx participates in Google Summer of Code, you may volunteer as a mentor if you like.

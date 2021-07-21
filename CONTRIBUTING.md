# Mixxx Contribution Guidelines #
Thank you for contributing to [Mixxx](https://mixxx.org/)! Your work helps DJs all over the world! We are a global team that works by consensus and we are excited to have you join us.

All Mixxx contributors, including the core team members, are volunteers. This means that Mixxx is made possible by their passion and commitment to the project on their free time. This is why it's so important to treat everyone with respect and understanding, and be patient if things sometimes move a bit slower than you would like. Don't forget to read and follow our [Code Of Conduct](https://github.com/mixxxdj/mixxx/blob/master/CODE_OF_CONDUCT.md), as it provides a framework for everyone to feel accepted, respected and safe in all their interactions within the mixxx community.

## Table of Contents ##
- [Getting in touch with the community](#getting-in-touch-with-the-community)
- [How can I contribute?](#how-can-i-contribute)
- [Your first Mixxx code contribution](#Your-first-Mixxx-code-contribution)
   - [Compile Mixxx for the first time](#Compile-Mixxx-for-the-first-time)
   - [Pick an issue to work on](#Pick-an-issue-to-work-on)
   - [Time to code](#Time-to-code)
   - [Open a Pull Request](# Open-a-Pull-Request)
   - [How to become a 5-star contributor](#How-to-become-a-5-star-contributor)
- [Git Workflow](#Git-Workflow)
   - [All Contributors](#All-Contributors)
   - [Core Team](#Core-Team)

## Getting in touch with the community ##

We use [Zulip chat](https://mixxx.zulipchat.com/) to discuss and coordinate everything related to Mixxx development (and more!).

- We encourage you to say hi on the [introduce yourself channel](https://mixxx.zulipchat.com/#narrow/stream/109123-introduce-yourself) and share what you would like to contribute.
- If you need help with your contribution, the Mixxx codebase or anything else related to Mixxx development don't hesitate to ask on the [development help channel](https://mixxx.zulipchat.com/#narrow/stream/247620-development-help).
- The [development channel](https://mixxx.zulipchat.com/#narrow/stream/109171-development) is where day to day discussion about code, new features and releases happens.

We also have lots of additional information for users and developers on the [Mixxx wiki](https://github.com/mixxxdj/mixxx/wiki), make sure to check it out if there's some piece of information you can't find.

## How can I contribute? ##

There are lots of ways you can contribute to Mixxx. Even if you don't know how to program, there's plenty of things you can do to help:

 - [Report a bug](https://github.com/mixxxdj/mixxx/wiki/Reporting%20bugs)
 - [Help testing the latest developments](https://github.com/mixxxdj/mixxx/wiki/Testing)
 - [Help translate Mixxx to other languages](https://github.com/mixxxdj/mixxx/wiki/Internationalization)
 - [Improve the Mixxx Manual](https://github.com/mixxxdj/manual#readme)
 - [Improve the Mixxx website](https://github.com/mixxxdj/website)

If you have some coding skills, or you are willing to learn them you can do even more:

 - [Add native support for new hardware controllers](https://github.com/mixxxdj/mixxx/wiki/Contributing%20mappings)
 - [Fix bugs or program new features](#Your-first-Mixxx-code-contribution)

We welcome programmers of all backgrounds and skill levels, and our community will do their best to help you with any problems you encounter.

## Your first Mixxx code contribution ##

### Compile Mixxx for the first time ###
Your first goal is to get the Mixxx source code and compile it. The steps to do so depend on your operating system:
- [Compiling on Linux](https://github.com/mixxxdj/mixxx/wiki/Compiling%20on%20Linux)
- [Compiling on Windows](https://github.com/mixxxdj/mixxx/wiki/Compiling%20on%20Windows)
- [Compiling on macOS](https://github.com/mixxxdj/mixxx/wiki/Compiling%20on%20macOS)

### Pick an issue to work on ###
You might already have in mind that feature you always dreamed of! If that's not the case or you want to start with something easy, you can ask for ideas on [Zulip](https://mixxx.zulipchat.com/#narrow/stream/247620-development-help) or pick an [easy issue](https://bugs.launchpad.net/mixxx/+bugs?field.tag=easy).

### Time to code ###
Now it's time to write some code! 
We recommend to [set up an IDE](https://github.com/mixxxdj/mixxx/wiki/Developer%20Tools) to assit you in the code writing process.

We use [Git](https://git-scm.com/) for source control. Git is a distributed version control system that allows us to keep track of how our code changes and work together on it. If you are new to Git or source control in general, you can have a look at [Using Git](https://github.com/mixxxdj/mixxx/wiki/Using%20Git) to learn how.

If you have trouble with any of this remember that you can always ask on the [Zulip development help channel](https://mixxx.zulipchat.com/#narrow/stream/247620-development-help).

### Open a Pull Request ###
Once you finish coding, it's time to show your changes to other Mixxx developers. You do so by [opening a Pull Request](https://github.com/mixxxdj/mixxx/wiki/Using%20Git#open-a-pull-request).

### How to become a 5-star contributor ###
Now that you know your basics on how to contribute to Mixxx and engage in its community, it's time to learn a bit more our rules and guidelines.

These rules and guidelines describe how our community has agreed to collaborate together effectively. Reading and following these guidelines shows other collaborators that you respect their time and effort. In return, they will do their best to address your issue and help you get your contributiuons merged.

If you have trouble on how to follow any of this guidelines remember that you can always ask on the [Zulip development help channel](https://mixxx.zulipchat.com/#narrow/stream/247620-development-help).

#### Commits ####
* Every commit should build. This is important so [git bisect](https://git-scm.com/book/en/v2/Git-Tools-Debugging-with-Git#_binary_search) works.
* Commits should be as small as they can while still building. The smaller the commit, the easier it is to review. It also makes it easier to revert if it is later identified as the source of a bug. If you have lots of changes that you need to commit, a [GUI Git client](https://git-scm.com/downloads/guis) can be helpful for picking out specific changes for multiple small commits.
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

#### Branches & Pull Requests ####
* Each feature/bug fix should be done on its own Git branch so they can be reviewed and merged independently. Refer to [Using Git](https://mixxx.org/wiki/doku.php/using_git) to learn how to do this.
* Generally, prefer merging to rebasing. Do not rebase unless you have discussed that with whoever is reviewing the pull request. When you rebase a branch with an open pull request, prior review comments made inline in the code on GitHub lose their connection to that spot in the code. If you want to correct minor mistakes with a rebase or `git commit --amend` within a few minutes of pushing commits, that is okay as long as no one has started reviewing those commits yet.
* If you are helping with someone else's pull request that is not yet merged, open a pull request targeted at their fork. Leave a comment on the upstream pull request (which targets mixxxdj/mixxx) with a link to your pull request so other Mixxx contributors are aware of your changes.
* Low risk bug fixes should be targeted at the stable branch (currently 2.2). However, bug fixes for the stable branches must have a direct impact on users. If you spot a minor bug reading the code or only want to clean up the code, target that at the master or beta branch.
* Controller mappings should be targeted at the stable branch unless they use features that are new in the beta or master branch.
* If you are making changes to the GUI with a pull request, please post before and after screenshots of the changes.
* Please help review other people's pull requests. When others review your pull requests, please return the favor. The continued progress of Mixxx depends on all of us working together. Even if you are not familiar with the area of the code being changed in a pull request, you can be helpful by building the branch, verifying that it works as described, and commenting with feedback about the user experience design.

#### Core Team ####
Mixxx core team members are contributors who have write access to the [upstream mixxxdj repositories](https://github.com/mixxxdj/) on GitHub, access to the Jenkins web interface for the build servers, and access to the private Zulip stream for the core team.

If you demonstrate good coding skills, help review pull requests, contribute major features, and show a commitment to Mixxx over time, we may invite you to the core team.

Core Team members should follow the [Core Team Memebers Guidelines](https://github.com/mixxxdj/mixxx/wiki/Core-Team-Guidelines).

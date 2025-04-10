#!/usr/bin/env python3
import datetime
import os
import sys
import subprocess
import re

import markdown
import bs4
from lxml import etree


def is_in_merge():
    git_dir = (
        subprocess.check_output(
            (
                "git",
                "rev-parse",
                "--git-dir",
            )
        )
        .decode()
        .strip()
    )
    return os.path.exists(
        os.path.join(git_dir, "MERGE_MSG")
    ) and os.path.exists(os.path.join(git_dir, "MERGE_HEAD"))


def is_ammending():
    ppid = os.getppid()
    pppid = (
        subprocess.check_output(("ps", "-p", str(ppid), "-oppid="))
        .decode()
        .strip()
    )
    git_command = subprocess.check_output(("ps", "-p", pppid, "-ocommand="))
    return True if b"--amend" in git_command else False


def parse_changelog(content, development_release_date):
    for section in re.split("^## ", content.strip(), flags=re.MULTILINE)[1:]:
        version, sep, description_md = section.partition("\n")
        matchobj = re.match(
            r"\[(?P<number>\d\.\d\.\d)\]\([^\)]+\)"
            r"(?P<date>(?: \((?i:Unreleased|\d{4}-\d{2}-\d{2})\))?)",
            version,
        )
        if not matchobj:
            continue

        attrib = {
            "version": matchobj.group("number"),
        }
        try:
            release_date = datetime.datetime.strptime(
                matchobj.group("date"), " (%Y-%m-%d)"
            ).replace(tzinfo=datetime.timezone.utc)
        except ValueError:
            # Every release must have an associated timestamp, even development
            # releases. Otherwise, `appstreamcli validate` will complain.
            release_date = development_release_date
            attrib["type"] = "development"
        else:
            attrib["type"] = "stable"

        attrib["date"] = release_date.strftime("%Y-%m-%d")
        attrib["timestamp"] = "{:.0f}".format(release_date.timestamp())

        soup = bs4.BeautifulSoup(
            markdown.markdown(description_md), "html.parser"
        )

        # The description tag contains HTML-like markup, but only supports a
        # few select tags. Hence we need to strip some tags that are not
        # supported.
        #
        # See the specification for details:
        # https://freedesktop.org/software/appstream/docs/chap-Metadata.html#tag-description
        for tag in soup.find_all("a"):
            tag.replace_with(tag.get_text())
        for tag in soup.find_all("h3"):
            new_tag = soup.new_tag("p")
            new_tag.string = tag.get_text()
            tag.replace_with(new_tag)

        # Although the `<code>` tag is theoretically supported, it apparently
        # leads to parser errors when using `appstream-util validate-relax` (as
        # of version 0.7.18).
        for tag in soup.find_all("code"):
            tag.replace_with(tag.get_text())

        desc = soup.new_tag("description")
        desc.extend(soup.contents)

        release = etree.Element("release", attrib=attrib)
        release.append(etree.fromstring(desc.prettify()))
        yield release


def main(argv=None):
    rootpath = os.path.join(os.path.dirname(__file__), "..")
    metainfo_path = os.path.join(
        rootpath, "res/linux/org.mixxx.Mixxx.metainfo.xml"
    )
    parser = etree.XMLParser(remove_blank_text=True, remove_comments=False)
    tree = etree.parse(metainfo_path, parser)
    root = tree.getroot()
    releases = root.find("releases")
    releases.clear()
    changelog_path = os.path.join(rootpath, "CHANGELOG.md")

    # We need to define a date for development releases. We can't use
    # `datetime.datetime.now()` or something like that, because that would
    # involve updating the metainfo.xml during every single commit. The same
    # goes for using the latest commit date.
    #
    # As a compromise, we're using the date of the parent commit before the
    # changelog was updated.
    #
    # In case of staged changes we use HEAD, because will become the parent
    # commit, after committing. We need to skip merge commits, because we
    # cannot modify a commit during merging of a pull request on GitHub

    diff_result = subprocess.run(
        (
            "git",
            "diff",
            "--quiet",
            "--staged",
            "--",
            changelog_path,
        ),
        capture_output=True,
        text=True,
    )
    changelog_changes_staged = diff_result.returncode

    last_changelog_commit_first_parent = "HEAD"
    if is_in_merge():
        last_changelog_commit = (
            subprocess.check_output(
                (
                    "git",
                    "log",
                    "-1",
                    "--no-merges",
                    "--format=%H",
                    "HEAD",
                    "MERGE_HEAD",
                    "--",
                    changelog_path,
                )
            )
            .decode()
            .strip()
        )
        last_changelog_commit_first_parent = last_changelog_commit + "~1"
    elif not changelog_changes_staged or is_ammending():
        last_changelog_commit = (
            subprocess.check_output(
                (
                    "git",
                    "log",
                    "-1",
                    "--no-merges",
                    "--format=%H",
                    "--",
                    changelog_path,
                )
            )
            .decode()
            .strip()
        )
        last_changelog_commit_first_parent = last_changelog_commit + "~1"

    parent_changelog_change_date = datetime.datetime.fromisoformat(
        subprocess.check_output(
            (
                "git",
                "log",
                "-1",
                "--format=%aI",
                last_changelog_commit_first_parent,
            )
        )
        .decode()
        .strip()
    )

    with open(changelog_path, mode="r", encoding="utf-8") as fp:
        for release in parse_changelog(
            fp.read(), development_release_date=parent_changelog_change_date
        ):
            releases.append(release)
    tree.write(
        metainfo_path,
        encoding="UTF-8",
        xml_declaration=True,
        pretty_print=True,
    )


if __name__ == "__main__":
    sys.exit(main())

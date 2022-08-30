#!/usr/bin/env python3
import datetime
import os
import subprocess
import re

import markdown
import bs4
from lxml import etree


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
    # changelog was updated. The only feasible alternative is to remove
    # development releases completely, or remove the appstream metadata
    # from the repository and only generate it on demand when an actual
    # package is built.
    #
    # We need to exclude merge commits here, because the CI checks the latest
    # commit for pushes, but actually performs a merge when checking pull
    # requests. Therefore, the date would differ depending on the CI job reason
    # and cause issues when merging a stable release branch into the dev
    # branch.

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

    parent_changelog_change_date = datetime.datetime.fromisoformat(
        subprocess.check_output(
            (
                "git",
                "log",
                "-1",
                "--no-merges",
                "--format=%cI",
                last_changelog_commit + "~1",
            )
        )
        .decode()
        .strip()
    )

    with open(changelog_path, mode="r") as fp:
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
    main()

#!/usr/bin/env python3
"""
Complementary tool to json2github.py

When bulk-transfering issues between repositories using githubs
internal bulk-transfer tools. The generate CSV file containing
a mapping from source to destination URL.
This tool updates the state serialized state of json2github.py
(specified via the json2github --output-file flag) from the
pre-transfer issue numbers to the new issue numbers, specified
by the mapping file we received from github.

This tool only maps issue ID's and assumes all the source issues
and all the destination issues belong to the same repo.

According to people from github, the format they use is a CSV
table with the source URL in the 0th column and the destination
URL in the 1st column

"""

import argparse
import urllib.parse
import csv
import json

import sys


def eprint(str):
    print(str, file=sys.stderr)


def issue_nr_from_url(url):
    path_segments = urllib.parse.urlparse(url).path.split("/")
    assert path_segments[-2].startswith("issue")
    return int(path_segments[-1])


def make_mapping(id_mappings_csv):
    csv_reader = csv.DictReader(
        id_mappings_csv, fieldnames=("source", "destination")
    )
    return {
        issue_nr_from_url(line["source"]): issue_nr_from_url(
            line["destination"]
        )
        for line in csv_reader
    }


def fixup_ids(lp_issues, mapping):
    for issue in lp_issues:
        num = issue.get("gh_issue_number")
        if num is None:
            eprint(
                f"issue {issue['id']} has not been "
                f"imported to github, skipping!"
            )
            continue
        issue["gh_issue_number"] = mapping[num]


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--mapping_file", type=argparse.FileType("r"), required=True
    )
    parser.add_argument(
        "--lp_issues_file", type=argparse.FileType("r"), required=True
    )
    parser.add_argument(
        "--output_file", type=argparse.FileType("w"), required=True
    )
    args = parser.parse_args(argv)

    mapping_file = args.mapping_file
    lp_issues = json.load(args.lp_issues_file)

    mapping = make_mapping(mapping_file)

    fixup_ids(lp_issues, mapping)

    json.dump(lp_issues, args.output_file)


if __name__ == "__main__":
    main()

# /usr/bin/env python3

import argparse
import json
import logging
from launchpadlib.launchpad import Launchpad


def gh_issue_to_commenttext(issue):
    return (
        f"Mixxx now uses GitHub for bug tracking. "
        f"This bug has been migrated to: \n"
        f"https://github.com/mixxxdj/mixxx/issues/{issue}"
    )


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("bugs_file", type=argparse.FileType("r"))
    parser.add_argument("--output-file")
    args = parser.parse_args(argv)

    logging.basicConfig(
        format="%(asctime)s %(levelname)-8s %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
        level=logging.INFO,
    )

    lp_issues = {x["id"]: x for x in json.load(args.bugs_file)}

    # This will open up a web-browser to do OAuth authentication, so
    # you need a GUI and webbrowser so this authentication works.
    launchpad = Launchpad.login_with(
        "Mixxx Issue Migration Backlinks",
        "production",  # change me to 'production'/'staging' when ready
        version="devel",
    )
    try:
        for i, issuedata in enumerate(
            sorted(
                lp_issues.values(), key=lambda x: (x["date_created"], x["id"])
            ),
            1,
        ):
            if issuedata.get("lp_backlink_imported"):
                continue

            lp_issue = launchpad.bugs[issuedata["id"]]
            print(lp_issue)
            msg_content = gh_issue_to_commenttext(issuedata["gh_issue_number"])
            lp_issue.newMessage(content=msg_content, send_notifications=False)
            lp_issue.lock(status="Comment-only")
            issuedata["lp_backlink_imported"] = True
    finally:
        if args.output_file:
            with open(args.output_file, mode="w") as f:
                json.dump(list(lp_issues.values()), f)


if __name__ == "__main__":
    main()

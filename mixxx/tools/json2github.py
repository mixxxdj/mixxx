#!/usr/bin/env python3
"""
Alternative lp2gh JSON Importer for Mixxx issue migration

You can install this like this:

    $ python3 -m venv venv
    $ source venv/bin/activate
    $ pip install PyGithub

To use it, you first need to obtain a bugs and milestone json file from lp2gh:

    https://github.com/Swiftb0y/lp2gh/blob/mixxx-import-fork/docs/moving_issues.rst#exporting-your-bugs
    https://github.com/Swiftb0y/lp2gh/blob/mixxx-import-fork/docs/moving_issues.rst#exporting-your-milestones

Its important you are using my Fork with the `mixxx-import-fork` branch as I
made some slight modifications to add features.

You can use it like this:

    $ python json2github.py --repo Holzhaus/mixxx-gh-issue-migration \
      --token mytoken --output-file=mixxx_bugs.json \
      mixxx_bugs.json mixxx_milestones.json

"""
import argparse
import binascii
import datetime
import json
import logging
import random
import re
import textwrap
import time
import github
import requests

LAUNCHPAD_STATUS_MAP = {
    "Confirmed": ["confirmed"],
    "Fix Committed": [],
    "Fix Released": [],
    "Incomplete": ["incomplete"],
    "In Progress": [],
    "Invalid": ["invalid"],
    "New": [],
    "Triaged": ["confirmed"],
    "Won't Fix": ["wontfix"],
    "Opinion": ["opinion"],
    "Expired": ["expired"],
}

LAUNCHPAD_IMPORTANCE_MAP = {
    "Critical": ["party stopper", "bug"],
    "High": ["bug"],
    "Low": ["bug"],
    "Medium": ["bug"],
    "Undecided": [],
    "Wishlist": ["feature"],
}

LAUNCHPAD_USER_MAP = {
    "aart": "amvanbaren",
    "alex-jercaianu": "jercaianu",
    "alex.barker": "kwhat",
    # (not sure, email on launchpad matches
    # with domain linked on github profile)
    "another-rob": "borfo",
    "arbeit-u": "dg3nec",
    "arli0715": "dodler",
    "badescunicu": "badescunicu",
    "be.ing": "Be-ing",
    "bencoder": "bencoder",
    "bkgood": "bkgood",
    "bruno-buccolo": "buccolo",
    "buzz-dee": "BuZZ-dEE",
    "cardinot": "cardinot",
    "chloe-avrillon": "DJChloe",
    "crislacerda": "crislacerda",
    "daniel-64studio": "danielhjames",
    "daschuer": "daschuer",
    "default-kramer": "default-kramer",
    "dj-kaza": "kazakore",
    "eradkoff12": "radkoff",
    "ewanuno": "ewanuno",  # (not sure)
    "federicobriata": "federicobriata",
    "ferranpujol": "ferranpujolcamins",
    "foss4": "foss-",
    "frank-breitling": "fkbreitl",
    "gamegod": "asantoni",
    "gmsoft": "gmsoft-tuxicoman",
    "goddisignz": "goddisignz",
    "hacksdump": "hacksdump",
    "hile": "hile",
    "holthuis-jan": "Holzhaus",
    "htown202": "htown101",
    "iamcodemaker": "iamcodemaker",
    "ironstorm-gmail": "deftdawg",
    "jatwill": "idcmp",
    "jean-claveau-g": "jclaveau",
    "jessboerner": "doodlebro",
    "jmigual": "jmigual",
    "joerg-ubuntu": "JoergAtGithub",
    "johan-lasperas": "johanLsp",
    "josepma": "JosepMaJAZ",
    "juha-pitkanen": "JuhaPit",
    "jus": "esbrandt",
    "kabelfrickler": "snue",
    "kek001": "kek001",
    "ketanlambat": "ketan-lambat",
    "kevin-m-wern": "kevinwern",
    "khyew": "khyew",
    "kousu": "kousu",  # not sure, but first name seems to match
    "kshitij98": "kshitij98",
    "launchpad-net-poelzi": "poelzi",
    "lbot": "leematos",
    "leigh123linux-j": "leigh123linux",
    "lindybalboa": "LindyBalboa",
    "marczis": "marczis",
    "markusb": "markusb",
    "max-linke": "kain88-de",
    "mdizer": "mdizer",
    "melgrubb": "MelGrubb",
    "metastableb": "metastableB",
    "mevsme": "mevsme",
    "mhaulo": "mhaulo",
    "michael-z-freeman": "Michael-Z-Freeman",
    "mxmilkiib": "mxmilkiib",
    "nachtigall": "jenszo",
    "naught101": "naught101",
    "nik-martin": "nikmartin",
    "nimitbhardwaj": "nimitbhardwaj",
    "ninomp": "ninomp",
    "nm2107": "nm2107",
    "nopeppermint": "nopeppermint",
    "nschloe": "nschloe",
    "pasanen-tuukka": "illuusio",
    "pegasus-renegadetech": "Pegasus-RPG",
    "pestrela": "pestrela",
    "poelzi": "poelzi",
    "pwhelan": "pwhelan",
    "quentinfaidide": "QuentinFAIDIDE",
    "raulbehl": "raulbehl",
    "rawrr": "rawrr",
    "ronso0": "ronso0",
    "rryan": "rryan",
    "sblaisot": "sblaisot",
    "smashuu": "smashuu",
    "stephane-guillou": "stragu",
    "swiftb0y": "Swiftb0y",
    "toszlanyi": "toszlanyi",
    "troyane3": "troyane",
    "uklotzde": "uklotzde",
    "ulatekh": "ulatekh",
    "vlada-dudr": "vlada-dudr",
    "vrince": "vrince",
    "wzssyqa": "wzssyqa",
    "xerus2000": "xeruf",
    "xor29a": "xorik",
    "ywwg": "ywwg",
    "zezic": "zezic",
}

# Github has restrictions on assignees
# Since this script was made to import to a private repo and transfer over
# afterwards, most assignees would be invalid and so we won't use them.
GITHUB_ALLOWED_ASSIGNEES = {
    # "Holzhaus",
    # "asantoni",
    # "Be-ing",
    # "daschuer",
    # "esbrandt",
    # "Pegasus-RPG",
    # "ronso0",
    # "rryan",
    # "sblaisot",
    # "Swiftb0y",
    # "uklotzde",
    # "ywwg",
}

LABELS = {
    "aac": None,
    "accessibility": None,
    "analyzer": None,
    "autodj": None,
    "auxiliary": None,
    "beatgrid": None,
    "bpm": None,
    "broadcast": None,
    "browse": None,
    "build": None,
    "cloud": None,
    "cmake": None,
    "controllers": None,
    "coverart": None,
    "crash": None,
    "cue": None,
    "effects": None,
    "engine": None,
    "eq": None,
    "follower": None,
    "gui": None,
    "hid": None,
    "i18n": None,
    "installer": None,
    "itunes": None,
    "jack": None,
    "key": None,
    "keyboard": None,
    "library": None,
    "looping": None,
    "lyrics": None,
    "m4a": None,
    "manual": None,
    "metadata": None,
    "microphone": None,
    "midi": None,
    "mp3": None,
    "overview": None,
    "packaging": None,
    "passthrough": None,
    "performance": None,
    "playlist": None,
    "polish": None,
    "portaudio": None,
    "preferences": None,
    "qt5": None,
    "quantize": None,
    "recording": None,
    "rekordbox": None,
    "sampler": None,
    "scanner": None,
    "serato": None,
    "skin": None,
    "slip": None,
    "soundio": None,
    "soundsource": None,
    "standards": None,
    "sync": None,
    "tooltip": None,
    "touchscreen": None,
    "transport": None,
    "usability": None,
    "vinylcontrol": None,
    "waveform": None,
    # Importance (red)
    "bug": "ff0000",
    "security": "ff4400",
    "regression": "ef233c",
    "party stopper": "7b0d1e",
    "feature": "bd2d87",
    # status (yellow-ish)
    "incomplete": "fff45c",
    "invalid": "f9e900",
    "confirmed": "faa916",
    "wontfix": "ccbe00",
    "duplicate": "8f8500",
    "opinion": "9e9517",
    "expired": "b5bf00",
    # target system (blue-ish)
    "windows": "1929b3",
    "linux": "3b4be3",
    "macos": "121d7d",
    "raspberry": "5e6ce8",
    # size (light green)
    "easy": "68fa61",
    "weekend": "8efb88",
    "hackathon": "c7fdc4",
}

EXP_BACKUP_EXPONENT = 2


def format_text(text):
    parts = text.split("\n\n")
    for i, part in enumerate(parts):
        preformatted = True
        if "\n" in part.strip("\n") and all(
            line.startswith(">") for line in part.strip("\n").splitlines()[1:]
        ):
            preformatted = False
        if (
            "\n" not in part.strip()
            and part.strip().lower().startswith("on ")
            and part.strip().lower().endswith("wrote:")
        ):
            preformatted = False
        if not any(
            c in part
            for c in (
                "<",
                "\\",
                "$",
                ";",
                "|",
                "{",
                "}",
                " ?? ",
                "]:",
                "(gdb) bt",
            )
        ):
            preformatted = False

        if preformatted:
            parts[i] = textwrap.indent(part, prefix="    ")
        else:
            # Prevent unintended GH issue links
            parts[i] = re.sub(r"#(\d+)", r"#&#x2060;\1", part)
    return "\n\n".join(parts)


class LaunchpadImporter:
    def __init__(self, token, repo, milestonedata, mention):
        self.logger = logging.getLogger(__name__)
        self.gh = github.Github(login_or_token=token)
        self.repo = self.gh.get_repo(repo)
        self.gh_labels = {
            label.name: label for label in self.repo.get_labels()
        }
        self.gh_milestones = {
            milestone.title: milestone
            for milestone in self.repo.get_milestones(state="all")
        }
        self.lp_milestones = {x["name"]: x for x in milestonedata}
        self.mention = mention

    def get_user(self, username):
        try:
            username = LAUNCHPAD_USER_MAP[username]
            if self.mention:
                username = "@" + username
            else:
                # link to profile instead of mention
                # to avoid notification/email spam
                username = f"[{username}](https://github.com/{username})"
        except KeyError:
            username = f"[{username}](https://launchpad.net/~{username})"
        return username

    def get_label(self, name):
        try:
            label = self.gh_labels[name]
        except KeyError:
            label = self.import_label(name)
        return label

    def format_body(self, issuedata):
        header = [
            f"Reported by: **{self.get_user(issuedata['owner'])}**",
            f"Date: {issuedata['date_created']}",
            f"Status: {issuedata['status']}",
            f"Importance: {issuedata['importance']}",
            f"Launchpad Issue: [lp{issuedata['id']}]({issuedata['lp_url']})",
        ]
        if issuedata["tags"]:
            header.append("Tags: %s" % ", ".join(issuedata["tags"]))
        if issuedata["attachments"]:
            url_strings = [
                f"[{data['title']}]({data['url']})"
                for data in issuedata["attachments"]
            ]
            header.append("Attachments: %s" % ", ".join(url_strings))

        formatted_text = format_text(issuedata["description"])
        if len(formatted_text.strip()) > 0:
            formatted_text = "\n\n---\n\n" + formatted_text

        return "\n".join(header) + formatted_text

    def format_comment(self, commentdata):
        header = [
            f"Commented by: **{self.get_user(commentdata['owner'])}**",
            f"Date: {commentdata['date_created']}",
        ]
        if commentdata["attachments"]:
            url_strings = [
                f"[{data['title']}]({data['url']})"
                for data in commentdata["attachments"]
            ]
            header.append("Attachments: %s" % ", ".join(url_strings))
        formatted_text = format_text(commentdata["content"])
        if len(formatted_text.strip()) > 0:
            formatted_text = "\n\n---\n\n" + formatted_text
        return "\n".join(header) + formatted_text

    def name_to_milestone(self, milestone_name):
        try:
            milestone = self.gh_milestones[milestone_name]
        except KeyError:
            milestonedata = self.lp_milestones.get(
                milestone_name,
                {
                    "active": True,
                    "date_targeted": None,
                    "name": milestone_name,
                    "summary": "",
                },
            )
            milestone = self.import_milestone(milestonedata)
        return milestone

    def handle_ratelimit(self, func):
        abuse_timeout = 30

        def sleep_exp_backoff():
            nonlocal abuse_timeout
            time.sleep(abuse_timeout)
            abuse_timeout *= EXP_BACKUP_EXPONENT

        while True:
            try:
                return func()
            except github.RateLimitExceededException:
                rate_limit_resettime = datetime.datetime.utcfromtimestamp(
                    self.gh.rate_limiting_resettime
                )
                self.logger.warning(
                    "Rate limit exceeded (%d left/%d total), waiting until %s",
                    *self.gh.rate_limiting,
                    rate_limit_resettime.isoformat(),
                )
                time_to_wait = (
                    rate_limit_resettime - datetime.datetime.utcnow()
                )
                if time_to_wait.total_seconds() <= 0:
                    self.logger.warning(
                        "Failed to detect wait time, assuming 10 seconds..."
                    )
                    time_to_wait = datetime.timedelta(seconds=10)
                self.logger.warning(f"Sleeping for {time_to_wait}")
                time.sleep(time_to_wait.total_seconds())
            except github.GithubException as e:
                if e.status == 403 and "abuse" in e.data.get("message", ""):
                    self.logger.warning(
                        "Triggered abuse detection, sleeping %d seconds...",
                        abuse_timeout,
                    )
                    sleep_exp_backoff()
                elif e.status == 403 and "secondary rate limit" in e.data.get(
                    "message", ""
                ):
                    abuse_timeout = e.headers.get("Retry-After", abuse_timeout)
                    self.logger.warning(
                        "Triggered secondary rate limit, "
                        "sleeping %d seconds...",
                        abuse_timeout,
                    )
                    sleep_exp_backoff()
                elif e.status in range(500, 600):
                    self.logger.warning(
                        f"Internal server Error, sleeping {abuse_timeout} "
                        f"seconds and retrying"
                    )
                    self.logger.warning(e)
                    sleep_exp_backoff()
                else:
                    raise
            except requests.exceptions.ConnectionError as e:
                self.logger.warning(e)
                self.logger.warning(
                    "encountered connection error, retrying..."
                )
                sleep_exp_backoff()
            else:
                break

    def import_milestone(self, milestonedata):
        state = "open" if milestonedata["active"] else "closed"
        due_on = github.GithubObject.NotSet
        if milestonedata["date_targeted"]:
            due_on = datetime.datetime.strptime(
                milestonedata["date_targeted"], "%Y-%m-%dT%H:%M:%SZ"
            )
        description = github.GithubObject.NotSet
        if milestonedata["summary"]:
            description = milestonedata["summary"]
        milestone = self.handle_ratelimit(
            lambda: self.repo.create_milestone(
                milestonedata["name"], state, description, due_on
            )
        )
        self.logger.info("Created milestone: %r", milestone)
        self.gh_milestones[milestone.title] = milestone
        return milestone

    def status_to_label(self, status):
        for label_name in LAUNCHPAD_STATUS_MAP.get(status, []):
            yield self.get_label(label_name)

    def importance_to_label(self, importance):
        for label_name in LAUNCHPAD_IMPORTANCE_MAP.get(importance, []):
            yield self.get_label(label_name)

    def import_label(self, label_name):
        color = LABELS.get(label_name, None)
        if not color:
            color = "{c}{c}{c}".format(
                c=binascii.hexlify(random.randbytes(1)).decode()
            )
        label = self.handle_ratelimit(
            lambda: self.repo.create_label(label_name, color)
        )
        self.logger.info("Created label: %r", label)
        self.gh_labels[label_name] = label
        return label

    def name_to_assignee(self, name):
        username = LAUNCHPAD_USER_MAP.get(name)
        if username and username in GITHUB_ALLOWED_ASSIGNEES:
            return username
        else:
            return github.GithubObject.NotSet

    def import_issue(self, issuedata):
        milestone = github.GithubObject.NotSet
        if issuedata["milestone"]:
            milestone = self.name_to_milestone(issuedata["milestone"])
        labels = []
        labels.extend(self.status_to_label(issuedata["status"]))
        labels.extend(self.importance_to_label(issuedata["importance"]))
        if issuedata["duplicate_of"]:
            labels.append(self.get_label("duplicate"))
        if issuedata["security_related"]:
            labels.append(self.get_label("security"))
        if issuedata["tags"]:
            labels.extend(
                [
                    self.get_label(tag)
                    for tag in issuedata["tags"]
                    if tag in LABELS
                ]
            )
        assignee = self.name_to_assignee(issuedata["assignee"])
        # todo upload attachments here
        issue = self.handle_ratelimit(
            lambda: self.repo.create_issue(
                title=issuedata["title"],
                body=self.format_body(issuedata),
                milestone=milestone,
                assignee=assignee,
                labels=labels,
            )
        )
        self.logger.info("Created issue: %r", issue)
        return issue

    def import_issuecomment(self, issue, commentdata):
        comment = self.handle_ratelimit(
            lambda: issue.create_comment(self.format_comment(commentdata))
        )
        self.logger.info("Created issue comment: %r", comment)
        return comment

    def run_import(self, lp_issues, lp_milestones):
        num_issues = len(lp_issues.values())
        for i, issuedata in enumerate(
            sorted(
                lp_issues.values(), key=lambda x: (x["date_created"], x["id"])
            ),
            1,
        ):
            gh_issue_number = issuedata.get("gh_issue_number")
            if gh_issue_number:
                issue_milestone = issuedata["milestone"]

                if (
                    issuedata.get("gh_comments_imported")
                    == len(issuedata["comments"])
                    and issuedata.get("gh_status_comment_imported", False)
                    and issue_milestone is None
                ):
                    continue

                self.logger.info(
                    f"checking if issue #{gh_issue_number} requires fixup"
                )

                issue = self.handle_ratelimit(
                    lambda: self.repo.get_issue(gh_issue_number)
                )
                # fixup milestone ownership in case it got lost
                # (happens during bulk issue transfer between repos using
                # githubs internal tools)
                if issue_milestone is not None:
                    milestone = self.name_to_milestone(issue_milestone)
                    if issue.milestone != milestone:
                        self.logger.info(
                            f"fixing up milestone "
                            f'"{issue_milestone}" for '
                            f"issue #{gh_issue_number}"
                        )
                        # issue on launchpad is attached to milestone
                        # but corresponding pre-existing issue on gh does not
                        # have the milestone. Attach it here.
                        self.handle_ratelimit(
                            lambda: issue.edit(milestone=milestone)
                        )
            else:
                issue = self.import_issue(issuedata)
                lp_issues[issuedata["id"]]["gh_issue_number"] = issue.number

            comments_imported = issuedata.get("gh_comments_imported", 0)
            lp_issues[issuedata["id"]][
                "gh_comments_imported"
            ] = comments_imported

            comments = issuedata["comments"][comments_imported:]
            for comment in comments:
                comment = self.import_issuecomment(issue, comment)
                lp_issues[issuedata["id"]]["gh_comments_imported"] += 1

            if issuedata.get("gh_status_comment_imported", False):
                continue

            if issuedata["status"] in (
                "Fix Released",
                "Fix Committed",
                "Invalid",
                "Won't Fix",
                "Expired",
                "Incomplete",
            ):
                comment = (
                    f'Issue closed with status **{issuedata["status"]}**.'
                )
                self.handle_ratelimit(lambda: issue.create_comment(comment))
                if issue.state != "closed":
                    self.handle_ratelimit(lambda: issue.edit(state="closed"))
            lp_issues[issuedata["id"]]["gh_status_comment_imported"] = True
            self.logger.info(
                f"Imported {i}/{num_issues} "
                f"({'{:.2f}'.format((i / num_issues) * 100)}%)"
            )

        for issuedata in sorted(
            lp_issues.values(), key=lambda x: (x["date_created"], x["id"])
        ):
            if issuedata.get("gh_duplicate_comment_imported", False):
                continue

            duplicate_of = issuedata.get("duplicate_of")

            if duplicate_of is not None:
                duplicate_issue_number = lp_issues[issuedata["id"]][
                    "gh_issue_number"
                ]

                original = lp_issues.get(duplicate_of)
                if original is None:
                    # Bug this is marked as a duplicate of does not
                    # belong to mixxxx
                    # so link to the original on launchpad
                    original = (
                        f"[lp:{duplicate_of}]"
                        f"(https://bugs.launchpad.net/bugs/{duplicate_of})"
                    )
                else:
                    original = f"#{original['gh_issue_number']}"
                self.logger.info(
                    f"Marking #{duplicate_issue_number} "
                    f"as duplicate of {original}"
                )

                issue = self.handle_ratelimit(
                    lambda: self.repo.get_issue(duplicate_issue_number)
                )
                comment = f"Duplicate of {original}"
                self.handle_ratelimit(lambda: issue.create_comment(comment))
                if issue.state != "closed":
                    self.handle_ratelimit(lambda: issue.edit(state="closed"))
            lp_issues[issuedata["id"]]["gh_duplicate_comment_imported"] = True

        self.logger.info(f"Successfully imported {num_issues} issues")


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", required=True)
    parser.add_argument("--token", required=True)
    parser.add_argument("--output-file")
    parser.add_argument(
        "--mention", default=False, action=argparse.BooleanOptionalAction
    )
    parser.add_argument("bugs_file", type=argparse.FileType("r"))
    parser.add_argument("milestone_file", type=argparse.FileType("r"))
    args = parser.parse_args(argv)

    logging.basicConfig(
        format="%(asctime)s %(levelname)-8s %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
        level=logging.INFO,
    )

    lp_milestones = json.load(args.milestone_file)
    lp_issues = {x["id"]: x for x in json.load(args.bugs_file)}
    importer = LaunchpadImporter(
        args.token, args.repo, lp_milestones, args.mention
    )
    import_start = datetime.datetime.utcnow()
    try:
        importer.run_import(lp_issues, lp_milestones)
    finally:
        logging.info(
            f"Import took {datetime.datetime.utcnow() - import_start}"
        )
        if args.output_file:
            with open(args.output_file, mode="w") as f:
                json.dump(list(lp_issues.values()), f)


if __name__ == "__main__":
    main()

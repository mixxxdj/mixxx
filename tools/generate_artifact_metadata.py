#!/usr/bin/env python3
import argparse
import datetime
import functools
import glob
import hashlib
import json
import os
import posixpath
import subprocess
import urllib.parse


def sha256(file_path):
    with open(file_path, mode="rb") as fp:
        read_chunk = functools.partial(fp.read, 1024)
        m = hashlib.sha256()
        for data in iter(read_chunk, b""):
            m.update(data)
        return m.hexdigest()


def generate_metadata(file_path):
    file_stat = os.stat(file_path)
    file_sha256 = sha256(file_path)
    file_name = os.path.basename(file_path)

    commit_id = os.environ["GITHUB_SHA"]
    github_run_id = os.environ["GITHUB_RUN_ID"]
    github_server_url = os.environ["GITHUB_SERVER_URL"]
    github_repository = os.environ["GITHUB_REPOSITORY"]
    baseurl = os.environ["DEPLOY_BASEURL"]

    git_branch = (
        subprocess.check_output(("git", "rev-parse", "--abbrev-ref", "HEAD"))
        .decode()
        .strip()
    )
    os_image = os.environ["OS_IMAGE"]
    os_name = os.environ["OS"]
    destpath = os.environ["DEPLOY_DESTPATH"].format(
        git_branch=git_branch,
        os_name=os_name,
        os_image=os_image,
    )

    return {
        "commit_id": commit_id,
        "commit_url": (
            f"{github_server_url}/{github_repository}/commit/{commit_id}"
        ),
        "build_log_url": (
            f"{github_server_url}/{github_repository}/actions/"
            f"runs/{github_run_id}"
        ),
        "file_url": f"{baseurl}/{destpath}/{file_name}",
        "file_size": file_stat.st_size,
        "file_date": datetime.datetime.fromtimestamp(
            file_stat.st_ctime
        ).isoformat(),
        "file_sha256": file_sha256,
    }


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("file")
    args = parser.parse_args(argv)

    artifact_paths = glob.glob(args.file)
    assert len(artifact_paths) == 1
    file_path = artifact_paths[0]

    artifact_slug = os.environ["ARTIFACT_SLUG"]
    metadata = generate_metadata(file_path)
    print(json.dumps(metadata, indent=2, sort_keys=True))
    print(
        "::set-output name=artifact-{}::{}".format(
            artifact_slug, json.dumps(metadata)
        )
    )

    if os.getenv("CI") == "true":
        url = urllib.parse.urlparse(metadata["file_url"])
        deploy_dir = posixpath.dirname(url.path)
        with open(os.environ["GITHUB_ENV"], mode="a") as fp:
            fp.write(f"DEPLOY_DIR={deploy_dir}\n")


if __name__ == "__main__":
    main()

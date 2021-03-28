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
import urllib.request


def url_fetch(url, headers=None, **kwargs):
    request_headers = {
        "User-Agent": (
            "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:86.0) Gecko/20100101 "
            "Firefox/86.0"
        ),
    }
    if headers:
        request_headers.update(headers)
    req = urllib.request.Request(url, headers=request_headers, **kwargs)
    return urllib.request.urlopen(req, timeout=10)


def url_exists(url):
    try:
        resp = url_fetch(url, method="HEAD")
    except IOError:
        return False
    return resp.status == 200


def url_download_json(url):
    try:
        resp = url_fetch(url)
        manifest_data = resp.read().decode()
    except IOError:
        return None

    return json.loads(manifest_data)


def sha256(file_path):
    with open(file_path, mode="rb") as fp:
        read_chunk = functools.partial(fp.read, 1024)
        m = hashlib.sha256()
        for data in iter(read_chunk, b""):
            m.update(data)
        return m.hexdigest()


def find_git_branch(path="."):
    return subprocess.check_output(
        ("git", "rev-parse", "--abbrev-ref", "HEAD"),
        cwd=path,
        encoding="utf-8",
    ).strip()


def generate_file_metadata(file_path, destdir):
    file_stat = os.stat(file_path)
    file_sha256 = sha256(file_path)
    file_name = os.path.basename(file_path)

    commit_id = os.environ["GITHUB_SHA"]
    github_run_id = os.environ["GITHUB_RUN_ID"]
    github_server_url = os.environ["GITHUB_SERVER_URL"]
    github_repository = os.environ["GITHUB_REPOSITORY"]
    baseurl = os.environ["DEPLOY_BASEURL"]

    return {
        "commit_id": commit_id,
        "commit_url": (
            f"{github_server_url}/{github_repository}/commit/{commit_id}"
        ),
        "build_log_url": (
            f"{github_server_url}/{github_repository}/actions/"
            f"runs/{github_run_id}"
        ),
        "file_url": f"{baseurl}/{destdir}/{file_name}",
        "file_size": file_stat.st_size,
        "file_date": datetime.datetime.fromtimestamp(
            file_stat.st_ctime
        ).isoformat(),
        "file_sha256": file_sha256,
    }


def collect_manifest_data(job_data):
    job_result = job_data["result"]
    print(f"Build job result: {job_result}")
    assert job_result == "success"

    manifest_data = {}
    for output_name, output_data in job_data["outputs"].items():
        prefix, _, slug = output_name.partition("-")
        if prefix != "artifact" or not slug:
            print(f"Ignoring output '{output_name}'...")
            continue
        artifact_data = json.loads(output_data)

        url = artifact_data["file_url"]

        resp = url_fetch(url, method="HEAD")
        if not resp.status == 200:
            raise LookupError(f"Unable to find URL '{url}' on remove server")

        manifest_data[slug] = artifact_data

    return manifest_data


def main(argv=None):
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()
    artifact_parser = subparsers.add_parser(
        "artifact", help="Generate artifact metadata from file"
    )
    artifact_parser.add_argument("file")
    artifact_parser.add_argument("slug")
    artifact_parser.set_defaults(cmd="artifact")

    manifest_parser = subparsers.add_parser(
        "manifest",
        help="Collect artifact metadata and generate manifest.json file",
    )
    manifest_parser.set_defaults(cmd="manifest")

    args = parser.parse_args(argv)

    git_branch = find_git_branch()
    destdir = os.environ["DESTDIR"].format(
        git_branch=git_branch,
    )

    if args.cmd == "artifact":
        artifact_paths = glob.glob(args.file)
        assert len(artifact_paths) == 1
        file_path = artifact_paths[0]

        metadata = generate_file_metadata(file_path, destdir)
        print(json.dumps(metadata, indent=2, sort_keys=True))

        if os.getenv("CI") == "true":
            print(
                "::set-output name=artifact-{}::{}".format(
                    args.slug, json.dumps(metadata)
                )
            )
            url = urllib.parse.urlparse(metadata["file_url"])
            deploy_dir = posixpath.dirname(url.path)
            with open(os.environ["GITHUB_ENV"], mode="a") as fp:
                fp.write(f"DEPLOY_DIR={deploy_dir}\n")
    elif args.cmd == "manifest":
        job_data = json.loads(os.environ["JOB_DATA"])
        manifest_data = collect_manifest_data(job_data)
        print(json.dumps(manifest_data, indent=2, sort_keys=True))

        with open("manifest.json", mode="w") as fp:
            json.dump(manifest_data, fp, indent=2, sort_keys=True)

        if os.getenv("CI") == "true":
            manifest_url = os.environ["MANIFEST_URL"].format(
                git_branch=git_branch
            )
            try:
                remote_manifest_data = url_fetch(manifest_url)
            except IOError:
                remote_manifest_data = None

            if manifest_data == remote_manifest_data:
                return

            deploy_dir = os.environ["DESTDIR"].format(git_branch=git_branch)
            with open(os.environ["GITHUB_ENV"], mode="a") as fp:
                fp.write(f"DEPLOY_DIR={deploy_dir}\n")
                fp.write("MANIFEST_DIRTY=1\n")


if __name__ == "__main__":
    main()

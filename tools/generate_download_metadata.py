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
    """Make a web request to the given URL and return the response object."""
    request_headers = {
        # Override the User-Agent because our download server seems to block
        # requests with the default UA value and responds "403 Forbidden".
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
    """Make a HEAD request to the URL and check if the response is "200 OK"."""
    try:
        resp = url_fetch(url, method="HEAD")
    except IOError:
        return False
    return resp.status == 200


def url_download_json(url):
    """Returns the JSON object from the given URL or return None."""
    try:
        resp = url_fetch(url)
        manifest_data = resp.read().decode()
    except IOError:
        return None

    return json.loads(manifest_data)


def sha256(file_path):
    """Returns the sha256 hexdigest for a file."""
    with open(file_path, mode="rb") as fp:
        read_chunk = functools.partial(fp.read, 1024)
        m = hashlib.sha256()
        for data in iter(read_chunk, b""):
            m.update(data)
        return m.hexdigest()


def find_git_branch(path="."):
    """Return the checked out git branch for the given path."""
    return subprocess.check_output(
        ("git", "rev-parse", "--abbrev-ref", "HEAD"),
        cwd=path,
        encoding="utf-8",
    ).strip()


def generate_file_metadata(file_path, destdir):
    """
    Generate the file metadata for file_Path.

    The destdir argument is used for for generating the download URL.
    """
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
        "sha256": file_sha256,
        "sha256_url": f"{baseurl}/{destdir}/{file_name}.sha256sum",
    }


def collect_manifest_data(job_data):
    """Parse the job metadata dict and return the manifest data."""
    job_result = job_data["result"]
    print(f"Build job result: {job_result}")
    assert job_result == "success"

    manifest_data = {}
    for output_name, output_data in job_data["outputs"].items():
        # Filter out unrelated job outputs that don't start with "artifact-".
        prefix, _, slug = output_name.partition("-")
        if prefix != "artifact" or not slug:
            print(f"Ignoring output '{output_name}'...")
            continue
        artifact_data = json.loads(output_data)

        url = artifact_data["file_url"]

        # Make sure that the file actually exists on the download server
        resp = url_fetch(url, method="HEAD")
        if not resp.status == 200:
            raise LookupError(f"Unable to find URL '{url}' on remote server")

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
        # Check that we have exactly one file artifact
        artifact_paths = glob.glob(args.file)
        assert len(artifact_paths) == 1
        file_path = artifact_paths[0]

        # Generate metadata and print it
        metadata = generate_file_metadata(file_path, destdir)
        print(json.dumps(metadata, indent=2, sort_keys=True))

        if os.getenv("CI") == "true":
            # Set GitHub Actions job output
            print(
                "::set-output name=artifact-{}::{}".format(
                    args.slug, json.dumps(metadata)
                )
            )

            # Set the DEPLOY_DIR variable for the next build step
            url = urllib.parse.urlparse(metadata["file_url"])
            deploy_dir = posixpath.dirname(url.path)
            with open(os.environ["GITHUB_ENV"], mode="a") as fp:
                fp.write(f"DEPLOY_DIR={deploy_dir}\n")
    elif args.cmd == "manifest":
        # Parse the JOB_DATA JSON data, generate the manifest data and print it
        job_data = json.loads(os.environ["JOB_DATA"])
        manifest_data = collect_manifest_data(job_data)
        print(json.dumps(manifest_data, indent=2, sort_keys=True))

        # Write the manifest.json for subsequent deployment to the server
        with open("manifest.json", mode="w") as fp:
            json.dump(manifest_data, fp, indent=2, sort_keys=True)

        if os.getenv("CI") == "true":
            # Check if generated manifest.json file differs from the one that
            # is currently deployed.
            manifest_url = os.environ["MANIFEST_URL"].format(
                git_branch=git_branch
            )
            try:
                remote_manifest_data = url_fetch(manifest_url)
            except IOError:
                remote_manifest_data = None

            if manifest_data == remote_manifest_data:
                return

            # The manifest data is different, so we set the DEPLOY_DIR and
            # MANIFEST_DIRTY env vars.
            deploy_dir = os.environ["DESTDIR"].format(git_branch=git_branch)
            with open(os.environ["GITHUB_ENV"], mode="a") as fp:
                fp.write(f"DEPLOY_DIR={deploy_dir}\n")
                fp.write("MANIFEST_DIRTY=1\n")


if __name__ == "__main__":
    main()

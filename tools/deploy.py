#!/usr/bin/env python3
import argparse
import datetime
import functools
import hashlib
import json
import os
import pathlib
import shutil
import subprocess
import sys
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


def git_info(info, path="."):
    """Return the checked out git branch for the given path."""
    if info == "branch":
        cmd = ("git", "rev-parse", "--abbrev-ref", "HEAD")
    elif info == "commit":
        cmd = ("git", "rev-parse", "HEAD")
    elif info == "describe":
        # A dirty git state should only be possible on local builds, but since
        # this script may be used locally we'll add it here.
        cmd = ("git", "describe", "--dirty")
    else:
        raise ValueError("Invalid git info type!")

    return subprocess.check_output(
        cmd,
        cwd=path,
        encoding="utf-8",
    ).strip()


def splitext(filename):
    """
    Split filename into name without extenstion and file extension.

    This includes a workaround for ".tar.gz" files.
    """
    filename_without_ext, file_ext = os.path.splitext(filename)
    filename_without_ext2, file_ext2 = os.path.splitext(filename_without_ext)
    if file_ext2 == ".tar":
        filename_without_ext = filename_without_ext2
        file_ext = f"{file_ext2}{file_ext}"
    return filename_without_ext, file_ext


def tree(path):
    for dirpath, dirnames, filenames in os.walk(top=path):
        relpath = os.path.relpath(dirpath, start=path)
        if relpath != ".":
            yield relpath
        for filename in filenames:
            yield os.path.join(relpath, filename)


def slug(text):
    download_slug, _, package_slug = text.partition("-")
    if not download_slug or not package_slug:
        raise ValueError("Failed to parse slug")
    return download_slug, package_slug


def prepare_deployment(args):
    # Get artifact and build metadata
    file_stat = os.stat(args.file)
    file_sha256 = sha256(args.file)

    try:
        commit_id = os.environ["GITHUB_SHA"]
    except KeyError:
        commit_id = git_info("commit")

    metadata = {
        "git_commit": commit_id,
        "git_branch": git_info("branch"),
        "git_describe": git_info("describe"),
        "file_size": file_stat.st_size,
        "file_date": datetime.datetime.fromtimestamp(
            file_stat.st_ctime
        ).isoformat(),
        "file_sha256": file_sha256,
    }

    if os.getenv("CI") == "true":
        github_run_id = os.getenv("GITHUB_RUN_ID")
        github_server_url = os.getenv("GITHUB_SERVER_URL")
        github_repository = os.getenv("GITHUB_REPOSITORY")
        metadata.update(
            {
                "git_commit_url": (
                    f"{github_server_url}/{github_repository}/"
                    f"commit/{commit_id}"
                ),
                "build_log_url": (
                    f"{github_server_url}/{github_repository}/actions/"
                    f"runs/{github_run_id}"
                ),
            }
        )

    filename_without_ext, file_ext = splitext(args.file)
    download_slug, package_slug = args.slug

    # Build destination path scheme
    print(f"Destination path pattern: {args.dest_path}")
    destpath = args.dest_path.format(
        filename=os.path.basename(args.file),
        filename_without_ext=filename_without_ext,
        ext=file_ext,
        git_branch=metadata["git_branch"],
        git_commit_id=metadata["git_commit"],
        git_describe=metadata["git_describe"],
        package_slug=package_slug,
        download_slug=download_slug,
    )
    print(f"Destination path: {destpath}")

    # Move files to deploy in place and create sha256sum file
    output_destpath = os.path.join(args.output_dir, destpath)
    os.makedirs(os.path.dirname(output_destpath), exist_ok=True)
    shutil.copy2(args.file, output_destpath)

    output_filename = os.path.basename(destpath)
    with open(f"{output_destpath}.sha256sum", mode="w") as fp:
        fp.write(f"{file_sha256}  {output_filename}\n")

    metadata.update(
        {
            "file_url": f"{args.dest_url}/{destpath}",
            "file_sha256_url": f"{args.dest_url}/{destpath}.sha256sum",
        }
    )

    # Show metadata and files to deploy
    print("Metadata: ", json.dumps(metadata, indent=2, sort_keys=True))
    print("Files:")
    for path in tree(args.output_dir):
        print(path)

    # Write metadata to GitHub Actions step output, so that it can be used for
    # manifest creation in the final job after all builds finished.
    if os.getenv("CI") == "true":
        # Set GitHub Actions job output
        print(
            "::set-output name=artifact-{}-{}::{}".format(
                download_slug,
                package_slug,
                json.dumps(metadata),
            )
        )

    return 0


def collect_manifest_data(job_data):
    """Parse the job metadata dict and return the manifest data."""
    job_result = job_data["result"]
    print(f"Build job result: {job_result}")
    assert job_result == "success"

    manifest_data = {}
    for output_name, output_data in job_data["outputs"].items():
        # Filter out unrelated job outputs that don't start with "artifact-".
        prefix, _, artifact_slug = output_name.partition("-")
        if prefix != "artifact" or not artifact_slug:
            print(f"Ignoring output '{output_name}'...")
            continue
        artifact_data = json.loads(output_data)

        url = artifact_data["file_url"]

        # Make sure that the file actually exists on the download server
        resp = url_fetch(url, method="HEAD")
        if not resp.status == 200:
            raise LookupError(f"Unable to find URL '{url}' on remote server")

        manifest_data[artifact_slug] = artifact_data

    return manifest_data


def generate_manifest(args):
    try:
        commit_id = os.getenv("GITHUB_SHA")
    except KeyError:
        commit_id = git_info("commit")

    format_data = {
        "git_branch": git_info("branch"),
        "git_commit_id": commit_id,
        "git_describe": git_info("describe"),
    }

    # Build destination path scheme
    print(f"Destination path pattern: {args.dest_path}")
    destpath = args.dest_path.format_map(format_data)
    print(f"Destination path: {destpath}")

    # Create the deployment directory
    output_destpath = os.path.join(args.output_dir, destpath)
    os.makedirs(os.path.dirname(output_destpath), exist_ok=True)

    # Parse the JOB_DATA JSON data, generate the manifest data and print it
    job_data = json.loads(os.environ["JOB_DATA"])
    manifest_data = collect_manifest_data(job_data)
    print("Manifest:", json.dumps(manifest_data, indent=2, sort_keys=True))

    # Write the manifest.json for subsequent deployment to the server
    with open(output_destpath, mode="w") as fp:
        json.dump(manifest_data, fp, indent=2, sort_keys=True)

    # If possible, check if the remote manifest is the same as our local one
    remote_manifest_data = None
    if args.dest_url:
        # Check if generated manifest.json file differs from the one that
        # is currently deployed.
        manifest_url = f"{args.dest_url}/{destpath}"
        manifest_url = manifest_url.format_map(format_data)

        try:
            remote_manifest_data = url_fetch(manifest_url)
        except IOError:
            pass

    # Skip deployment if the remote manifest is the same as the local one.
    if manifest_data != remote_manifest_data:
        print("Remote manifest differs from local version.")
        if os.getenv("CI") == "true":
            with open(os.environ["GITHUB_ENV"], mode="a") as fp:
                fp.write("MANIFEST_DIRTY=1\n")
    else:
        print("Remote manifest is the same as local version.")

    print("Files:")
    for path in tree(args.output_dir):
        print(path)

    return 0


def main(argv=None):
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()

    artifact_parser = subparsers.add_parser(
        "prepare-deployment", help=" artifact metadata from file"
    )
    artifact_parser.set_defaults(cmd=prepare_deployment)
    artifact_parser.add_argument(
        "--slug",
        action="store",
        required=True,
        type=slug,
        help="Artifact identifier for the website's download page",
    )
    artifact_parser.add_argument(
        "--output-dir",
        action="store",
        default="deploy",
        help="Directory to write output to (default: 'deploy')",
    )
    artifact_parser.add_argument(
        "--dest-path",
        action="store",
        required=True,
        help="Destination path inside the output directory",
    )
    artifact_parser.add_argument(
        "--dest-url",
        action="store",
        required=True,
        help="Destination URL prefix",
    )
    artifact_parser.add_argument(
        "file", type=pathlib.Path, help="Local file to deploy"
    )

    manifest_parser = subparsers.add_parser(
        "generate-manifest",
        help="Collect artifact metadata and generate manifest.json file",
    )
    manifest_parser.set_defaults(cmd=generate_manifest)
    manifest_parser.add_argument(
        "--output-dir",
        action="store",
        default="deploy",
        help="Directory to write output to (default: 'deploy')",
    )
    manifest_parser.add_argument(
        "--dest-path",
        action="store",
        required=True,
        help="Destination path inside the output directory",
    )
    manifest_parser.add_argument(
        "--dest-url", action="store", help="Destination URL prefix"
    )

    args = parser.parse_args(argv)

    if os.path.exists(args.output_dir):
        if not os.path.isdir(args.output_dir):
            raise OSError("Output dir is not a directory!")
        if os.listdir(args.output_dir):
            raise OSError("Output dir is not empty!")

    return args.cmd(args)


if __name__ == "__main__":
    sys.exit(main())

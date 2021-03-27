import json
import os
import subprocess
import sys
import urllib.error
import urllib.request


def url_exists(url):
    req = urllib.request.Request(url, method="HEAD")
    resp = urllib.request.urlopen(req, timeout=10)
    return resp.status == 200


def download(url):
    try:
        resp = urllib.request.urlopen(url, timeout=10)
        manifest_data = resp.read().decode()
    except IOError:
        return None

    return json.loads(manifest_data)


job_data = json.loads(os.environ["JOB_DATA"])
job_result = job_data["result"]
print(f"Build job result: {job_result}")
assert job_result == "success"

manifest_data = {}
missing_artifacts = False
for output_name, output_data in job_data["outputs"].items():
    prefix, _, slug = output_name.partition("-")
    if prefix != "artifact" or not slug:
        print(f"Ignoring output '{output_name}'...")
        continue
    artifact_data = json.loads(output_data)

    # FIXME: Unfortunately this doesn't work because the server responds with
    # "403 Forbidden". We should reenable it when we figure out the issue
    # print("Checking if package actually exists...", end="")
    # url = artifact_data["file_url"]
    # package_exists = url_exists(url)
    # if not package_exists:
    #    print(f"fail ({url})")
    #    missing_artifacts = True
    #    continue
    # print("ok")

    manifest_data[slug] = artifact_data

print(json.dumps(manifest_data, indent=2, sort_keys=True))

assert manifest_data
if missing_artifacts:
    sys.exit(1)

with open("manifest.json", mode="w") as fp:
    json.dump(manifest_data, fp, indent=2, sort_keys=True)

if os.getenv("CI") == "true":
    git_branch = (
        subprocess.check_output(("git", "rev-parse", "--abbrev-ref", "HEAD"))
        .decode()
        .strip()
    )
    manifest_url = os.environ["MANIFEST_URL"].format(git_branch=git_branch)
    if manifest_data == download(manifest_url):
        sys.exit(0)

    deploy_dir = os.environ["DEPLOY_DESTPATH"].format(git_branch=git_branch)
    with open(os.environ["GITHUB_ENV"], mode="a") as fp:
        fp.write(f"DEPLOY_DIR={deploy_dir}\n")
        fp.write("MANIFEST_DIRTY=1\n")

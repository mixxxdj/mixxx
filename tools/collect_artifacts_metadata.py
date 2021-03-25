import os
import json
import subprocess
import urllib.request
import urllib.error


def url_exists(url):
    req = urllib.request.Request(url, method="HEAD")
    try:
        resp = urllib.request.urlopen(req, timeout=10)
    except urllib.error.URLError:
        return False
    return resp.status == 200


job_data = json.loads(os.environ["JOB_DATA"])
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

    print("Checking if package actually exists...", end="")
    url = artifact_data["file_url"]
    package_exists = url_exists(url)
    if not package_exists:
        print("fail ({url})")
        continue
    print("ok")

    manifest_data[slug] = artifact_data

print(json.dumps(manifest_data, indent=2, sort_keys=True))

assert manifest_data

with open("manifest.json", mode="w") as fp:
    json.dump(manifest_data, fp, indent=2, sort_keys=True)

if os.getenv("CI") == "true":
    git_branch = (
        subprocess.check_output(("git", "rev-parse", "--abbrev-ref", "HEAD"))
        .decode()
        .strip()
    )
    deploy_dir = os.environ["GITHUB_ENV"].format(git_branch=git_branch)
    with open(os.environ["GITHUB_ENV"], mode="a") as fp:
        fp.write(f"DEPLOY_DIR={deploy_dir}\n")

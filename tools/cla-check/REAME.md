# IaC for CLA Checks

This IaC for OpenToFu is used to provision all the required setup in GCP, in order to perform authenticate a service account to check the CLA signing sheet.

It will provision:

- A GCP project
- An Identity pool
- A service account
- The permission binding to this service account to create an auth token

## State

Terraform/OpenToFu should be used with a "backend" to persist the state of the last known applied setup. However, since we don't have a setup for this, we use a stateless approach. This means that every time you would run a `tofu` command, it will rebuild a local state, using the `import.tf`, which references any existing resources.

This means two fundamental caveats to be aware of:

- There is no locking mechanism to prevent concurrency changes. Always make sure no other core member is deploying changes
- New resources must be referenced in the `import.tf`

## Applying changes

```bash
gcloud auth application-default login
tofu init
tofu apply
```

resource "google_project" "main" {
  name       = "Mixxx"
  project_id = "mixxx-485709"
  org_id     = "251716126893"
}

resource "google_service_account" "sa" {
  project    = google_project.main.project_id
  account_id = "cla-check-sa"
}

resource "google_iam_workload_identity_pool" "main" {
  provider                  = google-beta
  project                   = google_project.main.project_id
  workload_identity_pool_id = "gh-pool"
  display_name              = "Github Actions"
  description               = "CI workflows hosted on Github"
  disabled                  = false
}

resource "google_iam_workload_identity_pool_provider" "main" {
  provider                           = google-beta
  project                            = google_project.main.project_id
  workload_identity_pool_id          = google_iam_workload_identity_pool.main.workload_identity_pool_id
  workload_identity_pool_provider_id = "gh-provider"
  display_name                       = "Github Actions"
  description                        = "Provider for Service Account in GCP used by Github actions"
  attribute_condition                = local.pool_provider_assertion
  attribute_mapping = {
    "google.subject"             = "assertion.sub"
    "attribute.actor"            = "assertion.actor"
    "attribute.aud"              = "assertion.aud"
    "attribute.repository"       = "assertion.repository"
    "attribute.repository_owner" = "assertion.repository_owner"
    "attribute.ref"              = "assertion.ref"
    "attribute.ref_type"         = "assertion.ref_type"
  }
  oidc {
    allowed_audiences = []
    issuer_uri        = var.issuer_uri
  }
}

resource "google_service_account_iam_member" "wif-sa" {
  service_account_id = google_service_account.sa.name
  role               = "roles/iam.workloadIdentityUser"
  member             = local.iam_member
}

resource "google_service_account_iam_member" "satc-sa" {
  service_account_id = google_service_account.sa.name
  role               = "roles/iam.serviceAccountTokenCreator"
  member             = local.iam_member
}

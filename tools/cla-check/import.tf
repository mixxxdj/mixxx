import {
  to = google_project.main
  id = "projects/mixxx-485709"
}
import {
  to = google_service_account.sa
  id = "projects/mixxx-485709/serviceAccounts/cla-check-sa@mixxx-485709.iam.gserviceaccount.com"
}
import {
  to = google_iam_workload_identity_pool.main
  id = "projects/mixxx-485709/locations/global/workloadIdentityPools/gh-pool"
}
import {
  to = google_service_account_iam_member.satc-sa
  id = "projects/mixxx-485709/serviceAccounts/cla-check-sa@mixxx-485709.iam.gserviceaccount.com roles/iam.serviceAccountTokenCreator principalSet://iam.googleapis.com/projects/763965891340/locations/global/workloadIdentityPools/gh-pool/attribute.repository_owner/mixxxdj"
}
import {
  to = google_service_account_iam_member.wif-sa
  id = "projects/mixxx-485709/serviceAccounts/cla-check-sa@mixxx-485709.iam.gserviceaccount.com roles/iam.workloadIdentityUser principalSet://iam.googleapis.com/projects/763965891340/locations/global/workloadIdentityPools/gh-pool/attribute.repository_owner/mixxxdj"
}
import {
  to = google_iam_workload_identity_pool_provider.main
  id = "projects/mixxx-485709/locations/global/workloadIdentityPools/gh-pool/providers/gh-provider"
}

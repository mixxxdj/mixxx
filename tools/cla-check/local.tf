locals {
  pool_provider_assertion_origins = {
    for repository, branches in var.allowed_origins : repository => join(" || ", [
      for branch in branches : "assertion.ref == 'refs/heads/${branch}'"
    ])
  }
  pool_provider_assertion = join(" || ", [
    for repository, branch_condition in local.pool_provider_assertion_origins :
    "(assertion.repository == '${repository}' && (${branch_condition}) && assertion.ref_type == 'branch')"
  ])
  # We only use a single SA to perform CLA check so we only use the organisation name. Main security is ensure by the pool assertion condition, we restrict all origins to ensure they are strictly allowed
  iam_member = "principalSet://iam.googleapis.com/${google_iam_workload_identity_pool.main.name}/attribute.repository_owner/mixxxdj"
}

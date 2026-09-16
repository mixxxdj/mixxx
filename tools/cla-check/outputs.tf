output "pool_name" {
  description = "Pool name"
  value       = google_iam_workload_identity_pool.main.name
}

output "provider_name" {
  description = "Provider name"
  value       = google_iam_workload_identity_pool_provider.main.name
}

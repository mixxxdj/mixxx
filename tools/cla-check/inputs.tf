variable "issuer_uri" {
  type        = string
  description = "Workload Identity Pool Issuer URL"
  default     = "https://token.actions.githubusercontent.com"
}

variable "allowed_origins" {
  type        = map(list(string))
  description = "Origins allowed to access the CLA sheet Service Account"
  default     = {}
}

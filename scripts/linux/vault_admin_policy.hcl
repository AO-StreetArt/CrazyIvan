# This section grants all access on "secret/*".
path "secret/*" {
  capabilities = ["read", "list"]
}

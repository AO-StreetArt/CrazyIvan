# This section grants all access on "secret/*".
path "secret/*" {
  capabilities = ["read", "list"]
}

path "consul/*" {
  capabilities = ["read", "list"]
}

path "pki/*" {
  capabilities = ["write", "read", "list"]
}

#!/bin/bash
set -e
#This script will setup a dev vault server with the needed config for the
# vault test to run

export VAULT_ADDR='http://127.0.0.1:8200'

vault auth enable userpass
vault write auth/userpass/users/test password=test policies=admins
vault policy write admins vault_admin_policy.hcl
vault secrets enable -version=2 kv
vault secrets enable pki
vault secrets tune -max-lease-ttl=8760h pki

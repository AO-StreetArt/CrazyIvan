#!/bin/bash
set -e

vault kv put secret/NEO4J_AUTH_UN NEO4J_AUTH_UN=test
vault kv put secret/NEO4J_AUTH_PW NEO4J_AUTH_PW=test
vault kv put secret/AOSSL_CONSUL_SSL_CERT AOSSL_CONSUL_SSL_CERT=test
vault kv put secret/AOSSL_CONSUL_ACL_TOKEN AOSSL_CONSUL_ACL_TOKEN=test

#!/bin/bash
set -e

vault kv put secret/IVAN_PROD_NEO4J_AUTH_UN IVAN_PROD_NEO4J_AUTH_UN=neo4j
vault kv put secret/IVAN_PROD_NEO4J_AUTH_PW IVAN_PROD_NEO4J_AUTH_PW=admin
# vault kv put secret/AOSSL_CONSUL_SSL_CERT AOSSL_CONSUL_SSL_CERT=test
# vault kv put secret/AOSSL_CONSUL_ACL_TOKEN AOSSL_CONSUL_ACL_TOKEN=test

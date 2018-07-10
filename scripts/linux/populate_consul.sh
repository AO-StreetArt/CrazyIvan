#!/bin/bash

#Populate default configuration values into Consul

curl -X PUT -d 'ivan.log' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_LOG_FILE
curl -X PUT -d 'Debug' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_LOG_LEVEL
curl -X PUT -d 'json' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_EVENT_FORMAT
curl -X PUT -d 'udp' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_EVENT_STREAM_METHOD
curl -X PUT -d 'json' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_TRANSACTION_FORMAT
curl -X PUT -d 'True' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_NEO4J_DISCOVER
curl -X PUT -d 'True' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_TRANSACTION_ID_STAMP
curl -X PUT -d 'True' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_TRANSACTION_SECURITY_SSL_ENABLED
curl -X PUT -d 'True' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_EVENT_SECURITY_AES_ENABLED
curl -X PUT -d 'True' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_TRANSACTION_SECURITY_SSL_CA_VAULT_ACTIVE
curl -X PUT -d 'test' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_TRANSACTION_SECURITY_SSL_CA_VAULT_ROLE_NAME
curl -X PUT -d 'test' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_TRANSACTION_SECURITY_SSL_CA_VAULT_COMMON_NAME
curl -X PUT -d 'single' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_TRANSACTION_SECURITY_AUTH_TYPE

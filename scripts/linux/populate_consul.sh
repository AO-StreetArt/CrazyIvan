#!/bin/bash

#Populate default configuration values into Redis

curl -X PUT -d 'neo4j://neo4j:neo4j@localhost:7687' http://localhost:8500/v1/kv/ivan/DB_ConnectionString

curl -X PUT -d '127.0.0.1--6379----2--5--0' http://localhost:8500/v1/kv/ivan/RedisConnectionString

curl -X PUT -d 'True' http://localhost:8500/v1/kv/ivan/StampTransactionId

curl -X PUT -d 'True' http://localhost:8500/v1/kv/ivan/AtomicTransactions

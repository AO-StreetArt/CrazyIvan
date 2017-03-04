#!/bin/bash

CONSUL_ARG=$1
IP_ARG=$2
PORT_ARG=$3
LOG_ARG=$4
BRANCH_NAME=${5:-latest}

if [ $BRANCH_NAME != "latest" ]; then
  cd CrazyIvan && git checkout $BRANCH_NAME && git pull origin $BRANCH_NAME
fi

# Build the Project & Unit Tests
cd CrazyIvan && make && make test

# Execute Unit Tests
cd CrazyIvan && ./scene_test
cd CrazyIvan && ./configuration_test -config-file=src/test/test.properties

#Start up Crazy Ivan
cd CrazyIvan && ./crazy_ivan $CONSUL_ARG $IP_ARG $PORT_ARG $LOG_ARG

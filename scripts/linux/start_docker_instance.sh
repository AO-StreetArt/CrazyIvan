#!/bin/bash

CONSUL_ARG=$1
IP_ARG=$2
PORT_ARG=$3
LOG_ARG=$4
BRANCH_NAME=${5:-latest}
BASE_DIR=${6:-/usr/local/etc}

mkdir $BASE_DIR/CrazyIvan

#Pull the project source from github
git clone https://github.com/AO-StreetArt/CrazyIvan.git $BASE_DIR/CrazyIvan

if [ $BRANCH_NAME != "latest" ]; then
  cd $BASE_DIR/CrazyIvan && git checkout $BRANCH_NAME && git pull origin $BRANCH_NAME
fi

# Build the Project & Unit Tests
cd $BASE_DIR/CrazyIvan && make && make test

# Execute Unit Tests
cd $BASE_DIR/CrazyIvan && ./scene_test
cd $BASE_DIR/CrazyIvan && ./configuration_test -config-file=src/test/test.properties

#Start up Crazy Ivan
cd $BASE_DIR/CrazyIvan && ./crazy_ivan $CONSUL_ARG $IP_ARG $PORT_ARG $LOG_ARG

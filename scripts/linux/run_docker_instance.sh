#!/bin/bash

BRANCH_NAME=${1:-master}
CONSUL_ADDR=${2:-"localhost:8500"}
IP=${3:-localhost}
PORT=${4:-5555}

if [ $BRANCH_NAME == "master" ]; then
  docker run --name crazyivan -d aostreetart/crazyivan -consul-addr=$CONSUL_ADDR -ip=$IP -port=$PORT &
else
  docker run --name crazyivan -d aostreetart/crazyivan:$BRANCH_NAME -consul-addr=$CONSUL_ADDR -ip=$IP -port=$PORT &
fi

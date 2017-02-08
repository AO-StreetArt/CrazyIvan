#!/bin/bash

BRANCH_NAME=${1:-master}
CONSUL_ADDR=${2:-"localhost:8500"}
IP=${3:-localhost}
PORT=${4:-5555}
LINK_NAME=${5:-database}
LINK_IMAGE=${6:-neo4j}

if [ $BRANCH_NAME == "master" ]; then
  docker run --name crazyivan --link $LINK_NAME:$LINK_IMAGE -d aostreetart/crazyivan -consul-addr=$CONSUL_ADDR -ip=$IP -port=$PORT &
else
  docker run --name crazyivan --link $LINK_NAME:$LINK_IMAGE -d aostreetart/crazyivan:$BRANCH_NAME -consul-addr=$CONSUL_ADDR -ip=$IP -port=$PORT &
fi

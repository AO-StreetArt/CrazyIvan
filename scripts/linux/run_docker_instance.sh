#!/bin/bash

BRANCH_NAME=${1:-master}
CONSUL_ADDR=${2:-"localhost:8500"}
IP=${3:-localhost}
PORT=${4:-5555}
LINK_NAME=${5:-database}
LINK_IMAGE=${6:-neo4j}

TAG_NAME=""

if [ $BRANCH_NAME != "master" ]; then
  TAG_NAME=$BRANCH_NAME
fi

docker run --name crazyivan --link $LINK_NAME:$LINK_IMAGE -d aostreetart/crazyivan:$TAG_NAME -consul-addr=$CONSUL_ADDR -ip=$IP -port=$PORT

#!/bin/bash

BRANCH_NAME=${1:-master}
CONSUL_ADDR=${2:-"localhost:8500"}
IP=${3:-localhost}
PORT=${4:-5555}

TAG_NAME="latest"

if [ $BRANCH_NAME != "master" ]; then
  TAG_NAME=$BRANCH_NAME
fi

docker run --name crazyivan --link database:neo4j --link registry:consul --link cache:redis -p 5555:5555 -d aostreetart/crazyivan:$TAG_NAME -consul-addr=$CONSUL_ADDR -ip=$IP -port=$PORT -log-conf=CrazyIvan/log4cpp.properties

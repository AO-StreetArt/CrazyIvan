#!/bin/bash

BRANCH_NAME=${1:-master}
CONSUL_ADDR=${2:-"localhost:8500"}
IP=${3:-localhost}
PORT=${4:-5555}
LINK_NAME=${5:-database}
LINK_IMAGE=${6:-neo4j}
LINK_NAME2=${7:-""}
LINK_IMAGE2=${8:-""}
LINK_NAME3=${9:-""}
LINK_IMAGE3=${10:-""}

if [ $BRANCH_NAME == "master" ]; then
  if [ $LINK_NAME2 == "" ]; then
    docker run --name crazyivan --link $LINK_NAME:$LINK_IMAGE -d aostreetart/crazyivan -consul-addr=$CONSUL_ADDR -ip=$IP -port=$PORT
  else
    docker run --name crazyivan --link $LINK_NAME:$LINK_IMAGE --link $LINK_NAME2:$LINK_IMAGE2 --link $LINK_NAME3:$LINK_IMAGE3 -d aostreetart/crazyivan -consul-addr=$CONSUL_ADDR -ip=$IP -port=$PORT
  fi
else
  if [ $LINK_NAME2 == "" ]; then
    docker run --name crazyivan --link $LINK_NAME:$LINK_IMAGE -d aostreetart/crazyivan:$BRANCH_NAME -consul-addr=$CONSUL_ADDR -ip=$IP -port=$PORT
  else
    docker run --name crazyivan --link $LINK_NAME:$LINK_IMAGE --link $LINK_NAME2:$LINK_IMAGE2 --link $LINK_NAME3:$LINK_IMAGE3 -d aostreetart/crazyivan:$BRANCH_NAME -consul-addr=$CONSUL_ADDR -ip=$IP -port=$PORT
  fi
fi

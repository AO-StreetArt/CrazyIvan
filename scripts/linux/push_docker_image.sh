#!/bin/bash

DOCKER_USER=$1
DOCKER_PASS=$2
BRANCH_NAME=$3
DIR=$4

TAG_NAME="v2"

echo $BRANCH_NAME

if [ $BRANCH_NAME != "v2" ]; then
  TAG_NAME=$BRANCH_NAME
fi

cd $DIR && docker login -u $DOCKER_USER -p $DOCKER_PASS
cd $DIR && docker push aostreetart/crazyivan:$TAG_NAME

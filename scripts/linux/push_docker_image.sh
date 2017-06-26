#!/bin/bash

DOCKER_EMAIL=$1
DOCKER_USER=$2
DOCKER_PASS=$3
BRANCH_NAME=$4
DIR=$5

TAG_NAME="latest"

echo $BRANCH_NAME

if [ $BRANCH_NAME != "master" ]; then
  TAG_NAME=$BRANCH_NAME
fi

cd $DIR && docker login -e $DOCKER_EMAIL -u $DOCKER_USER -p $DOCKER_PASS
cd $DIR && docker push aostreetart/crazyivan:$TAG_NAME

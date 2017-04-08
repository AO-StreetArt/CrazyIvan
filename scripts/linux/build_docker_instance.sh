#!/bin/bash

BRANCH_NAME=$1
DIR=$2

TAG_NAME="latest"

if [ $BRANCH_NAME != "master" ]; then
  TAG_NAME=$BRANCH_NAME
fi

cd $DIR && sudo docker build --build-arg TAG_NAME=$TAG_NAME -t "aostreetart/crazyivan:$TAG_NAME" .

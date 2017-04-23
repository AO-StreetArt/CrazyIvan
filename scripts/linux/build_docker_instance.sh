#!/bin/bash

BRANCH_NAME=$1
DIR=$2

TAG_NAME="latest"

if [ $BRANCH_NAME != "master" ]; then
  cd $DIR && sudo docker build --file DevelDockerfile -t "aostreetart/crazyivan:$BRANCH_NAME" .
  TAG_NAME=$BRANCH_NAME
else
  cd $DIR && sudo docker build -t "aostreetart/crazyivan:$TAG_NAME" .
fi

#!/bin/bash

BRANCH_NAME=$1
DIR=$2

TAG_NAME="latest"

echo $BRANCH_NAME
echo $DIR

if [ $BRANCH_NAME != "master" ]; then
  cd $DIR && docker build --file DevelDockerfile -t "aostreetart/crazyivan:$BRANCH_NAME" .
  TAG_NAME=$BRANCH_NAME
else
  cd $DIR && docker build -t "aostreetart/crazyivan:$TAG_NAME" .
fi

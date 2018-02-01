#!/bin/bash

BRANCH_NAME=$1
DIR=$2

TAG_NAME="latest"

echo $BRANCH_NAME
echo $DIR

if [ $BRANCH_NAME != "master" ]; then
  TAG_NAME=$BRANCH_NAME
fi

cd $DIR && docker build -t "aostreetart/crazyivan:$TAG_NAME" .

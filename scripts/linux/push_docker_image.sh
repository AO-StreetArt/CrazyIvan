#!/bin/bash

DOCKER_EMAIL=$1
DOCKER_USER=$2
DOCKER_PASS=$3
BRANCH_NAME=$4
DIR=$5

cd $DIR && sudo docker login -e $DOCKER_EMAIL -u $DOCKER_USER -p $DOCKER_PASS
cd $DIR && sudo docker push aostreetart/crazyivan:$BRANCH_NAME

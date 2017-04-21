#!/bin/bash

sudo docker build --file DebugDockerfile --build-arg TAG_NAME="devel" -t "aostreetart/crazyivan:debug" .

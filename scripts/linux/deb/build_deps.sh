#!/bin/bash
set -e
#This script will attempt to build CrazyIvan dependencies

#Based on Ubuntu 14.04 LTS
#Not intended for use with other OS (should function correctly with Debian 7, untested)

printf "Creating Dependency Folder"
PRE=./downloads
RETURN=..
mkdir $PRE

printf "Calling apt-get update"

#Update the Ubuntu Server
apt-get -y update
apt-get install -y git libboost-all-dev

#Build & Install the Shared Service Library

if [ ! -d /usr/local/include/aossl ]; then

  #Create the folder to clone into
  mkdir $PRE/aossl

  #Pull the code down
  git clone https://github.com/AO-StreetArt/AOSharedServiceLibrary.git $PRE/aossl

  #Build the dependencies for the shared service library
  mkdir $PRE/aossl_deps
  cp $PRE/aossl/scripts/deb/build_deps.sh $PRE/aossl_deps/
  cd $PRE/aossl_deps
  ./build_deps.sh
  cd ../$RETURN

  #Build the shared service library
  cd $PRE/aossl
  make
  make install
  ldconfig
  cd ../..

fi

#Determine if we Need RapidJSON
if [ ! -d /usr/local/include/rapidjson ]; then

  printf "Cloning RapidJSON"

  mkdir $PRE/rapidjson

  #Get the RapidJSON Dependency
  git clone https://github.com/miloyip/rapidjson.git $PRE/rapidjson

  #Move the RapidJSON header files to the include path
  cp -r $PRE/rapidjson/include/rapidjson/ /usr/local/include

fi

#Ensure we have access to the Protocol Buffer Interfaces
if [ ! -d /usr/local/include/dvs_interface ]; then
  mkdir $PRE/interfaces/
  git clone https://github.com/AO-StreetArt/DvsInterface.git $PRE/interfaces
  cd $PRE/interfaces && make install
  cd ../..
fi

# Install librdkafka
if [ ! -d /usr/local/include/librdkafka ]; then
  wget https://github.com/edenhill/librdkafka/archive/v0.11.3.tar.gz
  tar -xvzf v0.11.3.tar.gz
  cd librdkafka-0.11.3 && ./configure && make && make install
  cd ..
fi

# Here we look to install cppkafka
if [ ! -d /usr/local/include/cppkafka ]; then
  printf "Cloning CppKafka\n"

  mkdir $PRE/cppkafka

  #Get the CppKafka Dependency
  git clone https://github.com/mfontanini/cppkafka.git $PRE/cppkafka

  # Build and install
  mkdir $PRE/cppkafka/build && cd $PRE/cppkafka/build && cmake .. && make && make install

fi

#Install python, pyzmq, protobuf, boost, and the protobuf compiler
apt-get install -y python-pip python-dev libprotobuf-dev protobuf-compiler libboost-all-dev
pip install pyzmq

printf "Finished installing dependencies"

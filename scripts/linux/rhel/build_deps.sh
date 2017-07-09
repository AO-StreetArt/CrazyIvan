#!/bin/bash
exec 3>&1 4>&2
trap 'exec 2>&4 1>&3' 0 1 2 3
exec 1>build_deps.log 2>&1

#This script will attempt to build CLyman dependencies

#Based on CentOS 7
#Not intended for use with other OS (should function correctly with Red Hat Enterprise Linux 7, untested)

printf "Creating Dependency Folder"
PRE=./downloads
RETURN=..
mkdir $PRE

printf "Calling apt-get update"

sudo yum install openssl-devel

#Update the Ubuntu Server
sudo yum -y update

#Build the dependencies and place them in the correct places

#Determine if we Need RapidJSON
if [ ! -d /usr/local/include/rapidjson ]; then

  printf "Cloning RapidJSON"

  mkdir $PRE/rapidjson

  #Get the RapidJSON Dependency
  git clone https://github.com/miloyip/rapidjson.git $PRE/rapidjson

  #Move the RapidJSON header files to the include path
  sudo cp -r $PRE/rapidjson/include/rapidjson/ /usr/local/include

fi

#Ensure we have access to the Protocol Buffer Interfaces
if [ ! -d /usr/local/include/dvs_interface ]; then
  mkdir $PRE/interfaces/
  git clone https://github.com/AO-StreetArt/DvsInterface.git $PRE/interfaces
  cd $PRE/interfaces && sudo make install
fi

#Build & Install the Shared Service Library

if [ ! -d /usr/local/include/aossl ]; then

  #Create the folder to clone into
  mkdir $PRE/aossl

  #Pull the code down
  git clone https://github.com/AO-StreetArt/AOSharedServiceLibrary.git $PRE/aossl

  #Build the dependencies for the shared service library
  mkdir $PRE/aossl_deps
  cp $PRE/aossl/scripts/rhel/build_deps.sh $PRE/aossl_deps/
  cd $PRE/aossl_deps && sudo ./build_deps.sh
  cd ../$RETURN

  #Build the shared service library
  cd $PRE/aossl && make && sudo make install
  sudo ldconfig

fi

#Install python, pyzmq, protobuf, and the protobuf compiler
sudo yum install -y python-pip python-devel protobuf-devel protobuf-compiler
sudo pip install pyzmq

printf "Finished installing dependencies"

#!/bin/bash
set -e

#This script will attempt to build CLyman dependencies

#Based on CentOS 7
#Not intended for use with other OS (should function correctly with Red Hat Enterprise Linux 7, untested)

PRE=./downloads
RETURN=..
mkdir $PRE

# Set up a reasonably new version of gcc
yum -y install openssl-devel boost-devel centos-release-scl wget git gcc gcc-c++
yum -y update
yum -y install devtoolset-6
scl enable devtoolset-6 bash

#Build & Install the Shared Service Library
if [ ! -d /usr/local/include/aossl ]; then

  wget https://github.com/AO-StreetArt/AOSharedServiceLibrary/releases/download/v2.4.4/aossl-rhel-2.4.4.tar.gz
  tar -xvzf aossl-rhel-2.4.4.tar.gz

  #Build the dependencies for the shared service library
  mkdir $PRE/aossl_deps
  cp aossl-rhel/deps/build_deps.sh $PRE/aossl_deps/
  cd $PRE/aossl_deps && ./build_deps.sh
  cd ../$RETURN

  #Build the shared service library
  cd aossl-rhel && make clean && make && make install
  cd ../

fi

# Build and install NeoCpp
if [ ! -d /usr/local/include/neocpp ]; then

  git clone https://github.com/AO-StreetArt/NeoCpp.git
  #Build the dependencies for the shared service library
  mkdir $PRE/neocpp_deps
  cp NeoCpp/scripts/linux/rhel/build_deps.sh $PRE/neocpp_deps/
  cd $PRE/neocpp_deps && ./build_deps.sh
  cd ../$RETURN
  cd NeoCpp && make install
  cd ../

fi

# Install GLM
if [ ! -d /usr/local/include/glm ]; then

  wget https://github.com/g-truc/glm/releases/download/0.9.9.0/glm-0.9.9.0.zip
  unzip glm-0.9.9.0.zip
  mkdir /usr/local/include/glm/
  cp -r glm/glm/* /usr/local/include/glm/

fi

printf "Finished installing dependencies"

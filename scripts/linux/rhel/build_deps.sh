#!/bin/bash
set -e

#This script will attempt to build CLyman dependencies

#Based on CentOS 7
#Not intended for use with other OS (should function correctly with Red Hat Enterprise Linux 7, untested)

PRE=./downloads
RETURN=..
mkdir $PRE

# Set up a reasonably new version of gcc
sudo yum -y install openssl-devel boost-devel centos-release-scl wget git
sudo yum -y update
sudo yum -y install devtoolset-7
scl enable devtoolset-7 bash

#Build POCO
wget https://pocoproject.org/releases/poco-1.9.0/poco-1.9.0-all.tar.gz
tar -xvzf poco-1.9.0-all.tar.gz
cd poco-1.9.0-all && ./configure --omit=Data/ODBC,Data/MySQL && gmake -s && sudo gmake -s install
cd ../

#Build & Install the Shared Service Library
if [ ! -d /usr/local/include/aossl ]; then

  wget https://github.com/AO-StreetArt/AOSharedServiceLibrary/releases/download/v2.0.0/aossl-rhel-2.0.0.tar.gz
  tar -xvzf aossl-rhel-2.0.0.tar.gz

  #Build the dependencies for the shared service library
  mkdir $PRE/aossl_deps
  cp aossl-rhel/deps/build_deps.sh $PRE/aossl_deps/
  cd $PRE/aossl_deps && sudo ./build_deps.sh
  cd ../$RETURN

  #Build the shared service library
  cd aossl-rhel && make && sudo make install
  cd ../

fi

# Build and install NeoCpp
if [ ! -d /usr/local/include/neocpp ]; then

  git clone https://github.com/AO-StreetArt/NeoCpp.git
  #Build the dependencies for the shared service library
  mkdir $PRE/neocpp_deps
  cp NeoCpp/scripts/linux/rhel/build_deps.sh $PRE/neocpp_deps/
  cd $PRE/neocpp_deps && sudo ./build_deps.sh
  cd ../$RETURN
  cd NeoCpp && sudo make install

fi

sudo ldconfig

printf "Finished installing dependencies"

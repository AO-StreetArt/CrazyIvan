#!/bin/bash
#This script will install the pre-packaged release of CrazyIvan
#Can be run with no command line arguments to simply install library
#-d flag will run install_deps script
#-r flag will uninstall

OPT="-none"

if [ "$#" -gt 0 ]; then
  OPT=$1
fi

if [ "$#" -gt 1 ]; then
  printf "You can execute this script with no input parameters to simply install the library,"
  printf "or with the '-r' flag to remove the library.  You may also supply the '-d' flag to"
  printf "install the dependencies in addition to the CrazyIvan executable.\n"
  exit 64
fi

if [ $OPT = "-r" ]; then
  printf "Attempting to uninstall CrazyIvan\n"
  sudo rm /usr/local/bin/crazy_ivan
  sudo rm /usr/local/bin/stop_crazyivan.py
  sudo rm -r /etc/crazyivan
  sudo rm -r /var/log/crazyivan
  sudo deluser crazyivan
  sudo groupdel crazyivan
  sudo rm /etc/systemd/system/crazyivan.service
  exit 0
else
  if [ $OPT = "-d" ]; then
    #Install the dependencies
    printf "This will install multiple libraries to your system, of various licenses including LGPL.  This may pull development versions,"
    printf "and it is therefore inadvisable to use in Production setups."
    printf "Please be sure to review the dependency page found at http://crazyivan.readthedocs.io/en/latest/pages/deps.html"
    printf "You may find the source code for this application, and the licenses for all dependencies at https://github.com/AO-StreetArt/CrazyIvan"
    printf "Do you wish to continue? [y/n]:"
    read deps_confirm
    if [deps_confirm = 'y']; then
      printf "Installing Dependencies\n"
      sudo ./scripts/build_deps.sh
    fi
  fi

  printf "Installing CrazyIvan\n"

  # Install the executable to the bin folder
  sudo cp crazy_ivan /usr/local/bin

  # Copy the stop_crazyivan script
  sudo cp scripts/stop_crazyivan.py /usr/local/bin

  # Copy the systemd unit file
  sudo cp crazyivan.service /etc/systemd/system

  # Create the crazyivan user and group
  sudo useradd --system --user-group crazyivan

  # Build the logging directory
  sudo mkdir /var/log/crazyivan

  # Create the config directory and copy configuration files
  sudo mkdir /etc/crazyivan
  sudo cp log4cpp.properties /etc/crazyivan
  sudo cp ivan.properties /etc/crazyivan

  # Ensure that the crazyivan user has permission to access both the configuration and logging directories
  sudo chown -R crazyivan:crazyivan /etc/crazyivan
  sudo chown -R crazyivan:crazyivan /var/log/crazyivan

  exit 0

fi

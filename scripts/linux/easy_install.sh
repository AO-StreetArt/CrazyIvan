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
  rm /usr/bin/crazy_ivan
  rm -r /etc/crazyivan
  rm -r /var/log/crazyivan
  deluser crazyivan
  groupdel crazyivan
  rm /etc/systemd/system/crazyivan.service
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
      ./scripts/build_deps.sh
    fi
  fi

  printf "Installing CrazyIvan\n"
  make install

  # Create the crazyivan user and group
  useradd --system --user-group crazyivan

  # Ensure that the crazyivan user has permission to access both the configuration and logging directories
  mkdir -p /etc/crazyivan
  mkdir -p /var/log/crazyivan
  chown -R crazyivan:crazyivan /etc/ivan
  chown -R crazyivan:crazyivan /var/log/crazyivan
  chown -R crazyivan:crazyivan /var/crazyivan

  sudo cp crazy_ivan /usr/bin/
  sudo cp app.properties /etc/ivan

  exit 0

fi

#!/bin/bash
#This script will install the pre-packaged release of AOSSL

#Pull the command line parameters
#The generated tar file will have the name RELEASE_NAME-RELEASE_VERSION.tar.gz
#The dependencies script in the folder specified in the third option will be packaged with the library
if [ "$#" -ne 3 ]; then
    echo "The correct usage of this script is './release_gen.sh release-name release-version dependency-script-location'\n"
    exit 64
fi
RELEASE_NAME=$1
RELEASE_VERSION=$2
DEPS_SCRIPT_LOC=$3

printf "Generating CrazyIvan Release: $RELEASE_NAME - $RELEASE_VERSION\n"
printf "Dependency Script Location: $DEPS_SCRIPT_LOC\n"

printf "Building library\n"
cd ../.. && make

#Generate the release folder & subfolders
printf "Generating Release Folders\n"
mkdir ../$RELEASE_NAME
mkdir ../$RELEASE_NAME/scripts
mkdir ../$RELEASE_NAME/scripts/linux
mkdir ../$RELEASE_NAME/release
mkdir ../$RELEASE_NAME/licenses

#Copy the dependency licenses into the licenses folder
printf "Copying Dependency Licenses\n"
cp licenses/*.txt ../$RELEASE_NAME/licenses

#Copy the correct dependency script into the release subfolder 'deps'
printf "Copying Library Elements\n"
cp scripts/$DEPS_SCRIPT_LOC/build_deps.sh ../$RELEASE_NAME/scripts

#Copy the license & notice
cp LICENSE.txt ../$RELEASE_NAME

#Copy the easy install script into the release folder
cp scripts/linux/easy_install.sh ../$RELEASE_NAME

#Copy the populate consul script into the release scripts folder
cp scripts/linux/populate_consul.sh ../$RELEASE_NAME/scripts/linux

#Copy the populate consul script into the release scripts folder
cp scripts/linux/ivan_controller.sh ../$RELEASE_NAME/scripts/linux

#Copy the systemd unit file
cp crazyivan.service ../$RELEASE_NAME

#Copy the executable
cp crazy_ivan ../$RELEASE_NAME

#Copy the configuration file
cp app.properties ../$RELEASE_NAME/app.properties
cp ssl.properties ../$RELEASE_NAME/ssl.properties

#Copy the Release Notes file
cp release/RELEASE_NOTES.md ../$RELEASE_NAME/RELEASE_NOTES.md

#Generate the Release tar files
printf "Generate Release Tar Files\n"
tar -czvf $RELEASE_NAME-$RELEASE_VERSION.tar.gz ../$RELEASE_NAME
tar -cjvf $RELEASE_NAME-$RELEASE_VERSION.tar.bz2 ../$RELEASE_NAME
zip -r $RELEASE_NAME-$RELEASE_VERSION.zip ../$RELEASE_NAME

#Move the tar files outside the main git repository
mv *.tar.* ../
mv *.zip ../

printf "Script finished, release folders can be found at ../../../$RELEASE_NAME, tar files at ../../..\n"

exit 0

.. _devnotes:

Developer Notes
===============

This page contains a series of notes intended to be beneficial for any contributors to Crazy Ivan.

Running Test Cases
------------------
Building the tests can be done with:

``make test``

Tests cases are run using Catch2 (https://github.com/catchorg/Catch2), a few examples are shown below:

Run all tests:

``./tests/tests``

Run only the unit tests:

``./tests/tests [unit]``

Run only the integration tests:

``./tests/tests [integration]``

Development Docker Image
------------------------
Generating a development Docker Image is made easy by the DebugDockerfile.
This image is unique in that it does not enter directly into Crazy Ivan, but
rather installs all of the necessary dependencies and then waits.

First, execute the below command from the root folder of the project to build your local debug image:
``docker build --no-cache --file DebugDockerfile -t "aostreetart/crazyivan:debug" .``

Once this completes, run your image with the below command:
``docker run --name crazyivan -p 5555:5555 -d aostreetart/crazyivan:debug``

You can update the port number to whatever you like, and keep in mind that you may
also need to connect the container to a docker network, depending on your configuration.
For example:
``docker run --name crazyivan --network=dvs -p 5555:5555 -d aostreetart/crazyivan:debug``

Finally, you can open up a terminal within the box with:
``docker exec -i -t crazyivan /bin/bash``

The container will have Crazy Ivan and all it's dependencies pre-installed, so you can get right to work!

Generating Releases
-------------------

The release_gen.sh script is utilized to generate releases for various systems.
It accepts three command line arguments:
* the name of the release: crazyivan-*os_name*-*os_version*
* the version of the release: we follow `semantic versioning <http://semver.org/>`__
* the location of the dependency script: current valid paths are linux/deb (uses apt-get) and linux/rhel (uses yum)

:ref:`Read About Crazy Ivan Automated Testing <tests>`

:ref:`Go Home <index>`

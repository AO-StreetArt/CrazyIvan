.. _quickstart:

Getting Started with CrazyIvan
==============================

:ref:`Go Home <index>`

Building from Source
--------------------

The recommended system for development of CrazyIvan is either
Ubuntu 18.04 or CentOS7.  You will need gcc 5.0 or greater installed to
successfully compile the program.

``git clone https://github.com/AO-StreetArt/CrazyIvan.git``

``mkdir crazyivan_deps``

``cp CrazyIvan/scripts/deb/build_deps.sh crazyivan_deps/build_deps.sh``

``cd crazyivan_deps``

``./build_deps.sh``

You will be asked once for your sudo password.  You will also need to ensure that
the POCO dependency is on the linker path, which can be done with:

`export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"`

Now, we can build Crazy Ivan:

``cd ../CrazyIvan``

``make``

This will result in creation of the crazy_ivan executable, which we can run
with the below command:

``./crazy_ivan``

When not supplied with any command line parameters, CrazyIvan will look for an app.properties file to start from.

You may also build the test executable in the tests/ directory with:

``make tests``

In order to run CrazyIvan from a properties file, you will need:

-  A Neo4j Server installed locally.  Instructions
   can be found at https://neo4j.com/developer/get-started/

Neo4j can be started via a Docker image:

`docker run -d --publish=7474:7474 --publish=7687:7687 --env=NEO4J_AUTH=none --volume=$HOME/neo4j/data:/data --name=database neo4j`

Continue on to the :ref:`Configuration Section <configuration>` for more details
on the configuration options available when starting CrazyIvan.

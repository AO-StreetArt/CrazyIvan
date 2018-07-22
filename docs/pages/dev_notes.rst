.. _devnotes:

Developer Notes
===============

This page contains a series of notes intended to be beneficial for any contributors to Crazy Ivan.

Vagrant
-------
We provide a Vagrantfile to setup a development environment, but this requires
that you install `Vagrant <(https://www.vagrantup.com/)>`__.
Once you have Vagrant installed, cd into the main directory and run:

.. code-block:: bash

   vagrant up

Once the box starts, you can enter it with:

.. code-block:: bash

   vagrant ssh

The Project folder on your machine is synced to the /vagrant folder in the VM, so you will
need to move there before building.  Once in that folder, you can build the executable and tests:

.. code-block:: bash

   make && make test

Packer
------

A Packer file is provided, which can be used with
`Hashicorp Packer <https://www.packer.io>`__.  Configuration is provided for
building a Docker Image, which can be executed with:

.. code-block:: bash

   packer build packer.json

This will create a tagged image, which can then be pushed with

.. code-block:: bash

   docker push aostreetart/crazyivan:v2

Docker
------

The `Crazy Ivan Docker Hub Repository <https://hub.docker.com/r/aostreetart/crazyivan/>`__
contains the latest Docker images for Crazy Ivan.

Running Test Cases
------------------
Building the tests can be done with:

.. code-block:: bash

   make test

Tests cases are run using Catch2 (https://github.com/catchorg/Catch2), a few examples are shown below:

Run all tests:

.. code-block:: bash

   ./tests/tests

Run only the unit tests:

.. code-block:: bash

   ./tests/tests [unit]

Run only the integration tests:

.. code-block:: bash

   ./tests/tests [integration]

Continuous Integration
----------------------

Travis CI is used to run automated tests against Crazy Ivan each time a commit
or pull request is submitted against the main repository.  The configuration for
this can be updated via the .travis.yml file in the main folder of the project
repository.

`Latest CI Runs <https://travis-ci.org/AO-StreetArt/CrazyIvan/>`__

Documentation
-------------

Documentation is built using Sphinx and hosted on Read the Docs.

Updates to documentation can be made in the docs/ folder of the project
repository, with files being in the .rst format.

Generating Releases
-------------------

The release_gen.sh script is utilized to generate releases for various systems.
It accepts three command line arguments:
* the name of the release: crazyivan-*os_name*-*os_version*
* the version of the release: we follow `semantic versioning <http://semver.org/>`__
* the location of the dependency script: current valid paths are linux/deb (uses apt-get) and linux/rhel (uses yum)

:ref:`Go Home <index>`

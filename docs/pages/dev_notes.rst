.. _devnotes:

Developer Notes
===============

This page contains a series of notes intended to be beneficial for any contributors to Crazy Ivan.

Vagrant
-------
We provide a Vagrantfile to setup a development environment, but this requires that you install [Vagrant](https://www.vagrantup.com/).
Once you have Vagrant installed, cd into the main directory and run:

`vagrant up`

Once the box starts, you can enter it with:

`vagrant ssh`

The Project folder on your machine is synced to the /vagrant folder in the VM, so you will
need to move there before building.  Once in that folder, you can build the executable and tests:

`make && make test`

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

Generating Releases
-------------------

The release_gen.sh script is utilized to generate releases for various systems.
It accepts three command line arguments:
* the name of the release: crazyivan-*os_name*-*os_version*
* the version of the release: we follow `semantic versioning <http://semver.org/>`__
* the location of the dependency script: current valid paths are linux/deb (uses apt-get) and linux/rhel (uses yum)

:ref:`Read About Crazy Ivan Automated Testing <tests>`

:ref:`Go Home <index>`

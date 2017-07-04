.. devnotes:

Developer Notes
===============

This page contains a series of notes intended to be beneficial for any contributors to Crazy Ivan.

Generating Releases
-------------------

The release_gen.sh script is utilized to generate releases for various systems.
It accepts three command line arguments:
- the name of the release: crazyivan-*os_name*-*os_version*
- the version of the release: we follow [semantic versioning](http://semver.org/)
- the location of the dependency script: current valid paths are linux/deb (uses apt-get) and linux/rhel (uses yum)

:ref:`Read About Crazy Ivan Automated Testing <tests>`

:ref:`Go Home <index>`

.. dependencies:

Dependencies
============

:ref:`Go Home <index>`

CrazyIvan is built on top of the work of many others, and here you will find information
on all of the libraries and components that CrazyIvan uses to be successful.

Licenses for all dependencies can be found in the licenses folder within the repository.


RapidJson
---------
`RapidJson <https://github.com/miloyip/rapidjson>`__ is a very fast JSON parsing/writing library.

`RapidJson <https://github.com/miloyip/rapidjson>`__ is released under an MIT License.


AO Shared Service Library
-------------------------
`AOSSL <https://github.com/AO-StreetArt/AOSharedServiceLibrary>`__ is a collection
of C++ wrappers on many of the C libraries listed here.

`AOSSL <https://github.com/AO-StreetArt/AOSharedServiceLibrary>`__ is released under an MIT License.


NeoCpp
--------
`NeoCpp <https://github.com/AO-StreetArt/NeoCpp>`__ is a wrapper on
`LibNeo4j <https://github.com/cleishm/libneo4j-client/>`__, which is used to
communicate with Neo4j, a Graph Based Database.

`NeoCpp <https://github.com/AO-StreetArt/NeoCpp>`__ is released under an Apache 2 License.
`LibNeo4j <https://github.com/cleishm/libneo4j-client/>`__ is released under an Apache 2 License.


LibUUID
-------
`LibUUID <https://sourceforge.net/projects/libuuid/>`__ is a linux utility for generating Universally Unique ID's.

`LibUUID <https://sourceforge.net/projects/libuuid/>`__ is released under a BSD License.


POCO
----
`The POCO Project <https://pocoproject.org/>`__ is a set of libraries for building networked C++ applications.

It is released under the Boost Software License.


Boost
-----
`The Boost Project <https://www.boost.org/>`__ is a set of C++ libraries, that are primarily used for UDP Processing.

It is released under the Boost Software License.

GLM
---
`GLM <https://glm.g-truc.net/0.9.9/index.html>`__ is the OpenGL math library,
and is used to perform transformation calculations between scenes and/or devices.

This is licensed under an MIT license.

Automatic Dependency Resolution
-------------------------------

For Ubuntu 16.04 or Centos7, the build\_deps.sh scripts should allow for
automatic resolution of dependencies.

Developers can utilize the Vagrant image, which will install dependencies in the VM.

End-Users can run the Docker image, which will install dependencies in the container.

Other Acknowledgements
----------------------

Here we will try to list authors of other public domain code that has been used:

::

    René Nyffenegger - Base64 Decoding Methods

.. dependencies:

Dependencies
============

:ref:`Go Home <index>`

CrazyIvan is built on top of the work of many others, and here you will find information
on all of the libraries and components that CrazyIvan uses to be successful.

Licenses for all dependencies can be found in the licenses folder within the repository.

ZeroMQ
------
`Zero MQ <http://zeromq.org/>`__ is a lightweight messaging library that CrazyIvan uses to communicate.  It is
fast, versatile, and has bindings for many major languages.

`Zero MQ <http://zeromq.org/>`__ is released under an LGPL License.


CppZmq
------
`CppZmq <https://github.com/zeromq/cppzmq>`__ is the C++ binding for libzmq, which was written in C.

`CppZmq <https://github.com/zeromq/cppzmq>`__ is released under an MIT License.


Log4cpp
-------
`Log4Cpp <http://log4cpp.sourceforge.net/>`__ is a logging library based on Log4j.

`Log4Cpp <http://log4cpp.sourceforge.net/>`__ is released under an LGPL License.


Eigen
-----
`Eigen <http://eigen.tuxfamily.org/index.php?title=Main_Page>`__ is a Linear Algebra library.

`Eigen <http://eigen.tuxfamily.org/index.php?title=Main_Page>`__ is released under an MPL License.


RapidJson
---------
`RapidJson <https://github.com/miloyip/rapidjson>`__ is a very fast JSON parsing/writing library.

`RapidJson <https://github.com/miloyip/rapidjson>`__ is released under an MIT License.


AO Shared Service Library
-------------------------
`AOSSL <https://github.com/AO-StreetArt/AOSharedServiceLibrary>`__ is a collection
of C++ wrappers on many of the C libraries listed here.

`AOSSL <https://github.com/AO-StreetArt/AOSharedServiceLibrary>`__ is released under an MIT License.


LibHiredis
----------
`LibHiredis <https://github.com/redis/hiredis>`__ is used to communicate with Redis, a distributed key-value store.

`LibHiredis <https://github.com/redis/hiredis>`__ is released under a BSD License.


LibNeo4j
--------
`LibNeo4j <https://github.com/cleishm/libneo4j-client/>`__ is used to communicate with Neo4j, a Graph Based Database.

`LibNeo4j <https://github.com/cleishm/libneo4j-client/>`__ is released under an Apache 2 License.


LibUUID
-------
`LibUUID <https://sourceforge.net/projects/libuuid/>`__ is a linux utility for generating Universally Unique ID's.

`LibUUID <https://sourceforge.net/projects/libuuid/>`__ is released under a BSD License.


LibCurl
-------
`LibCurl <https://curl.haxx.se/libcurl/>`__ is a ubiquitous networking library.

`LibCurl <https://curl.haxx.se/libcurl/>`__ is released under an MIT License.


LibProtobuf
-----------
LibProtobuf and the Protocol Buffer Compiler comprise a serialization system which
CrazyIvan can use to communicate in lieu of JSON.  You can find more information about
Protocol Buffers at `the Google Developer Site <https://developers.google.com/protocol-buffers>`__

The Protocol Buffer License is unique yet very unrestrictive.  For more information please see the
`license itself <https://github.com/google/protobuf/blob/master/LICENSE>`__


DVS Interface
-------------
Finally, we also depend on the `DVS Interface Library <https://github.com/AO-StreetArt/DvsInterface>`__
which houses a collection of .proto files for this project.

`DVS Interface <https://github.com/AO-StreetArt/DvsInterface>`__ is released under an MIT License.


Automatic Dependency Resolution
-------------------------------

For Ubuntu 16.04 & Debian 7, the build\_deps.sh script should allow for
automatic resolution of dependencies.

Other Acknowledgements
----------------------

Here we will try to list authors of other public domain code that has been used:

::

    René Nyffenegger - Base64 Decoding Methods

Crazy Ivan
==========

.. figure:: https://travis-ci.org/AO-StreetArt/CrazyIvan.svg?branch=master
   :alt:

Overview
--------

Crazy Ivan is a service designed to store 'scenes', with relationships
between them. These are relationships between coordinate systems, to
which each scene is associated. This allows for establishing
relationships between devices in real-space.

Features
--------

-  Storage of Scenes (Groups of virtual objects & user devices associated to a latitude/longitude)
-  Efficient calculation of coordinate system transformations based on existing data
-  Means to store manual corrections from users
-  Connect to other services over Zero MQ using Google Protocol Buffers.
-  Scalable microservice design

Crazy Ivan is a part of the AO Distributed Visualization Server, along
with `CLyman <https://github.com/AO-StreetArt/CLyman>`__. It therefore
utilizes the `DVS Interface
library <https://github.com/AO-StreetArt/DvsInterface>`__, also
available on github. It utilizes the Scene.proto file for inbound
communications.

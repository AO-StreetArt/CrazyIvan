Crazy Ivan
==========

.. figure:: https://travis-ci.org/AO-StreetArt/CrazyIvan.svg?branch=master
   :alt:

Overview
--------

Crazy Ivan is a service designed to store 'scenes', which means an arbitrary collection
of objects in 3-space within a geographic area.  Devices can register/de-register
from scenes as they move through the world, and as they do we build a network of
relationships that can be used to determine the transformations needed for other devices.

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

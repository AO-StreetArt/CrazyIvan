Crazy Ivan
==========

.. figure:: https://travis-ci.org/AO-StreetArt/CrazyIvan.svg?branch=master
   :alt:

Overview
--------

Crazy Ivan aims to solve several problems necessary to enabling Augmented Reality
experiences that multiple individuals can interact with.  It is one critical piece
in joining the digital world and the physical world together seamlessly.  On top
of this unique functionality is a robust, reliable, and secure core, capable
of enabling multi-user animation, design, gaming, and more.  Crazy Ivan is up to
the challenge of both the digital and physical realms, and that makes it unique.

This is a service designed to store 'Scenes'.  A Scene is a collection
of Renderable Objects (possibly associated to some geographic coordinate),
which users can move between.  These scenes can be anything, ranging from
levels in a traditional video game, to apartments in a building.

Devices can join and leave scenes as they move through the world, and as they do
we build a network of relationships that can be used to determine the transformations
needed for other devices.  Crazy Ivan also serves as a UDP Server, communicating
Updates out to registered devices.

As you decide on the foundation for your next application, you should choose one
that will grow with you, both in terms of scale and in terms of technology.
It's time to stop settling for 'good enough' when it comes to interactive, shared
visuals.

Detailed documentation can be found on `ReadTheDocs <http://crazyivan.readthedocs.io/en/latest/index.html>`__.

Features
--------

-  Storage of Scenes (Groups of virtual objects & user devices)
-  Track User Devices moving between Scenes to create a connected network of coordinate systems
-  Means to store manual corrections of coordinate system transformations from users
-  Efficient calculation of coordinate system transformations between Scenes based on existing data
-  Stream updates via UDP to Registered User Devices
-  Multi-layered security
-  Scalable and Flexible Deployment Strategies

Crazy Ivan is a part of the AO Aesel Project, along
with `CLyman <https://github.com/AO-StreetArt/CLyman>`__.

Stuck and need help?  Have general questions about the application?  Reach out to the development team at crazyivan@emaillist.io

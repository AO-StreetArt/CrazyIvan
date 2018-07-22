.. _architecture:

Architecture
============

In order to allow for real-time, distributed visualization, one of the
key problems that needs to be solved is ensuring that coordinate systems
between various user devices and objects are synchronized.  A key abstraction in
this case is a 'Scene' which is an arbitrary collection of objects and devices.
A device can register/de-register from any scene, as well as apply corrections
to the coordinate system relationship between it and the scene it's registered to.

This is done by storing relationships between scenes and devices, and then using
these to build relationships between scenes themselves.  When devices move between
these scenes, they will apply corrections.  As they apply corrections, we will
build a set of known mappings between scenes which should allow users to move without
needing any corrections by returning the pre-calculated differences.

Technical Overview
------------------

Crazy Ivan is designed to be used as a service within a larger
architecture. It will take in CRUD messages for scenes, as well as
track user device registrations (both over HTTP).

Running Crazy Ivan requires an instance of `Neo4j <http://www.neo4j.com/>`__
to connect to in order to perform most functions.  Neo4j serves as the
back-end database for Crazy Ivan.

Crazy Ivan can also be deployed with `Consul <https://www.consul.io/>`__
as a Service Discovery and Distributed Configuration architecture. This
requires the `Consul Agent <https://www.consul.io/downloads.html>`__ to
be deployed that Crazy Ivan can connect to.

Crazy Ivan can be deployed securely using `Vault <https://www.vaultproject.io>`__
as a secret store and/or intermediate CA.

Object Change Streams (Events)
------------------------------

Object Change Streams ensure that all registered User Devices remain up to date about
objects within their scenes.  Crazy Ivan receives UDP updates from outside sources,
with a specific format, and then forwards the message, once again via UDP, to all
registered devices.

The changes streams are designed to be high-speed and high-volume.  Crazy Ivan
can process many messages in parallel, and registration information is kept up-to-date
in a cache for immediate retrieval.  A separate background thread periodically loads
updated values from Neo4j.

Clustering
----------

Scene-specific clustering is a central idea in Crazy Ivan.  This is an idea
borrowed from large-scale MMORPG's, in which large maps are broken apart and
each piece is run by separate servers.  This allows for horizontal scaling of
the system to cover additional real-estate, physical or digital.

A cluster name can be provided by Crazy Ivan on startup, and other applications
should use this cluster name to identify the appropriate Crazy Ivan to send
messages to.

Note that Crazy Ivan clusters using different Neo4j clusters will not be able
to store or calculate cross-scene transformations for coordinate systems.

:ref:`Go Home <index>`

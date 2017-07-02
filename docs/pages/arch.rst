.. _architecture:

Architecture
============

This is designed to be used as a microservice within a larger
architecture. This will take in CRUD messages for scenes, as well as
track user device registrations.

A .proto file is included to allow generating the bindings for any
language (the [protocol buffer compiler]
(https://developers.google.com/protocol-buffers/) is installed by the
build\_deps script), which can be used to communicate via protocol
buffers.

Please note that running Crazy Ivan requires an instance of both
`Neo4j <http://www.neo4j.com/>`__ and `Redis 3.2.0 <http://redis.io/>`__
to connect to in order to run.

Crazy Ivan can also be deployed with `Consul <https://www.consul.io/>`__
as a Service Discovery and Distributed Configuration architecture. This
requires the `Consul Agent <https://www.consul.io/downloads.html>`__ to
be deployed that Crazy Ivan can connect to.

Design
======

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

:ref:`Go Home <index>`

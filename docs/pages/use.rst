.. _use:

How to Use Crazy Ivan
=====================

Overview
--------

Use of Crazy Ivan generally boils down to 5 steps:

1. Build Scenes
2. Calibrate Scenes
3. Start Event Streams
4. Register Devices
5. Stream Events

A Scene is a group of objects, that may or may not be associated to a
latitude/longitude point.  The updates in each scene are made with respect
to a single coordinate-system, known as the Scene Coordinate System.  There
is no single, universal, master coordinate system, but rather these are
defined by the relationships between each scene.  Once a device has aligned
it's coordinate system with that of a single scene, it can move to any other
which has a pre-defined relationship.  This act of moving between scenes is
known as 'registration'.

Event Streams are streams of UDP messages sent from Crazy Ivan to registered
devices.  Crazy Ivan receives a single UDP message against a scene, and then
broadcasts that message out to any devices that need it.

These concepts allow us to build a real-time, collaborative system for use-cases
like collaborative animation, gaming, and Augmented Reality applications.

Build Scenes
------------

This task is the starting point for any workflow, and can be
accomplished using the :ref:`Scene API <scene_api>`.

We can use this API to create all of the Scenes we will need.  What scenes
we create will vary wildly by use case.  For gaming-like applications,
each Scene can represent a single level within the game, a single multi-player
lobby within the game, or a section of map within a large-scale MMORPG.  For
augmented-reality applications, a single scene can represent anything from an
individual's apartment to a theatre.

Calibrate Scenes
----------------

This is an optional step where calibration devices are moved between scenes
prior to registration of actual devices.  This allows the scene network to be
generated prior to devices moving between scenes, guaranteeing that actual devices
will be able to move between scenes accurately.

Alternatively, end users can provide the calibration during live-use.  Both
approaches have drawbacks and benefits, but it is up to the application
developers which method to prefer/utilize.

Either way, this is accomplished via the :ref:`Registration API <scene_registration_api>`.

Start Event Streams
-------------------

Prior to using a Crazy Ivan server, each cluster should be assigned particular
Scenes to manage the event stream for.  This is done using the :ref:`Cache API <cache_api>`.

This allows us to ensure that specific instances of Crazy Ivan are streaming to
particular groups of devices, so that we can ensure minimal latency in the
transmission.  It also allows Crazy Ivan to cache scene and device information
in-memory, rather than having to hit the database for each Event.

Register Devices
----------------

During initial device registration for end-user devices, they may need to
calibrate their initial coordinate system with the scene coordinate system.
Note that this is only true, in general, for Augmented Reality applications.

Regardless, once devices are first registered, they can then further use the
:ref:`Registration API <scene_registration_api>` to move between scenes.  As
they move, they will receive Event Streams for the scenes they are registered to.

As devices move between scenes, they may be moving between different coordinate
systems, especially in the case of Augmented Reality.  This is where the network
we created during calibration comes into play: each device is provided all of
the information it needs to resolve the differences between the coordinate
systems when it registers for another.

Stream Events
-------------

Finally, devices can move about freely between scenes, sending Events which
are consumed by other components, and passed to Crazy Ivan for final routing.
This is done via the :ref:`Event Stream API <event_stream_api>`

Because of this, Crazy Ivan does not care what is actually in an 'Event'.  It
only reads the start of the message to find the scene, and beyond that it can
contain any text-based message you desire.

Event Streams are designed to be as fast as possible.  Communication of events
is limited to UDP and/or shared memory, and events can be restricted to
specific clusters of Crazy Ivan and other components to ensure minimal network
latency.

Optionally, Event Streams can also utilize AES symmetric-encryption, to make
sure that the live updates cannot be read by prying eyes.

:ref:`Go Home <index>`

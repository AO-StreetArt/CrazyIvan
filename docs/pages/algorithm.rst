.. _algorithm:

Algorithm Design
================

Overview
~~~~~~~~

Crazy Ivan stores Scenes and User Devices as nodes in Neo4j, and Transformations
(ie. Translation & Rotation) as the edges between nodes.  The creation of this
network of nodes and edges is the basis of how Crazy Ivan is able to calculate
relationships between arbitrary scenes.

The algorithm has two core components:

1. Network Creation
2. Transformation Calculation

Network Creation
~~~~~~~~~~~~~~~~

We build a scene-scene link when we receive a Scene Synchronization message.
This corresponds to a device-supplied correction of the transformation between
it's local coordinate system and the specified scene (scene A) coordinate system.

Upon receipt of this message, we check for other scenes the device is registered
to and, for each scene we find (scene B), we either:

* Create a new scene-scene link between scene A and scene B.
* Overwrite an existing scene-scene link between scene A and scene B.

The scene-scene transform is calculated as:

.. math:: (Device-Scene Transform)_A * (Device-Scene Transform)_B^-1

with both links represented as 4x4 matrix transformations.  The direction
of this transformation is from Scene A to Scene B.

Proof
-----

Given coordinate systems A, B, and C, and transformations T:subscript:`B` and
T:subscript:`C` such that, for any point a in A, a can be represented as a
point b in B by:

.. math:: b = T_B * a

and a can also be represented as a point c in C by:

.. math:: c = T_C * a

Apply matrix multiplication to equation (2):

.. math:: a = T_B^-1 * b

Using substitution, we find that:

.. math:: c = T_C * T_B^-1 * b

Therefore, any point b in B can be represented as a point c in C by applying
the transform :math:`T_C * T_B^-1`.

Transformation Calculation
~~~~~~~~~~~~~~~~~~~~~~~~~~

When a device moves from scene A to scene B, it requires a transformation
that allows it to move from the first scene to the second.  In order to find
this, Crazy Ivan uses a shortest-path algorithm (built in to Neo4j) to find
a route from the node for scene A to the node for scene B, traversing the
transformations between them.  The resulting series of transformations is then:

* Inverted if necessary, to ensure the same direction on all transformations
* Converted to matrix representation
* multiplied together

This yields a final transformation which can move directly from scene A
to scene B.

Proof
-----

Given coordinate systems A, B, and C, and transformations T:subscript:`B` and
T:subscript:`C` such that, for any point a in A, a can be represented as a
point b in B by:

.. math:: b = T_A * a

and any point b in B can be represented as a point c in C by:

.. math:: c = T_B * b

Then, using simple substitution, we find that:

.. math:: c = T_B * T_A * a

Therefore, any point a in A can be represented as a point c in C by applying
the transform :math:`T_B * T_A`.

:ref:`Go Home <index>`

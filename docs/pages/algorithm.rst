.. _algorithm:

Algorithm Design
================

Overview
~~~~~~~~

Anyone visiting this page without any prior knowledge of Crazy Ivan should visit
both the :ref:`Main Page <index>` and the :ref:`Usage Page <use>` in order to
become familiar with the basic purpose and functionality of the system.

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

Given translation :math:`T_A` from the device to Scene A, and translation :math:`T_B`
from the device to Scene B, the scene-scene translation is calculated as:

.. math:: T_B * T_A^{-1}

Given local rotation :math:`R_A` from the device to Scene A, and local
rotation :math:`R_B` from the device to Scene B, the scene-scene rotation
is calculated as:

.. math:: R_A^{-1} * R_B

with all links represented as 4x4 matrix transformations.  The direction
of this transformation is from Scene A to Scene B.

Proof
-----

Given coordinate systems A, B, and C, translations :math:`T_B` and
:math:`T_C`, and rotations :math:`R_B` and :math:`R_C` such that,
for any point a in A, a can be represented as a point b in B by:

.. math:: b = T_B * a * R_B

and a can also be represented as a point c in C by:

.. math:: c = T_C * a * R_C

Apply matrix multiplication to equation (3):

.. math:: a = T_B^{-1} * b * R_B^{-1}

Using substitution, we find that:

.. math:: c = T_C * T_B^{-1} * b * R_B{-1} * R_C

Transformation Calculation
~~~~~~~~~~~~~~~~~~~~~~~~~~

When a device registers to a scene (Scene B), Crazy Ivan checks to see if there are any
pre-existing registrations to other scenes for that device.  If there are, then
Crazy Ivan will take the first scene it finds (Scene A) and use it to calculate a
transformation that allows it to move from Scene A to Scene B.  In order to find
this, Crazy Ivan uses a shortest-path algorithm (built in to Neo4j) to find
a route from the node for scene A to the node for scene B, traversing the
transformations between them.  The resulting series of transformations is then:

* Inverted if necessary, to ensure the same direction on all transformations
* Converted to matrix representation
* multiplied together (locations using LHS multiplication, rotations with RHS multiplication)

This yields a final transformation which can move directly from scene A
to scene B.

Proof
-----

Given a positive integer N > 2 and another positive integer 1 < n < N, we have
a series of Scenes :math:`S_1, S_2, ..., S_N`, connected by a series of
Transforms :math:`T_1, T_2, ..., T_{N-1}`, each containing a
translation :math:`T_N(tr)` and a rotation :math:`T_N(rot)`.  We know that an
element :math:`s_n` in :math:`S_n` can be represented as an element
:math:`s_{n+1}` in :math:`S_{n+1}` by:

.. math:: s_{n+1} = T_n(tr) * s_n * T_n(rot)

Given another positive integer m such that m + n < N, we will show by induction
that:

.. math:: s_{n+m} = T_{n+m-1}(tr) * T_{n+m-2}(tr) * ... * T_n(tr) * s_n * T_n(rot) * ... * T_{n+m-2}(rot) * T_{n+m-1}(rot)

For our base case, we will consider that n = 1.  In this case, equation 7
simplifies to:

.. math:: s_2 = T_1(tr) * s_1 * T_1(rot)

Equation 8 simplifies to:

.. math:: s_{m+1} = T_{m}(tr) * T_{m-1}(tr) * ... * T_1(tr) * s_1 * T_1(rot) * ... * T_{m-1}(rot) * T_{m}(rot)

We can also prove this by induction.  Our base case will be m=1, in which case
the above simplifies to:

.. math:: s_2 = T_1(tr) * s_1 * T_1(rot)

This is the same as equation 9, which we already know to be true in this case.

Given some k < m, we assume that:

.. math:: s_k = T_k(tr) * T_{k-1}(tr) * ... * T_1(tr) * s_1 * T_1(rot) * ... * T_{k-1}(rot) * T_k(rot)

Then, by equation 7, we know that:

.. math:: s_{k+1} = T_k(tr) * s_k * T_k(rot)

Using substitution, we find that:

.. math:: s_{k+1} = T_k(tr) * T_{k-1}(tr) * ... * T_1(tr) * s_1 * T_1(rot) * ... * T_{k-1}(rot) * T_k(rot)

Now, we have proven the base case of our inductive argument.  We can now assume
that, for some j < n, that the following is true:

.. math:: s_{j+m} = T_{j+m-1}(tr) * T_{j+m-2}(tr) * ... * T_j(tr) * s_j * T_j(rot) * ... * T_{j+m-2}(rot) * T_{j+m-1}(rot)

By Equation 7, we know that:

.. math:: s_{(j+m)+1} = T_{j+m}(tr) * s_{j+m} * T_{j+m}(rot)

Using substitution, we find that:

.. math:: s_{j+m+1} = T_{j+m}(tr) * T_{j+m-1}(tr) * ... * T_j(tr) * s_j * T_j(rot) * ... * T_{j+m-1}(rot) * T_{j+m}(rot)

This concludes our inductive proof, as the above equation is the same as Equation 8.

:ref:`Go Home <index>`

.. _api:

API Overview
============

The CrazyIvan API utilizes either JSON or Protocol Buffers, based on
what the server is configured to process. In either case, the field
names and message structure remains the same. This document will focus
on the JSON API, but with this knowledge and the DVS Interface Protocol
Buffer files, the use of the Protocol Buffer API should be equally
clear.

Response Messages follow the same format as inbound messages.

To start with, here is an example JSON message which will create a
single scene, and register a user device to it:

{

 "msg\_type":4,

 "transaction\_id":"123465",

 "scenes":[

 {

 "key":"jklmnop",

 "name":"Test Scene 10",

 "latitude":124.0,

 "longitude":122.0,

 "distance":100.0,

 "devices":[

 {

 "key":"Ud\_132",

 "transform":{

 "translation":[1.0,1.0,1.0],

 "rotation":[1.0,1.0,1.0]

 }

 }

 ]

 }

 ]

}

Let’s take a look at the individual fields.

Scene List
----------

The Scene List is the highest level wrapper in the API. It only contains
5 keys, one of which is an array of scenes.

-  msg\_type – 0 for create scene, 1 for update scene, 2 for
   retrieve/query scene(s), 3 for delete scene, 4 for device
   registration, 5 for device de-registration, and 6 for device
   alignment. The message type applies to all objects in the objects
   array.
-  transaction\_id – An ID to distinguish a transaction within a larger
   network of applications
-  scenes – An array containing scenes
-  err\_code – Integer error code, full list of values can be found
   below in the appendix
-  err\_msg – A string error message, containing a human-readable
   description of the issue

Scene
-----

A single Scene , is represented by a single element of the array from
the “scenes” key of the scene list.

-  key – Scene Key value (UUID)
-  name – Name of the Scene
-  latitude – A float value representing the latitude of the Scene. Used
   for distance-based queries.
-  longitude – A float value representing the longitude of the Scene.
   Used for distance-based queries.
-  distance – A float value that is only required for distance based
   queries. With this, we can query Crazy Ivan for scenes within a
   specific distance of a lat/long position.
-  num\_records – An Integer value which represents the maximum number
   of scenes that can be returned from a query to Crazy Ivan
-  devices – ID For the Scene containing the object

User Device
-----------

A single device is represented by a single element of the array from the
“devices” key of the scene.

-  key – Device Key value (UUID)
-  transform – A transformation object which represents the
   transformation from the scene coordinate system to the device
   coordinate system.

Transformation
--------------

A transformation is represented by the object in the “transform” key of
the device.

-  translation – An array of 3 floats representing x, y, and z values
   for a translation
-  rotation – An array of 3 floats representing x, y, and z values for a
   local euler rotation

Field Mapping
=============

+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| Create            | Get              | Update   | Delete   | Register   | De-Register   | Align   |      |      |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| msg\_type         | Integer          | X        | X        | X          | X             | X       | X    | X    |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| err\_code         | Integer          |          |          |            |               |         |      |      |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| err\_msg          | String           |          |          |            |               |         |      |      |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| transaction\_id   | String           | \*       | \*       | \*         | \*            | \*      | \*   | \*   |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| num\_records      | String           |          | \*       |            |               |         |      |      |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| key (scene)       | String           |          | \*       | X          | X             | X       | X    | X    |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| name              | String           | X        | \*       | \*         |               | \*      |      |      |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| latitude          | Float            | X        | \*       | \*         |               | \*      |      |      |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| longitude         | Float            | X        | \*       | \*         |               | \*      |      |      |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| distance          | Float            |          | \*       | \*         |               |         |      |      |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| key (device)      | String           |          |          |            |               | X       | X    | X    |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| translation       | Array - Double   |          |          |            |               | \*      | \*   | \*   |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+
| rotation          | Array - Double   |          |          |            |               | \*      | \*   | \*   |
+-------------------+------------------+----------+----------+------------+---------------+---------+------+------+

X – Required

\* - Optional

Message Types
=============

Scene Create
------------

Create a new Scene. Returns a unique key for the scene.

Scene Retrieve
--------------

The scene retrieve message will retrieve a scene by key, and return the
full scene. It can also be used to run queries against other scene
attributes, as well as perform distance-based queries to find scenes
within a certain radius of a given lat/long coordinate.

Scene Update
------------

Scene updates can be used to update scene attributes. If Atomic
Transactions are enabled, then updates will be processed in the order
they are received, across all instances of Crazy Ivan connected to the
same Redis cluster.

Scene Destroy
-------------

Destroy an existing Scene by key. Basic success/failure response.

Device Register
---------------

Register a device to a scene. If no transformation is supplied, then
CrazyIvan will respond with an initial guess on what the correct
transform is.

Device De-Register
------------------

De-Register a device to a scene.

Device Align
------------

Apply a correction to the transformation currently stored between a
scene and user device.

Appendix A: JSON Message Samples
================================

Inbound
-------

Scene Create
~~~~~~~~~~~~

{

 "msg\_type":0,

 "err\_code":100,

 "err\_msg":"Test",

 "transaction\_id":"123464",

 "scenes":[

 {

 "key":"ijklmno",

 "name":"Test Scene 9",

 "latitude":124.0,

 "longitude":122.0,

 "distance":100.0

 }

 ]

}

Scene Retrieve
~~~~~~~~~~~~~~

{

 "msg\_type":2,

 "transaction\_id":"123465",

 "scenes":[

 {

 "key":"jklmnop"

 }

 ]

}

Scene Update
~~~~~~~~~~~~

{

 "msg\_type":1,

 "err\_code":100,

 "err\_msg":"Test",

 "transaction\_id":"123465",

 "scenes":[

 {

 "key":"jklmnop",

 "name":"Test Scene 101",

 "latitude":126.0,

 "longitude":129.0,

 "distance":110.0

 }

 ]

}

Scene Destroy
~~~~~~~~~~~~~

{

 "msg\_type":3,

 "transaction\_id":"123464",

 "scenes":[

 {

 "key":"ijklmno"

 }

 ]

}

Device Registration
~~~~~~~~~~~~~~~~~~~

{

 "msg\_type":4,

 "err\_code":100,

 "err\_msg":"Test",

 "transaction\_id":"123465",

 "scenes":[

 {

 "key":"jklmnop",

 "name":"Test Scene 10",

 "latitude":124.0,

 "longitude":122.0,

 "distance":100.0,

 "devices":[

 {

 "key":"Ud\_132",

 "transform":{

 "translation":[1.0,1.0,1.0],

 "rotation":[1.0,1.0,1.0]

 }

 }

 ]

 }

 ]

}

Device De-Registration
~~~~~~~~~~~~~~~~~~~~~~

{

 "msg\_type":5,

 "err\_code":100,

 "err\_msg":"Test",

 "transaction\_id":"123465",

 "scenes":[

 {

 "key":"jklmnop",

 "name":"Test Scene 10",

 "latitude":124.0,

 "longitude":122.0,

 "distance":100.0,

 "devices":[

 {

 "key":"Ud\_132",

 "transform":{

 "translation":[1.0,1.0,1.0],

 "rotation":[1.0,1.0,1.0]

 }

 }

 ]

 }

 ]

}

Device Alignment
~~~~~~~~~~~~~~~~

{

 "msg\_type":6,

 "err\_code":100,

 "err\_msg":"Test",

 "transaction\_id":"123465",

 "scenes":[

 {

 "key":"jklmnop",

 "name":"Test Scene 10",

 "latitude":124.0,

 "longitude":122.0,

 "distance":100.0,

 "devices":[

 {

 "key":"Ud\_132",

 "transform":{

 "translation":[6.0,1.0,1.0],

 "rotation":[1.0,45.0,1.0]

 }

 }

 ]

 }

 ]

}

Response
--------

Scene Create
~~~~~~~~~~~~

{"msg\_type":0,"err\_code":100,"scenes":[{"latitude":0.0,"longitude":0.0,"distance":0.0,"devices":[]}]}

Scene Retrieve
~~~~~~~~~~~~~~

{"msg\_type":2,"err\_code":100,"transaction\_id":"123465","scenes":[{"key":"jklmnop","name":"Test
Scene
10","latitude":124.0,"longitude":122.0,"distance":0.0,"devices":[]}]}

Scene Update
~~~~~~~~~~~~

{"msg\_type":1,"err\_code":100,"scenes":[{"key":"jklmnop","latitude":0.0,"longitude":0.0,"distance":0.0,"devices":[]}]}

Scene Destroy
~~~~~~~~~~~~~

{"msg\_type":3,"err\_code":100,"scenes":[{"key":"jklmnop","latitude":0.0,"longitude":0.0,"distance":0.0,"devices":[]}]}

Device Registration
~~~~~~~~~~~~~~~~~~~

{"msg\_type":4,"err\_code":100,"transaction\_id":"123465","scenes":[{"key":"jklmnop","latitude":0.0,"longitude":0.0,"distance":0.0,"devices":[]}]}

Device De-Registration
~~~~~~~~~~~~~~~~~~~~~~

{"msg\_type":5,"err\_code":100,"transaction\_id":"123465","scenes":[{"key":"jklmnop","latitude":0.0,"longitude":0.0,"distance":0.0,"devices":[]}]}

Device Alignment
~~~~~~~~~~~~~~~~

{"msg\_type":6,"err\_code":100,"transaction\_id":"123464","scenes":[{"key":"ijklmno","latitude":0.0,"longitude":0.0,"distance":0.0,"devices":[]}]}

Appendix B: Error Codes
=======================

const int NO\_ERROR = 100

const int ERROR = 101

const int NOT\_FOUND = 102

const int TRANSLATION\_ERROR = 110

const int PROCESSING\_ERROR = 120

const int BAD\_MSG\_TYPE\_ERROR = 121

const int INSUFF\_DATA\_ERROR = 122

:ref:`Go Home <index>`

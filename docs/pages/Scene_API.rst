.. _scene_api:

Scene API
=========

A Scene is a group of Objects associated to a particular Latitude and
Longitude. This API exposes CRUD and Query operations for Scenes.

Scene Creation
~~~~~~~~~~~~~~

.. http:post:: /v1/scene/

   Create a new scene.  A new key will be generated and returned in
   the response.

   :reqheader Content-Type: Application/json
   :statuscode 200: Success

.. include:: _examples/scene/scene_create.rst

Scene Creation
~~~~~~~~~~~~~~

.. http:put:: /v1/scene/(key)

   Create a new scene.  The provided key will be assigned to the scene.

   :reqheader Content-Type: Application/json
   :statuscode 200: Success

.. include:: _examples/scene/scene_create_key.rst


Scene Get
~~~~~~~~~

.. http:get:: /v1/scene/(key)

   Get a Scene by key.

   :statuscode 200: Success

.. include:: _examples/scene/scene_get.rst

Scene Update
~~~~~~~~~~~~

.. http:post:: /v1/scene/(key)

   Update an existing Scene with the specified key.

   :reqheader Content-Type: Application/json
   :statuscode 200: Success

.. include:: _examples/scene/scene_update.rst

Scene Deletion
~~~~~~~~~~~~~~

.. http:delete:: /v1/scene/(key)

   Delete a scene.

   CAUTION: This will delete all information associated to a scene,
   including all objects in the scene, and any registrations to devices.
   Any object which needs to be retained should be moved to another scene
   prior to deletion.

   :statuscode 200: Success

.. include:: _examples/scene/scene_delete.rst

Scene Query
~~~~~~~~~~~

.. http:post:: /v1/scene/query

   Devices can find scenes by any attribute, including distance.

   The fields ‘latitude’, ‘longitude’, and ‘distance’ should always
   appear together if present. The distance provided is taken in
   kilometers.

   :reqheader Content-Type: Application/json
   :statuscode 200: Success

.. include:: _examples/scene/scene_query.rst

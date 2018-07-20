.. _cache_api:

Cache API
=========

The Scene Cache holds registered device information to use for Event Streaming.
This enables minimal-latency streaming, but also necessitates that scenes are
assigned to Crazy Ivan instances before streaming through that instance.

Cache Add
~~~~~~~~~

.. http:put:: /v1/scene/cache/(scene)

   Add a scene to the Scene cache.

   :statuscode 200: Success

Cache Remove
~~~~~~~~~~~~

.. http:delete:: /v1/scene/cache/(scene)

   Remove a scene from the Scene cache.

   :statuscode 200: Success

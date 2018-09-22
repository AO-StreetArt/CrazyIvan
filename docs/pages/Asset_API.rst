.. _asset_api:

Asset API
=========

The Asset API provides a rapid way to add or remove Scene Assets.

Cache Add
~~~~~~~~~

.. http:put:: /v1/scene/(scene)/asset/(asset)

   Add an Asset to a Scene.

   :statuscode 200: Success

Cache Remove
~~~~~~~~~~~~

.. http:delete:: /v1/scene/(scene)/asset/(asset)

   Remove an Asset from a Scene.

   :statuscode 200: Success

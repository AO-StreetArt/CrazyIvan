.. _event_stream_api:

Event API
=========

An event is an update from an external system which needs to be streamed out to
devices registered to a particular scene.  An event is recieved via UDP, and then
sent via UDP.  Scenes should be assigned to an instance of Crazy Ivan via the
Cache API.  This allows for streaming of the updates with minimal latency.

Once an event is registered, it can be sent via UDP.  If configured, it may be
encrypted with an AES-256 symmetric key and salt.  The first line of the
message should be the Scene ID, with the second line of the message conveying
the event.  The lines should be separated only by the newline character.
Crazy Ivan does not decode the event itself, but rather takes the Scene ID
from the first line and uses it to perform streaming to other devices.

This means that the only requirement on the format of the messages is that:
- They start with a valid Scene ID that has been added to the Crazy Ivan cache.
- Then they contain a new line character.
- Then they contain the desired message to send.
- The total, encrypted, message is no more than 275 characters

For example:

``_SCENE_KEY_\n{“key”: “_OBJ_KEY_”, “scene”: “_SCENE_KEY_”, “transform”: [0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000]}``

# Architecture

This is designed to be used as a microservice within a larger architecture.  This will take in CRUD messages for objects in 3 space, and track location, rotation, and scaling.  It will also ensure that any updates are sent out on a separate port to allow streaming to all user devices.

A .proto file is included to allow generating the bindings for any language (the [protocol buffer compiler] (https://developers.google.com/protocol-buffers/) is installed by the build_deps script), which can be used to communicate via protocol buffers.

Please note that running Crazy Ivan requires an instance of both [Neo4j](http://www.neo4j.com/) and [Redis 3.2.0](http://redis.io/) to connect to in order to run.

Crazy Ivan can also be deployed with [Consul](https://www.consul.io/) as a Service Discovery and Distributed Configuration architecture.  This requires the [Consul Agent](https://www.consul.io/downloads.html) to be deployed that Crazy Ivan can connect to.

# Design

In order to allow for real-time, distributed visualization, one of the key problems that needs to be solved is allowing for the sharing of a common set of resources, such as Mesh file formats and shader scripts.  These resources can sometimes be incredibly large, and so a fast and efficient means of retrieval and storage is key.

Crazy Ivan stores data in chunked documents within MongoDB, and pieces them back together to respond to requests.  Crazy Ivan can chunk down large data that is provided, or allow data chunking to be performed in another service/client.

# Next
Now you can move on to the [Deployment] (https://github.com/AO-StreetArt/Crazy Ivan/tree/master/docs/deploy) section.

V2 - Alpha
==========

The V2 Release comes with several major changes from v1:

* Move from ZeroMQ messaging interface to HTTP interface
* Move from Kafka messaging interface to UDP interface
* Support secured HTTP (HTTPS with Basic Auth)
* Support secured UDP (AES-256 symmetric encryption)
* Multi-threaded processing of both Transactions (HTTP) and Events (UDP)
* In-memory cache for event streams

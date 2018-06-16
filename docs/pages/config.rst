.. _configuration:

Startup
=======

Crazy Ivan can be configured from one or more sources:

* Environment Variables
* Command Line Arguments
* Consul KV Store
* Properties File

The application gives priority to the values retrieved in the above order.  This means
that an environment variable setting will override any other setting.

Crazy Ivan has several startup options:

Consul Address - Starts Crazy Ivan against a Consul instance.  Specified by
either the `consul` command line argument or the `AOSSL_CONSUL_ADDRESS`
environment variable.  For example:

`./crazy_ivan consul=http://127.0.0.1:8500`

Properties File - Starts Crazy Ivan against a Properties File.  Specified by either
the `props` command line argument or the `AOSSL_PROPS_FILE` environment variable.  For example:

`./crazy_ivan props=app.properties`

If no properties file is specified, Crazy Ivan will look for one named `app.properties`.

The consul address can also be specified within the properties file, with the key `consul`.

SSL Context Configuration is performed on startup, if enabled, from the file `ssl.properties`.

Configuration
=============

Logging
-------

* Log File

`log.file=ivan.log`

* Log Level (Debug, Info, Warning, Error)

`log.level=Debug`

Transaction
-----------

* Format for transactions

`transaction.format=json`

* Transaction ID's active or inactive

`transaction.id.stamp=True`

Event
-----

* Method for streaming events

`event.stream.method=udp`

* Format for events

`event.format=json`

HTTP
----

* HTTP host to register with Consul

`http.host=127.0.0.1`

* HTTP Port

`http.port=8765`

Cluster
-------

* Name of the cluster this instance is in

`cluster.name=test`

Neo4j
-----

Optional connectivity information when dependent services are
needed and not registered with Consul

* Neo4j Connectivity String

`connection.neo4j=neo4j://localhost:7687`

Security
--------

* true to enable HTTPS socket, false to use HTTP socket

`transaction.security.ssl.enabled=true`

* Authentication type:
  - none: No authentication required
  - single: basic-auth with single user/password set in properties file
  - basic: basic-auth with users stored in Neo4j

`transaction.security.auth.type=single`

* If auth.type is single, these set the only user.  If auth.type is basic, these set the default user.

`transaction.security.auth.user=test`
`transaction.security.auth.password=test`

* Password used for SHA1 algorithm to generate hashes of the stored password

`transaction.security.hash.password=test`

* Key used for AES Encryption

`event.security.out.aes.key=s3cr3tk3y`

* Salt used for AES Encryption

`event.security.out.aes.salt=asdff8723lasdf(**923412`

* Key used for AES Decryption

`event.security.in.aes.key=4n0th3rk4y`

* Salt used for AES Decryption

`event.security.in.aes.salt=asdff8723lasdf(**923412`

:ref:`Go Home <index>`

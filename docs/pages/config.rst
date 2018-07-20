.. _configuration:

Startup
=======

Crazy Ivan can be configured from one or more sources:

* Environment Variables
* Command Line Arguments
* Consul KV Store
* Vault KV Store
* Properties File

The application gives priority to the values retrieved in the above order.  This means
that an environment variable setting will override any other setting.

Crazy Ivan has several startup options:

Cluster Name
------------
The 'cluster' option on the command line or in a properties file, or the 'AOSSL_CLUSTER_NAME' environment variable,
will set the name of the cluster.  A cluster is a grouping of Crazy Ivan instances, which have been assigned particular
scenes to manage.  Each Crazy Ivan instance is designed to manage a set number of scenes, and this allows for highly
optimized streaming of object updates.

The cluster name will affect both how Crazy Ivan registers with Consul, if provided, as well as the names of
cluster-specific security properties.

Vault
-----
Vault Address - Starts Crazy Ivan against a Vault instance.  Specified by
a collection of arguments:
* vault (Environment Variable AOSSL_VAULT_ADDRESS) - the address of the vault instance
* vault.cert (Environment Variable AOSSL_VAULT_SSL_CERT) - the location of the SSL certificate to use when communicating with Vault.
* vault.authtype (Environment Variable AOSSL_VAULT_AUTH_TYPE) - the authentication type used by Vault, currently supported options are 'APPROLE' and 'BASIC'
* vault.un (Environment Variable AOSSL_VAULT_AUTH_UN) - The Username/Role Id for authenticating with Vault
* vault.pw (Environment Variable AOSSL_VAULT_AUTH_PW) - The Password/Secret Id for authenticating with Vault

`./crazy_ivan vault=http://127.0.0.1:8200 vault.authtype=BASIC vault.un=test vault.pw=test`

In addition, the Vault UN and PW can be loaded from files on disk, 'vault_un.txt' and 'vault_pw.txt'.  This is the recommended
method to set authentication info in CI/CD processes within an application container.

Secure Properties
-----------------
Secure Properties can be loaded from a properties file for development purposes, but in a
Production scenario should always be loaded from a Vault instance.  Once Crazy Ivan is connected
to a Vault instance, the following properties can be loaded:

CONSUL_SSL_CERT - The SSL Certificate to use when communicating with Consul
CONSUL_ACL_TOKEN - The ACL Token to use when communicating with Consul
NEO4J_AUTH_UN - The Username to authenticate with discovered Neo4j instances
NEO4J_AUTH_PW - The Password to authenticate with discovered Neo4j instances
{cluster-name}_TRANSACTION_SECURITY_AUTH_USER - The username which will authenticate with Crazy Ivan over HTTP(s)
{cluster-name}_TRANSACTION_SECURITY_AUTH_PASSWORD - The password which will authenticate with Crazy Ivan over HTTP(s)
{cluster-name}_TRANSACTION_SECURITY_HASH_PASSWORD - The password for the hashing algorithm used to hash the password prior to storage.
{cluster-name}_EVENT_SECURITY_OUT_AES_KEY - The key for the AES-256 encryption used for sending UDP messages.
{cluster-name}_EVENT_SECURITY_OUT_AES_SALT - The salt used for the AES-256 encryption used for sending UDP messages.
{cluster-name}_EVENT_SECURITY_IN_AES_KEY - The key for the AES-256 encryption used for receiving UDP messages.
{cluster-name}_EVENT_SECURITY_IN_AES_SALT - The salt used for the AES-256 encryption used for receiving UDP messages.

Consul
------
Consul Address - Starts Crazy Ivan against a Consul instance.  Specified by
either the `consul` command line argument or the `AOSSL_CONSUL_ADDRESS`
environment variable.

`./crazy_ivan consul=http://127.0.0.1:8500`

We may also include the arguments:
* consul.cert (Environment Variable AOSSL_CONSUL_SSL_CERT) - The location of the SSL Certificate to use when communicating with Consul
* consul.token (Environment Variable AOSSL_CONSUL_ACL_TOKEN) - The ACL Token to use when communicating with Consul

This will enable property retrieval from Consul KV Store, registering with Consul on Startup, and the ability to use
Neo4j discovery to dynamically find and update the Neo4j instance processing updates.

Properties File
---------------
Properties File - Starts Crazy Ivan against a Properties File.  Specified by either
the `props` command line argument or the `AOSSL_PROPS_FILE` environment variable.  For example:

`./crazy_ivan props=app.properties`

If no properties file is specified, Crazy Ivan will look for one named `app.properties` in both the
current working folder, and in /etc/ivan/.

The consul address can also be specified within the properties file, with the key `consul`.

HTTPS Setup
-----------
SSL Context Configuration is performed on startup, if enabled.  If the following properties
are set, then SSL Certs for Crazy Ivan can be generated dynamically from Vault:

transaction.security.ssl.ca.vault.active - 'true' or 'false'
transaction.security.ssl.ca.vault.role_name - the name of the role to use to generate the SSL Cert
transaction.security.ssl.ca.vault.common_name - The Common-Name to use on the Certificate

Otherwise, SSL Certificate Generation can be configured from a file in the current working directory called 'ssl.properties'.

Neo4j Connection
----------------
A full connection string may be supplied as a properties value for, but in Production Scenarios
it is recommended to use Neo4j Discovery.  This is controlled by the property neo4j.discover, which
can be 'true' or 'false'.  If it is set to true, then Crazy Ivan will use Consul to find a Neo4j
instance, and will dynamically find new instances when it encounters many consecutive failures.

When enabled, you will want to utilize the secure properties 'NEO4J_AUTH_UN' and 'NEO4J_AUTH_PW' in Vault,
in order to store the authorization info for Neo4j securely.

Other Values
------------

There are a number of other options present in the default app.properties file provided.

Below is an overview of the remaining properties:

* Log File - Path on disk to write logs to

`log.file=ivan.log`

* Log Level - Debug, Info, Warning, Error

`log.level=Debug`

* Format for transactions (HTTP traffic).  Currently only json is supported.

`transaction.format=json`

* Transaction ID's active or inactive.  If active, Crazy Ivan will ensure a Transaction Id is stamped on each message.

`transaction.id.stamp=True`

* Method for streaming events.  Currently only udp is supported.

`event.stream.method=udp`

* Format for streaming events.  Currently only json is supported

`event.format=json`

* HTTP host to register with Consul

`http.host=127.0.0.1`

* HTTP Port

`http.port=8765`

:ref:`Go Home <index>`

.. _security:

Security
========

Crazy Ivan has several forms of security, with one form for transactions (HTTP API),
and another form for events (UDP API).

Transactions
------------

Transactional Security utilizes SSL and Basic Auth over HTTP (HTTPS).  The username/password
can be configured in the application configuration, and SSL will require a valid server
key and certificate.  The locations can then be entered into the ssl.properties file.

The following commands can be used to generate a self-signed SSL cert, along with
a client cert.  This can be used to test the secured transactional setup.

`openssl req -x509 -newkey rsa:4096 -keyout caKey.key -out caCert.pem -days 365`

`openssl genrsa -out clientKey.key 2048`

`openssl req -new -key clientKey.key -out clientCert.csr`

`openssl x509 -req -in clientCert.csr -CA caCert.pem -CAkey caKey.key -CAcreateserial -out MyClient1.crt -days 1024 -sha256`

Events
------

UDP Events utilize AES encryption, with the key and salt set in the application configuration.

Keep in mind that AES encryption is symmetrical, meaning that the encryption keys
must be distributed to the clients in order to encrypt traffic between them
and Crazy Ivan.  The key and salt are delivered to end user devices after a
registration transaction, which is both authenticated and encrypted.

Configuration
-------------

Secure configuration values should stored in Hashicorp Vault, with full encryption
and authentication enabled.  Connecting and authenticating to any service requires
accessing at least one secure property in Vault, ensuring that any malicious entities
must go through Vault to get into any system in the network.

This does mean that your Vault instance should be carefully guarded: it has all
of the keys to the castle.  However, it is a system designed specifically to
guard these secrets, so when used properly it is one of the best safeguards
available, along with a healthy dose of common-sense.

.. _advanced_walkthrough:

Secured Deployment Walkthrough
==============================

Overview
--------

A full deployment of Crazy Ivan involves several steps:

* Consul Setup
* Neo4j Setup
* Vault Setup
* Deploy Crazy Ivan

Here, we'll go through each step and deploy a Crazy Ivan instance which uses
encryption and authentication for all communications, and stores sensitive
configuration values securely in vault.  We will focus on configuration and
startup of the above applications, and it is assumed that you have either
installed all of the above either from their latest official release, or have
running Docker Images of each.  You'll also need openssl installed,
in order to generate SSL certs.

For those using a containerized infrastructure (ie. Docker Containers), there
are a few additional steps you will need to take.

* You may need to bind volumes to each container in order to provide each container
with the correct certificates/keys for SSL/TLS encryption.  In a full production
deployment, the best way to provide these to each container is via orchestration
architecture, such as Kubernetes, Ansible, etc.  For the case of this walkthrough,
however, no such architecture is needed.
* If you are going to network your containers together, you'll need to provide
SSL Certificates with Common Names that match to each container name for Neo4j,
Consul, and Vault.  Otherwise, you may get certificate validation errors.

SSL Setup
---------
We will have to generate SSL Certificates for every service, and in this
walk-through we'll be self-signing them.  This is not a good idea for a production
environment, where you should be getting your certificates signed by a valid CA.

We're going to start by adding an entry to the /etc/hosts file.  This is to ensure
that the hostname we use resolves to only 127.0.0.1, and not ::1.  Add the following
line to the file:

.. code-block::

  127.0.0.1   local
  127.0.0.1   local.local

You will need to enter 'local' as the Common Name during Certificate Generation,
this will prevent certificate errors from occurring for the tutorial.
Keep in mind that you will need to use your actual host and domain names here for
a production deployment.

In order to generate the CA certs we'll use to self-sign the server certificates,
run the following:

.. code-block:: bash

   sudo mkdir /var/ssl
   sudo mkdir /var/ssl/consul
   sudo mkdir /var/ssl/vault
   sudo mkdir /var/ssl/neo4j
   sudo openssl genrsa -des3 -out /var/ssl/ca.key 4096
   sudo openssl req -new -x509 -days 365 -key /var/ssl/ca.key -out /var/ssl/ca.crt

Next, we'll add the CA Certificate to the system trusted certificates, to
prevent certificate errors during the tutorial.  On Redhat/CentOS:

.. code-block:: bash

   sudo cp /var/ssl/ca.crt /etc/pki/ca-trust/source/anchors/
   sudo update-ca-trust

Ubuntu users can follow the steps here: https://askubuntu.com/questions/73287/how-do-i-install-a-root-certificate

Consul Setup
------------

Before we do anything else, we should go ahead and generate the SSL certificate
that Consul will use:

.. code-block:: bash

   sudo openssl genrsa -out /var/ssl/consul/clientKey.key 2048
   sudo openssl req -new -key /var/ssl/consul/clientKey.key -out /var/ssl/consul/clientCert.csr
   sudo openssl x509 -req -in /var/ssl/consul/clientCert.csr -CA /var/ssl/ca.crt -CAkey /var/ssl/ca.key -CAcreateserial -out /var/ssl/consul/MyClient1.crt -days 1024 -sha256

Now, generate an encryption key for Consul gossip:

.. code-block:: bash

   consul keygen

Then, take this value and save it in the file 'consul_config.json':

.. code-block:: json

  {
  	"acl_datacenter": "dc1",
  	"acl_master_token": "as3cr3t",
  	"acl_default_policy": "deny",
  	"acl_down_policy": "extend-cache"
    "encrypt": "your-encryption-key-here",
    "encrypt_verify_incoming": true,
    "encrypt_verify_outgoing": true
  }

Now, we can startup the agent:

.. code-block:: bash

   mkdir consul_data
   consul agent -server -bootstrap -data-dir consul_data/ -bind=127.0.0.1 -config-file consul_config.json -ui``

After this, we'll need to generate an Agent ACL token:

.. code-block:: bash

   curl --request PUT --header "X-Consul-Token: b1gs33cr3t" --data '{"Name": "Agent Token", "Type": "client", "Rules": "{\"key\":{\"\":{\"policy\":\"write\"}},\"node\":{\"\":{\"policy\":\"write\"}},\"service\":{\"\":{\"policy\":\"write\"}},\"agent\":{\"\":{\"policy\":\"write\"}},\"session\":{\"\":{\"policy\":\"write\"}}}"}' http://127.0.0.1:8500/v1/acl/create

This will generate a token, that needs to be added into the Consul config file.
We'll also go ahead and add our HTTPS information to enable encryption:

.. code-block:: json

  {
  	"acl_datacenter": "dc1",
  	"acl_master_token": "b1gs33cr3t",
  	"acl_default_policy": "deny",
  	"acl_down_policy": "extend-cache"
    "acl_agent_token": "agent-token-here"
    "encrypt": "encryption-key-here",
    "encrypt_verify_incoming": true,
    "encrypt_verify_outgoing": true,
    "addresses": {
      "https": "0.0.0.0"
    },
    "ports": {
      "https": 8289
    },
    "key_file": "/var/ssl/consul/clientKey.key",
    "cert_file": "/var/ssl/consul/MyClient1.crt",
    "ca_file": "/var/ssl/ca.crt"
  }


Once the agent is restarted with the new configuration, both encryption and
authentication fully enabled.

Neo4j Setup
-----------

Once again, we'll start by creating SSL Certificates for Neo4j.
Create the directory /var/ssl/neo4j.  Then, run the below commands to
generate a self-signed certificate (in production, you should use a certificate
signed by a valid CA).

.. code-block:: bash

   sudo openssl genrsa -des3 -out /var/ssl/neo4j/serv.key 1024
   sudo openssl req -new -key /var/ssl/neo4j/serv.key -out /var/ssl/neo4j/server.csr``
   sudo openssl x509 -req -days 365 -in /var/ssl/neo4j/server.csr -CA /var/ssl/ca.crt -CAkey /var/ssl/ca.key -set_serial 01 -out /var/ssl/neo4j/server.crt``
   sudo openssl pkcs8 -topk8 -inform PEM -outform PEM -nocrypt -in /var/ssl/neo4j/serv.key -out /var/ssl/neo4j/server.key``

Create the folder /var/ssl/trusted/neo4j, and copy the /var/ssl/neo4j/server.crt file into it.

Then, update the following settings in your Neo4j configuration file:

.. code-block:: properties

  dbms.ssl.policy.default.trusted_dir=/var/ssl/trusted/neo4j
  dbms.ssl.policy.default.public_certificate=/var/ssl/neo4j/server.crt
  dbms.ssl.policy.default.private_key=/var/ssl/neo4j/server.key
  dbms.ssl.policy.default.base_directory=/var/ssl/neo4j/
  dbms.connector.https.enabled=true
  dbms.connector.https.listen_address=:7473
  dbms.connector.bolt.enabled=true
  dbms.connector.bolt.tls_level=REQUIRED

Neo4j uses a pre-set configuration location for SSL certs to be used by the Bolt
connector.  In order to install our self-signed certs for use with Bolt, we need
to copy them into the folder Neo4j is expecting, with the correct names.

.. code-block:: bash

   sudo cp /var/ssl/neo4j/server.crt /var/lib/neo4j/certificates/neo4j.cert
   sudo cp /var/ssl/neo4j/server.key /var/lib/neo4j/certificates/neo4j.key

Now, restart the Neo4j server.  Once the server is started,
it will need to be registered for discovery with Consul.
This can be done with curl, for example:

.. code-block:: bash

   curl -X PUT --header "X-Consul-Token: b1gs33cr3t" -d '{"ID": "neo4j", "Name": "neo4j", "Tags": ["Primary"], "Address": "local", "Port": 7687}' http://127.0.0.1:8500/v1/agent/service/register

In addition, the username/password for the instance is normally set on startup in the UI.
Be sure to take note of this, as we'll need it to configure Crazy Ivan.

Vault Setup
-----------

Now, let's generate our SSL Certificate for Vault:

.. code-block:: bash

   sudo openssl genrsa -out /var/ssl/vault/clientKey.key 2048
   sudo openssl req -new -key /var/ssl/vault/clientKey.key -out /var/ssl/vault/clientCert.csr``
   sudo openssl x509 -req -in /var/ssl/vault/clientCert.csr -CA /var/ssl/ca.crt -CAkey /var/ssl/ca.key -CAcreateserial -out /var/ssl/vault/MyClient1.crt -days 1024 -sha256``

We'll be configuring Vault to use the Consul Storage backend, which means we are
going to need an ACL token for Vault to use:

.. code-block:: bash

   curl --request PUT --header "X-Consul-Token: b1gs33cr3t" --data '{"Name": "Agent Token", "Type": "client", "Rules": "{\"key\":{\"vault/\":{\"policy\":\"write\"}},\"node\":{\"\":{\"policy\":\"write\"}},\"service\":{\"vault\":{\"policy\":\"write\"}},\"agent\":{\"\":{\"policy\":\"write\"}},\"session\":{\"\":{\"policy\":\"write\"}}}"}' http://127.0.0.1:8500/v1/acl/create

Copy the resulting token, then save the below as a file 'vault_config.hcl':

.. code-block::

  storage "consul" {
  address = "127.0.0.1:8500"
  scheme = "https"
  path    = "vault/"
  token   = "your-acl-token-here"
  tls_skip_verify = 0
  tls_cert_file = "/var/ssl/vault/MyClient1.crt"
  tls_key_file = "/var/ssl/vault/clientKey.key"
  tls_ca_file = "/var/ssl/ca.crt"
  }

  listener "tcp" {
  address     = "127.0.0.1:8200"
  tls_disable = 0
  tls_cert_file = "/var/ssl/vault/MyClient1.crt"
  tls_key_file = "/var/ssl/vault/clientKey.key"
  }

  disable_mlock=true

Before starting the Vault server, you may need to add the CA certificate you
generated to your system chain.  On CentOS/Redhat, this can be done by copying
the CA certificate into the /etc/pki/ca-trust/source/anchors directory, and
then refreshing the certificate chain:

.. code-block:: bash

   sudo cp /var/ssl/ca.crt /etc/pki/ca-trust/source/anchors
   sudo update-ca-trust

You may need to reference the documentation for your particular OS otherwise.

Now, we can start the Vault server:

.. code-block:: bash

   vault server -config=vault_config.hcl

In a separate terminal, we'll need to configure the Vault.

.. code-block:: bash

   export VAULT_ADDR='https://local:8200'
   vault operator init``

Save the unseal keys and root key output when we initialize the vault above.

Next, we will unseal the Vault.  We'll need to run this operation 3 times,
with 3 unique unseal keys.

.. code-block:: bash

   vault operator unseal

Before we continue configuring the Vault, we need to login.  Be sure to enter
the root key you saw during Vault Initialization.

.. code-block:: bash

   vault login root-key-here

Our next step is enabling authentication in Vault.  Save the following
to a file 'vault_admin_policy.hcl':

.. code-block::

  path "secret/*" {
  capabilities = ["create", "read", "update", "delete", "list"]
  }

  path "consul/*" {
  capabilities = ["read", "list"]
  }

  path "pki/*" {
  capabilities = ["create", "read", "update", "list"]
  }

Now we can enable userpass authentication, and create a user and policy.

.. code-block:: bash

   vault auth enable userpass
   vault write auth/userpass/users/test password=test policies=admins
   vault policy write admins vault_admin_policy.hcl

Now, we can enable our other secrets engines:

.. code-block:: bash

   vault secrets enable -version=2 kv
   vault secrets enable pki
   vault secrets enable consul
   vault secrets tune -max-lease-ttl=8760h pki

We'll need to setup Vault to use a management token from Consul:

.. code-block:: bash

   curl --header "X-Consul-Token: b1gs33cr3t" --request PUT --data '{"Name": "sample", "Type": "management"}' http://127.0.0.1:8500/v1/acl/create

Copy the resulting token, and pass it to Vault to use:

.. code-block:: bash

   vault write consul/config/access address=127.0.0.1:8500 token=your-token-here

To complete the Consul Secrets Engine configuration, we can add a role which Crazy Ivan
can use to generate consul ACL tokens.

.. code-block:: bash

   vault write consul/roles/new-role policy=$(base64 <<< 'key "" {policy="read"} service "" {policy="write"}')

Next, let's finish the PKI Secrets Engine configuration, which will allow Crazy Ivan
to generate SSL Certificates from Vault on startup.

First, we have Vault generate an internal CA certificate (Note that this is not advised
in Production scenarios), and signing information:

.. code-block:: bash

   vault write pki/root/generate/internal common_name=my-website.com ttl=8760h
   vault write pki/config/urls issuing_certificates="http://127.0.0.1:8200/v1/pki/ca" crl_distribution_points="http://127.0.0.1:8200/v1/pki/crl"``

Finally, we'll set up another role that allows for generation of SSL Certificates

.. code-block:: bash

   vault write pki/roles/pki-role allowed_domains=local allow_subdomains=true max_ttl=72h

Crazy Ivan Setup
----------------

Before starting Crazy Ivan, we'll want to populate some configuration values.

Non-secure configuration options can be set in Consul.  Most of the defaults will
work for us here, so we'll just go ahead and enable authentication in Crazy Ivan HTTPS requests:

.. code-block:: bash

   curl --header "X-Consul-Token: b1gs33cr3t" --request PUT --data 'single' https://local:8500/v1/kv/ivan/prod/IVAN_PROD_TRANSACTION_SECURITY_AUTH_TYPE

Secure configuration options can be set in Vault.  Let's setup our core
encryption information in Vault.  First, we enter Event (UDP) encryption settings:

.. code-block:: bash

   vault kv put secret/IVAN_PROD_TEST_EVENT_SECURITY_IN_AES_SALT IVAN_PROD_TEST_EVENT_SECURITY_IN_AES_SALT=test
   vault kv put secret/IVAN_PROD_TEST_EVENT_SECURITY_IN_AES_KEY IVAN_PROD_TEST_EVENT_SECURITY_IN_AES_KEY=test
   vault kv put secret/IVAN_PROD_TEST_EVENT_SECURITY_OUT_AES_SALT IVAN_PROD_TEST_EVENT_SECURITY_OUT_AES_SALT=test
   vault kv put secret/IVAN_PROD_TEST_EVENT_SECURITY_OUT_AES_KEY IVAN_PROD_TEST_EVENT_SECURITY_OUT_AES_KEY=test

Next, we setup our authentication information for Neo4j:

.. code-block:: bash

   vault kv put secret/IVAN_PROD_NEO4J_AUTH_UN IVAN_PROD_NEO4J_AUTH_UN=neo4j
   vault kv put secret/IVAN_PROD_NEO4J_AUTH_PW IVAN_PROD_NEO4J_AUTH_PW=neo4j

Finally, we provide the authentication options for Transactions (HTTP(s)):

.. code-block:: bash

   vault kv put secret/IVAN_PROD_TRANSACTION_SECURITY_AUTH_USER IVAN_PROD_TRANSACTION_SECURITY_AUTH_USER=test
   vault kv put secret/IVAN_PROD_TRANSACTION_SECURITY_AUTH_PASSWORD IVAN_PROD_TRANSACTION_SECURITY_AUTH_PASSWORD=test
   vault kv put secret/IVAN_PROD_TRANSACTION_SECURITY_HASH_PASSWORD IVAN_PROD_TRANSACTION_SECURITY_HASH_PASSWORD=test

Full details on configuration options can be found in the :ref:`Configuration <config>`
section of the documentation.  Finally, you can start Crazy Ivan with:

.. code-block:: bash

   ./crazy_ivan ivan.prod.vault=https://local:8200 ivan.prod.vault.cert= ivan.prod.vault.authtype=BASIC ivan.prod.vault.un=test ivan.prod.vault.pw=test ivan.prod.consul.token.role=new-role ivan.prod.consul=https://local:8289 ivan.prod.consul.cert= ivan.prod.cluster=test ivan.prod.neo4j.discover=true ivan.prod.neo4j.ssl.ca.file=/var/ssl/ca.crt ivan.prod.transaction.security.ssl.ca.vault.active=true ivan.prod.transaction.security.ssl.ca.vault.role_name=pki-role ivan.prod.transaction.security.ssl.ca.vault.common_name=local.local

Several files will be created on startup, with the extensions '.key' and '.pem'.
These are all of the certificates and keys that Crazy Ivan is using to encrypt the
HTTPS connection.

Make sure your server is up using the health check endpoint:

.. code-block:: bash

   curl --user test:test https://local.local/health

:ref:`Go Home <index>`

.. _advanced_walkthrough:

Deployment
==========

A full deployment of Crazy Ivan involves several steps:

* Consul Setup
* Neo4j Setup
* Vault Setup
* Deploy Crazy Ivan

Here, we'll go through each step and deploy a Crazy Ivan instance which uses
encryption and authentication for all communications, and stores sensitive
configuration values securely in vault.  We will assume that the above programs
are installed on the computer in question, so we will focus on configuration
and startup.  Please refer to the :ref:`Getting Started guide <quickstart>` for
more information on installation.

You'll also need openssl installed, in order to generate SSL certs.

Consul Setup
------------

We're going to start by setting up Consul.  Save this as 'consul_config.json':

::
  {
  	"acl_datacenter": "dc1",
  	"acl_master_token": "b1gs33cr3t",
  	"acl_default_policy": "deny",
  	"acl_down_policy": "extend-cache"
  }

Now, we can startup the agent:

``mkdir consul_data``

``consul agent -server -bootstrap -data-dir consul_data/ -bind=127.0.0.1 -config-file consul_config.json -ui``

After this, we'll need to generate an Agent ACL token:

``curl --request PUT --header "X-Consul-Token: b1gs33cr3t" --data '{"Name": "Agent Token", "Type": "client", "Rules": "{\"key\":{\"\":{\"policy\":\"write\"}},\"node\":{\"\":{\"policy\":\"write\"}},\"service\":{\"\":{\"policy\":\"write\"}},\"agent\":{\"\":{\"policy\":\"write\"}},\"session\":{\"\":{\"policy\":\"write\"}}}"}' http://127.0.0.1:8500/v1/acl/create``

This will generate a token, that needs to be copied into the Consul config file.

::
  {
  	"acl_datacenter": "dc1",
  	"acl_master_token": "b1gs33cr3t",
  	"acl_default_policy": "deny",
  	"acl_down_policy": "extend-cache",
  	"acl_agent_token": "agent-token-here"
  }

Once the agent is restarted with the new configuration, ACL will be fully enabled.

Neo4j Setup
-----------

Neo4j requires SSL certificates in order to communicate securely.


Once the server is started, it will need to be registered for discovery with
Consul.  This can be done with curl, for example:

`curl -X PUT -d '{"ID": "neo4j", "Name": "neo4j", "Tags": ["Primary"], "Address": "localhost", "Port": 7687}' http://127.0.0.1:8500/v1/agent/service/register`

In addition, the username/password for the instance is normally set on startup in the UI.
Be sure to take note of this, as we'll need it to configure Crazy Ivan.

Vault Setup
-----------

We'll be configuring Vault to use the Consul Storage backend, which means we are
going to need an ACL token for Vault to use:

``curl --request PUT --header "X-Consul-Token: b1gs33cr3t" --data '{"Name": "Agent Token", "Type": "client", "Rules": "{\"key\":{\"vault/\":{\"policy\":\"write\"}},\"node\":{\"\":{\"policy\":\"write\"}},\"service\":{\"vault\":{\"policy\":\"write\"}},\"agent\":{\"\":{\"policy\":\"write\"}},\"session\":{\"\":{\"policy\":\"write\"}}}"}' http://127.0.0.1:8500/v1/acl/create``

Copy the resulting token, then save the below as a file 'vault_config.hcl':

::
  storage "consul" {
  address = "127.0.0.1:8500"
  path    = "vault/"
  token   = "your-acl-token-here"
  }

  listener "tcp" {
  address     = "127.0.0.1:8200"
  tls_disable = 1
  }

  disable_mlock=true

Now, we can start the Vault server:

``vault server -config=vault_config.hcl``

In a separate terminal, we'll need to configure the Vault.

``export VAULT_ADDR='http://127.0.0.1:8200'``

``vault operator init``

Save the unseal keys and root key output when we initialize the vault above.

Next, we will unseal the Vault.  We'll need to run this operation 3 times,
with 3 unique unseal keys.

``vault operator unseal``

Before we continue configuring the Vault, we need to login.  Be sure to enter
the root key you saw during Vault Initialization.

``vault login root-key-here``

Our next step is enabling authentication in Vault.  Save the following
to a file 'vault_admin_policy.hcl':

::
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

``vault auth enable userpass``

``vault write auth/userpass/users/test password=test policies=admins``

``vault policy write admins vault_admin_policy.hcl``

Now, we can enable our other secrets engines:

``vault secrets enable -version=2 kv``

``vault secrets enable pki``

``vault secrets enable consul``

``vault secrets tune -max-lease-ttl=8760h pki``

We'll need to setup Vault to use a management token from Consul:

``curl --header "X-Consul-Token: b1gs33cr3t" --request PUT --data '{"Name": "sample", "Type": "management"}' http://127.0.0.1:8500/v1/acl/create``

Copy the resulting token, and pass it to Vault to use:

``vault write consul/config/access address=127.0.0.1:8500 token=your-token-here``

To complete the Consul Secrets Engine configuration, we can add a role which Crazy Ivan
can use to generate consul ACL tokens.

``vault write consul/roles/new-role policy=$(base64 <<< 'key "" {policy="read"} service "" {policy="write"}')``

Crazy Ivan Setup
----------------

Before starting Crazy Ivan, we'll want to populate some configuration values.

Non-secure configuration options can be set in Consul.  For example:

``curl --header "X-Consul-Token: b1gs33cr3t" --request PUT --data 'single' http://localhost:8500/v1/kv/ivan/prod/IVAN_PROD_TRANSACTION_SECURITY_AUTH_TYPE``

Secure configuration options can be set in Vault.  For example:

``vault kv put secret/IVAN_PROD_TEST_EVENT_SECURITY_IN_AES_SALT IVAN_PROD_TEST_EVENT_SECURITY_IN_AES_SALT=test``

Full details on configuration options can be found in the :ref:`Configuration <config>`
section of the documentation.  Once you have set the desired configuration options
(in particular, be sure to set the secure configuration options IVAN_PROD_NEO4J_AUTH_UN
and IVAN_PROD_NEO4J_AUTH_PW), you can start Crazy Ivan with:

``./crazy_ivan ivan.prod.vault=http://127.0.0.1:8200 ivan.prod.vault.authtype=BASIC ivan.prod.vault.un=test ivan.prod.vault.pw=test ivan.prod.consul.token.role=new-role ivan.prod.consul=http://127.0.0.1:8500 ivan.prod.cluster=test ivan.prod.neo4j.discover=true``


:ref:`Go Home <index>`

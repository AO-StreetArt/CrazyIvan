.. _deployment:

Deployment
==========

This page includes an overview and notes on full production deployment of
Crazy Ivan.  A step-by-step walkthrough for setting up a secured, single-node
deployment is also available in the :ref:`Advanced Walkthrough <advanced_walkthrough>`.

A full deployment of Crazy Ivan involves several steps:

* Consul Setup
* Neo4j Setup
* Vault Setup
* Crazy Ivan Setup

Each component has it's own encryption and authentication layers.

Crazy Ivan Setup
----------------

Crazy Ivan instances are deployed in clusters, with each cluster managing a
particular set of scenes.  The clusters which contain scenes that may interact
should all be connected to the same Neo4j cluster.  In other words, User Devices
cannot move between Crazy Ivan clusters that run against different Neo4j clusters.

Crazy Ivan can load configuration values from Consul and/or Vault, and uses SSL
encryption with HTTP Basic Authentication for transactions.  Events (sent via UDP)
utilize AES symmetric encryption.

Many configuration values are cluster-specific.  This allows us to set, for example,
separate encryption keys by cluster.

Consul Setup
------------

Deploying a Consul Cluster is covered in detail `on the Consul webpage <https://www.consul.io/>`__.

Crazy Ivan uses the Consul KV Store for unsecured configuration values, as well as
using Consul for Service Discovery.  It can utilize SSL encryption, as well as the ACL layer.

Neo4j Setup
-----------

Deploying a Neo4j Cluster is covered in detail `here <https://neo4j.com/docs/operations-manual/current/>`__.
Note that only HA Clusters are currently supported, utilizing Causal Clustering will provide little to no benefit.

Neo4j in containers is also supported.  Either way, once Neo4j servers are active, they need to
be registered with Consul in order to be picked up by Service Discovery.  This can be done with curl, for example:

`curl -X PUT -d '{"ID": "neo4j", "Name": "neo4j", "Tags": ["Primary"], "Address": "localhost", "Port": 7687}' http://127.0.0.1:8500/v1/agent/service/register`

Vault Setup
-----------

Deploying a Vault Cluster is covered in detail `on the Vault webpage <https://www.vaultproject.io/>`__.

Crazy Ivan can utilize the following Secret Stores:

* Consul - Generate Consul ACL tokens
* PKI - Generate SSL Certs/Keys
* KV - Store secure configuration options

:ref:`Advanced Walkthrough <advanced_walkthrough>`

:ref:`Go Home <index>`

.. tests:

Automated Testing
=================

Crazy Ivan uses `Travis CI <https://travis-ci.org/AO-StreetArt/CrazyIvan>`__ for automated testing.

Within the Travis CI Configuration, several steps are executed to complete full functional testing:

* Set up `Docker <https://www.docker.com/>`__ instances of `Neo4j <https://neo4j.com/>`__, `Redis <https://redis.io/>`__, and `Consul <https://www.consul.io/>`__, and then populate the KV Store in `Consul <https://www.consul.io/>`__ with several configuration values.
* Build a new `Docker <https://www.docker.com/>`__ Image for Crazy Ivan and start it.
* Download `0-Meter <https://github.com/AO-StreetArt/0-Meter>`__.  This is a custom tool developed for 0MQ load testing, and is used to send a series of messages to Crazy Ivan over the course of the tests.  The configuration for 0-Meter CI Tests can be found in the ci/ folder.
* Run `0-Meter <https://github.com/AO-StreetArt/0-Meter>`__ to send a series of messages, some expected to fail and others to succeed, to Crazy Ivan.  Validate the err_code field in the response.
* If all tests pass, then push the newly built image to `Docker Hub <https://hub.docker.com/r/aostreetart/crazyivan/>`__.

Note that unit tests are performed within the Dockerfile itself, so that the Docker build will fail if any unit tests fail.  If you are adding unit tests to Crazy Ivan, you should add them within the Dockerfile as well.

:ref:`Go Home <index>`

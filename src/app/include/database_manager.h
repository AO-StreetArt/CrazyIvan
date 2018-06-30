/*
Apache2 License Notice
Copyright 2017 Alex Barry

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// This implements the Configuration Manager

// This takes in a Command Line Interpreter, and based on the options provided,
// decides how the application needs to be configured.  It may configure either
// from a configuration file, or from a Consul agent

#include <iostream>
#include <boost/cstdint.hpp>

#include "model/include/scene_factory.h"
#include "model/include/scene_interface.h"

#include "api/include/scene_list_factory.h"
#include "api/include/scene_list_interface.h"

#include "aossl/profile/include/network_app_profile.h"
#include "aossl/consul/include/consul_interface.h"

#include "neocpp/connection/interface/neo4j_interface.h"
#include "neocpp/connection/impl/libneo4j_factory.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "app/include/ivan_utils.h"

#ifndef SRC_APPLICATION_INCLUDE_DATABASE_MANAGER_H_
#define SRC_APPLICATION_INCLUDE_DATABASE_MANAGER_H_

//! Encapsulates the Neo4j Interface and allows
//! for safely updating the Neo4j DB that's connected
class DatabaseManager: public Neocpp::Neo4jInterface {
  Neocpp::Neo4jInterface *internal_connection = nullptr;
  AOSSL::NetworkApplicationProfile *internal_profile = nullptr;
  Poco::Logger& logger;
  std::string last_connection_string;
  bool secured = false;
  AOSSL::ServiceInterface *connected_service = nullptr;
  std::atomic<int> failures{0};
  Neocpp::LibNeo4jFactory neo_factory;
  std::string service_name = "neo4j";
  // We use a RW lock to let any number of queries execute
  // simultaneously, XOR let a connection get updated
  Poco::RWLock conn_usage_lock;
  int max_failures = 5;
  int max_retries = 11;
  inline void set_new_connection() {
    Poco::ScopedWriteRWLock scoped_lock(conn_usage_lock);
    if (failures.load() > max_failures) {
      // deregister the existing service
      if (connected_service) {
        logger.debug("De-registering Neo4j instance which is unresponsive");
        internal_profile->get_consul()->deregister_service(*connected_service);
      }
      // Attempt to find a new Neo4j instance to use
      if (connected_service) delete connected_service;
      connected_service = internal_profile->get_service(service_name);
      AOSSL::StringBuffer neo4j_un_buf;
      AOSSL::StringBuffer neo4j_pw_buf;
      internal_profile->get_opt(std::string("neo4j.auth.un"), neo4j_un_buf);
      internal_profile->get_opt(std::string("neo4j.auth.pw"), neo4j_pw_buf);
      std::string neo4j_conn_str = std::string("neo4j://") + neo4j_un_buf.val\
          + std::string(":") + neo4j_pw_buf.val + std::string("@")\
          + connected_service->get_address() + std::string(":")\
          + connected_service->get_port();
      // Reset the internal connection
      if (internal_connection) delete internal_connection;
      internal_connection = \
          neo_factory.get_neo4j_interface(neo4j_conn_str, true, 50, 0, 1);
      failures = 0;
    }
  }
 public:
  DatabaseManager(AOSSL::NetworkApplicationProfile *profile) : \
      logger(Poco::Logger::get("DatabaseManager")) \
      {internal_profile = profile;}
  ~DatabaseManager() \
      {if (internal_connection) delete internal_connection;\
      if (connected_service) delete connected_service;}

  //! Execute the given Cypher Query
  inline Neocpp::ResultsIteratorInterface* execute(const char * query) {
    int retries = 0;
    while (retries < max_retries) {
      if (!internal_connection) set_new_connection();
      bool failure = false;
      try {
        Poco::ScopedReadRWLock scoped_lock(conn_usage_lock);
        return internal_connection->execute(query);
      } catch (std::exception& e) {
        logger.error("Exception executing Neo4j Query");
        logger.error(e.what());
        failures++;
        failure = true;
      }
      if (failure) set_new_connection();
      retries++;
    }
    return nullptr;
  }

  //! Execute the given Cypher Query
  inline Neocpp::ResultsIteratorInterface* execute(std::string query) {
    return execute(query.c_str());
  }

  //! Execute a given Cypher Query with an input map of parameters
  inline Neocpp::ResultsIteratorInterface* execute(const char * query, \
      std::unordered_map<std::string, Neocpp::Neo4jQueryParameterInterface*>\
      query_params) {
    int retries = 0;
    while (retries < max_retries) {
      if (!internal_connection) set_new_connection();
      bool failure = false;
      try {
        Poco::ScopedReadRWLock scoped_lock(conn_usage_lock);
        return internal_connection->execute(query, query_params);
      } catch (std::exception& e) {
        logger.error("Exception executing Neo4j Query");
        logger.error(e.what());
        failures++;
        failure = true;
      }
      if (failure) set_new_connection();
      retries++;
    }
    return nullptr;
  }

  //! Execute a given Cypher Query with an input map of parameters
  inline Neocpp::ResultsIteratorInterface* execute(std::string query, \
      std::unordered_map<std::string, Neocpp::Neo4jQueryParameterInterface*> \
      query_params) {
    return execute(query.c_str(), query_params);
  }
};

#endif  // SRC_APPLICATION_INCLUDE_DATABASE_MANAGER_H_

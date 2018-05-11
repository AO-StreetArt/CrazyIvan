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

#include "query_helper_interface.h"
#include "algorithm_query_helper.h"

#include "neocpp/connection/interface/neo4j_interface.h"
#include "neocpp/connection/impl/libneo4j_factory.h"

#include "aossl/core/include/kv_store_interface.h"

#ifndef SRC_PROC_QUERY_INCLUDE_QUERY_HELPER_FACTORY_H_
#define SRC_PROC_QUERY_INCLUDE_QUERY_HELPER_FACTORY_H_

// ProcessorFactory allows for creation of ProcessorInterface instances
class QueryHelperFactory {
 public:
  QueryHelperFactory() {}
  ~QueryHelperFactory() {}
  QueryHelperInterface* build_query_helper(Neocpp::Neo4jInterface *neo4j, \
    Neocpp::LibNeo4jFactory *nf, AOSSL::KeyValueStoreInterface *con) {
    // Build and return a new message processor
    return new AlgorithmQueryHelper(neo4j, nf, con);
  }
};

#endif  // SRC_PROC_QUERY_INCLUDE_QUERY_HELPER_FACTORY_H_

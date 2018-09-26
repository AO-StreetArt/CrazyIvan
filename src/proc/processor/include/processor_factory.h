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

#include "neocpp/connection/interface/neo4j_interface.h"
#include "neocpp/connection/impl/libneo4j_factory.h"

#include "aossl/core/include/kv_store_interface.h"
#include "aossl/uuid/include/uuid_interface.h"

#include "aossl/profile/include/network_app_profile.h"
#include "aossl/consul/include/consul_interface.h"
#include "proc/processor/include/processor_interface.h"
#include "proc/processor/include/message_processor.h"

#ifndef SRC_PROC_PROCESSOR_INCLUDE_PROCESSOR_FACTORY_H_
#define SRC_PROC_PROCESSOR_INCLUDE_PROCESSOR_FACTORY_H_

// ProcessorFactory allows for creation of ProcessorInterface instances
class ProcessorFactory {
 public:
  ProcessorFactory() {}
  ~ProcessorFactory() {}
  ProcessorInterface* build_processor(Neocpp::LibNeo4jFactory *nf, \
    Neocpp::Neo4jInterface *neo4j, AOSSL::NetworkApplicationProfile *con, \
    AOSSL::UuidInterface *u, std::string cluster) {
    // Build and return a new message processor
    return new MessageProcessor(nf, neo4j, con, u, cluster);
  }
};

#endif  // SRC_PROC_PROCESSOR_INCLUDE_PROCESSOR_FACTORY_H_

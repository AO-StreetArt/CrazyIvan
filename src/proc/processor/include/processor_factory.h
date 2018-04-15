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

#include "include/processor_interface.h"
#include "include/message_processor.h"

#ifndef SRC_PROC_PROCESSOR_INCLUDE_PROCESSOR_FACTORY_H_
#define SRC_PROC_PROCESSOR_INCLUDE_PROCESSOR_FACTORY_H_

// ProcessorFactory allows for creation of ProcessorInterface instances
class ProcessorFactory {
 public:
  ProcessorFactory() {}
  ~ProcessorFactory() {}
  ProcessorInterface* build_processor(Neo4jComponentFactory *nf, \
    Neo4jInterface *neo4j, \
    ConfigurationManager *con, uuidInterface *u) {
    // Build and return a new message processor
    return new MessageProcessor(nf, neo4j, con, u);
  }
};

#endif  // SRC_PROC_PROCESSOR_INCLUDE_PROCESSOR_FACTORY_H_

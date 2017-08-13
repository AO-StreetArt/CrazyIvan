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

// tests for the configuration manager
// src/test/test.properties

#include <assert.h>
#include <string>

#include "aossl/logging/include/logging_interface.h"
#include "aossl/logging/include/factory_logging.h"

#include "ivan_log.h"

int main(int argc, char** argv) {
  LoggingComponentFactory *logging_factory = new LoggingComponentFactory;

  // -----------------------------Logging------------------------------------ //
  // ------------------------------------------------------------------------ //

  std::string initFileName = "test/log4cpp.properties";
  logging = logging_factory->get_logging_interface(initFileName);

  start_logging_submodules();

  logging->debug("Debug Test");
  logging->info("Info Test");
  logging->error("Error Test");

  shutdown_logging_submodules();

  delete logging;
  delete logging_factory;
  return 0;
}

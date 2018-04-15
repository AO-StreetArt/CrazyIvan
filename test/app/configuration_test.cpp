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
#include <vector>

#include "aossl/commandline/include/commandline_interface.h"
#include "aossl/commandline/include/factory_cli.h"

#include "aossl/logging/include/logging_interface.h"
#include "aossl/logging/include/factory_logging.h"

#include "aossl/uuid/include/uuid_interface.h"
#include "aossl/uuid/include/factory_uuid.h"

#include "ivan_log.h"
#include "configuration_manager.h"

int main(int argc, char** argv) {
  LoggingComponentFactory *logging_factory = new LoggingComponentFactory;
  CommandLineInterpreterFactory *cli_factory = \
    new CommandLineInterpreterFactory;
  uuidComponentFactory *id_factory = new uuidComponentFactory;

  // -----------------------------Logging------------------------------------ //
  // ------------------------------------------------------------------------ //

  std::string initFileName = "test/log4cpp.properties";
  logging = logging_factory->get_logging_interface(initFileName);

  start_logging_submodules();

  logging->debug("PreTest Setup");

  // Set up the UUID Generator
  uuidInterface *ua = id_factory->get_uuid_interface();

  // Set up our command line interpreter
  CommandLineInterface *cli = \
    cli_factory->get_command_line_interface(argc, argv);

  UuidContainer id_container = ua->generate();
  if (!id_container.err.empty()) {
    uuid_logging->error(id_container.err);
  }

  ConfigurationManager cm(cli, id_container.id);

  logging->debug("Configure the app");

  cm.configure();

  logging->debug("Checking Variable Retrieval");

  // Basic Tests

  assert(cm.get_dbconnstr() == "couchbase://localhost/default");
  assert(cm.get_ibconnstr() == "tcp://*:5555");
  logging->debug("Values checked");

  shutdown_logging_submodules();

  delete cli;
  delete ua;
  delete logging;
  delete id_factory;
  delete cli_factory;
  delete logging_factory;
  return 0;
}

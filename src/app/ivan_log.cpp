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

#include "include/ivan_log.h"

LoggingCategoryInterface *config_logging = NULL;
LoggingCategoryInterface *obj_logging = NULL;
LoggingCategoryInterface *main_logging = NULL;
LoggingCategoryInterface *processor_logging = NULL;
LoggingCategoryInterface *uuid_logging = NULL;

void start_logging_submodules() {
  if (!uuid_logging) uuid_logging = logging->get_category("uuid");
  if (!config_logging) config_logging = logging->get_category("configuration");
  if (!obj_logging) obj_logging = logging->get_category("obj");
  if (!main_logging) main_logging = logging->get_category("main");
  if (!processor_logging) {
    processor_logging = logging->get_category("processor");
  }
}

void shutdown_logging_submodules() {
  if (!uuid_logging) {
    logging->debug("UUID Logging Module delete called without object");
  } else {
    logging->debug("UUID Logging Module delete called");
    delete uuid_logging;
    uuid_logging = NULL;
  }
  if (!config_logging) {
    logging->debug("Configuration Logging Module delete called without object");
  } else {
    logging->debug("Configuration Logging Module delete called");
    delete config_logging;
    config_logging = NULL;
  }
  if (!obj_logging) {
    logging->debug("Object3 Logging Module delete called without object");
  } else {
    logging->debug("Object3 Logging Module delete called");
    delete obj_logging;
    obj_logging = NULL;
  }
  if (!main_logging) {
    logging->debug("Main Logging Module delete called without object");
  } else {
    logging->debug("Main Logging Module delete called");
    delete main_logging;
    main_logging = NULL;
  }
  if (!processor_logging) {
    logging->debug("Processor Logging Module delete called without object");
  } else {
    logging->debug("Processor Logging Module delete called");
    delete processor_logging;
    processor_logging = NULL;
  }
}

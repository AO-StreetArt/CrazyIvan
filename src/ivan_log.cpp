#include "include/ivan_log.h"

LoggingCategoryInterface *config_logging = NULL;
LoggingCategoryInterface *obj_logging = NULL;
LoggingCategoryInterface *redis_logging = NULL;
LoggingCategoryInterface *main_logging = NULL;
LoggingCategoryInterface *processor_logging = NULL;
LoggingCategoryInterface *uuid_logging = NULL;

void start_logging_submodules()
{
  if (!uuid_logging) {uuid_logging = logging->get_category("uuid");}
  if (!config_logging) {config_logging = logging->get_category("configuration");}
  if (!obj_logging) {obj_logging = logging->get_category("obj");}
  if (!redis_logging) {redis_logging = logging->get_category("redis");}
  if (!main_logging) {main_logging = logging->get_category("main");}
  if (!processor_logging) {processor_logging = logging->get_category("processor");}
}

void shutdown_logging_submodules()
{
  if (!uuid_logging)
  {
    logging->debug("UUID Logging Module delete called without initialized object");
  }
  else
  {
    logging->debug("UUID Logging Module delete called");
    delete uuid_logging;
  }
  if (!config_logging)
  {
    logging->debug("Configuration Logging Module delete called without initialized object");
  }
  else
  {
    logging->debug("Configuration Logging Module delete called");
    delete config_logging;
  }
  if (!obj_logging)
  {
    logging->debug("Object3 Logging Module delete called without initialized object");
  }
  else
  {
    logging->debug("Object3 Logging Module delete called");
    delete obj_logging;
  }
  if (!redis_logging)
  {
    logging->debug("Document Manager Logging Module delete called without initialized object");
  }
  else
  {
    logging->debug("Document Manager Logging Module delete called");
    delete redis_logging;
  }
  if (!main_logging)
  {
    logging->debug("Main Logging Module delete called without initialized object");
  }
  else
  {
    logging->debug("Main Logging Module delete called");
    delete main_logging;
  }
  if (!processor_logging)
  {
    logging->debug("Processor Logging Module delete called without initialized object");
  }
  else
  {
    logging->debug("Processor Logging Module delete called");
    delete processor_logging;
  }
}

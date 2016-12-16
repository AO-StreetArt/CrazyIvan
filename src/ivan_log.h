#ifndef IVAN_LOGGING
#define IVAN_LOGGING

#include "aossl/factory/logging_interface.h"

extern LoggingCategoryInterface *uuid_logging;
extern LoggingCategoryInterface *config_logging;
extern LoggingCategoryInterface *obj_logging;
extern LoggingCategoryInterface *redis_logging;
extern LoggingCategoryInterface *main_logging;
extern LoggingCategoryInterface *processor_logging;

void start_logging_submodules();
void shutdown_logging_submodules();

#endif

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

#ifndef SRC_APP_INCLUDE_IVAN_LOG_H_
#define SRC_APP_INCLUDE_IVAN_LOG_H_

#include "aossl/logging/include/logging_interface.h"

extern LoggingCategoryInterface *uuid_logging;
extern LoggingCategoryInterface *config_logging;
extern LoggingCategoryInterface *obj_logging;
extern LoggingCategoryInterface *main_logging;
extern LoggingCategoryInterface *processor_logging;

void start_logging_submodules();
void shutdown_logging_submodules();

#endif  // SRC_APP_INCLUDE_IVAN_LOG_H_

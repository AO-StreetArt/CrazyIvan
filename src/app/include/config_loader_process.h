#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "aossl/profile/include/tiered_app_profile.h"

#ifndef SRC_APP_INCLUDE_CONFIG_LOADER_PROCESS_H_
#define SRC_APP_INCLUDE_CONFIG_LOADER_PROCESS_H_

void update_config(AOSSL::TieredApplicationProfile *config, long interval) {
  while (true) {
    config->load_config();
    usleep(interval);
  }
}

#endif  // SRC_APP_INCLUDE_CONFIG_LOADER_PROCESS_H_

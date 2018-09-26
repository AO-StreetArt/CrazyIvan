#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "aossl/profile/include/tiered_app_profile.h"

#include "cache/include/cache_loader.h"

#ifndef SRC_APP_INCLUDE_LOADER_PROCESS_H_
#define SRC_APP_INCLUDE_LOADER_PROCESS_H_

void load_data(AOSSL::TieredApplicationProfile *config, \
    DeviceCacheLoader *loader, long interval) {
  while (true) {
    config->load_config();
    loader->load_scenes();
    usleep(interval);
  }
}

#endif  // SRC_APP_INCLUDE_LOADER_PROCESS_H_

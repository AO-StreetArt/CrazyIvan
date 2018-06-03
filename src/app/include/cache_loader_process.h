#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "cache/include/cache_loader.h"

#ifndef SRC_APP_INCLUDE_CACHE_LOADER_PROCESS_H_
#define SRC_APP_INCLUDE_CACHE_LOADER_PROCESS_H_

void load_device_cache(DeviceCacheLoader *loader, long interval) {
  while (true) {
    loader->load_scenes();
    usleep(interval);
  }
}

#endif  // SRC_APP_INCLUDE_CACHE_LOADER_PROCESS_H_

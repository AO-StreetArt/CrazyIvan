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

#include <stdlib.h>
#include <sstream>
#include <string>
#include <cstdlib>
#include <exception>

#include "aossl/redis/include/redis_interface.h"
#include "configuration_manager.h"
#include "ivan_log.h"

#ifndef SRC_APP_INCLUDE_REDIS_LOCKING_H_
#define SRC_APP_INCLUDE_REDIS_LOCKING_H_

class RedisLocker {
RedisInterface *redis = NULL;

 public:
  RedisLocker(RedisInterface *r) {redis = r;}
  ~RedisLocker() {}

  // Establish a Redis Mutex Lock on a given key
  inline void get_lock(std::string key, std::string val) {
    // Determine if another instance of CLyman has a lock on the Redis Mutex
    std::string current_mutex_key;
    bool lock_established = false;

    while (!lock_established) {
      redis_logging->debug("Redis Mutex Lock Routine Started");

      if (redis->exists(key)) {
        try {
          current_mutex_key = redis->load(key);
        }
        catch (std::exception& e) {
          redis_logging->error("Exception encountered during Redis Request");
          redis_logging->error(e.what());
        }
      }

      if (((current_mutex_key != "") && (current_mutex_key != val)) \
        || val.empty())  {
        // Another instance of Clyman has a lock on the redis mutex
        // Block until the lock is cleared
        redis_logging->error("Existing Redis Mutex Lock Detected");
        while (redis->exists(key)) {}
      }

      // Try to establish a lock on the Redis Mutex
      redis_logging->debug("Attempting to obtain Redis Mutex Lock");
      if (!(redis->exists(key))) {
        try {
          lock_established = redis->setnx(key, val);
        }
        catch (std::exception& e) {
          redis_logging->error("Exception encountered during Redis Request");
          redis_logging->error(e.what());
        }
      }

      if (lock_established) {redis_logging->debug("Redis Mutex Lock Acquired");}
    }
  }

  void get_lock(std::string key) {get_lock(key, "");}

  bool release_lock(std::string key, std::string val) {
    std::string current_mutex_key = "";
    if (redis->exists(key)) {
      current_mutex_key = redis->load(key);
    }
    if (val.empty() || (val == current_mutex_key)) {
      return redis->del(key);
    }
    return false;
  }
};

#endif  // SRC_APP_INCLUDE_REDIS_LOCKING_H_

#include <string>
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>


#include "Poco/HashMap.h"
#include "Poco/RWLock.h"

#ifndef SRC_CACHE_INCLUDE_DEVICE_CACHE_H_
#define SRC_CACHE_INCLUDE_DEVICE_CACHE_H_

// An entry in a thread-safe cache
class DeviceCacheEntry {
  std::vector<std::pair<std::string, int>> device_addresses_0;
  std::vector<std::pair<std::string, int>> device_addresses_1;
  std::mutex write_guard;
  std::atomic_int current_vector_id{0};
  std::atomic_int addresses0_counter{0};
  std::atomic_int addresses1_counter{0};
 public:
   // Constructors/Destructors
   DeviceCacheEntry() {}
   DeviceCacheEntry(std::vector<std::pair<std::string, int>>& addresses) {device_addresses_0 = std::move(addresses);}
   ~DeviceCacheEntry() {}
   // Get the latest complete list of addresses
   inline std::vector<std::pair<std::string, int>> get_addresses() {
     // Declare a local vector and copy the correct internal vector
     std::vector<std::pair<std::string, int>> return_vector;
     if (current_vector_id == 0) {
       addresses0_counter++;
       return_vector = device_addresses_0;
       addresses0_counter--;
     } else {
       addresses1_counter++;
       return_vector = device_addresses_1;
       addresses1_counter--;
     }
     // Return the local vector
     // Due to move semantics on vector, this should
     // copy the internal vector, then move local vector outside function
     return return_vector;
   }
   // Update the list of addresses
   // We keep the last known vector so that
   // we can continue serving requests while copying
   inline void set_addresses(std::vector<std::pair<std::string, int>>& updated_addresses) {
     // Get a lock on the write mutex
     std::lock_guard<std::mutex> lock(write_guard);
     // Based on the current vector that's available, we need to:
     //   - Wait until any get functions copying the internal vector are done
     //   - Move the supplied vector into the corresponding internal vector
     if (current_vector_id == 0) {
       while (addresses1_counter.load() > 0) {std::this_thread::yield();}
       device_addresses_1 = std::move(updated_addresses);
     } else {
       while (addresses0_counter.load() > 0) {std::this_thread::yield();}
       device_addresses_0 = std::move(updated_addresses);
     }
     current_vector_id = (current_vector_id + 1) % 2;
   }
};

// Equality Operator
inline bool operator==(DeviceCacheEntry& lhs, DeviceCacheEntry& rhs) {
  return (lhs.get_addresses() == rhs.get_addresses());
}

// Implements a hashmap which never locks for read operations unless scenes
// are added/removed.  Updating the contents of the scene requires no wait.
// Cannot be initialized with a copy constructor
class DeviceCache {
  Poco::HashMap<std::string, DeviceCacheEntry*> cache_entries;
  Poco::RWLock scene_update_lock;
  std::vector<std::string> scene_list;
 public:
  // Constructors/Destructors
  DeviceCache() {}
  ~DeviceCache() {for (std::string key : scene_list) {delete cache_entries[key];}}
  // Adding/removing scenes in the cache stops the world,
  // so it needs to be done very sparingly
  inline void add_scene(std::string scene_key) {
    scene_update_lock.writeLock();
    scene_list.push_back(scene_key);
    cache_entries[scene_key] = new DeviceCacheEntry();
    scene_update_lock.unlock();
  }
  inline void remove_scene(std::string scene_key) {
    scene_update_lock.writeLock();
    scene_list.erase(std::find(scene_list.begin(), scene_list.end(), scene_key));
    cache_entries.erase(scene_key);
    scene_update_lock.unlock();
  }
  inline std::vector<std::string>& get_scenes() {return scene_list;}
  // Thread-safe accessors
  inline std::vector<std::pair<std::string, int>> get_devices(std::string scene_key) {
    scene_update_lock.readLock();
    return cache_entries[scene_key]->get_addresses();
    scene_update_lock.unlock();
  }
  inline void set_devices(std::string scene_key, std::vector<std::pair<std::string, int>> devices) {
    scene_update_lock.readLock();
    cache_entries[scene_key]->set_addresses(devices);
    scene_update_lock.unlock();
  }
};

#endif  // SRC_CACHE_INCLUDE_DEVICE_CACHE_H_

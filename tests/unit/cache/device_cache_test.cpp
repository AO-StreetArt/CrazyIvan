#include <string>
#include <vector>
#include <utility>

#include "include/device_cache.h"

#include "catch.hpp"

TEST_CASE( "Test Device Cache - Single Thread", "[unit]" ) {
  // Setup a cache
  DeviceCache cache;
  std::string scene_name = "testScene";
  cache.add_scene(scene_name);
  std::vector<std::pair<std::string, int>> devices;
  std::string dev1 = "test1";
  std::string dev2 = "test2";
  std::pair<std::string, int> dev_addr1 (dev1, 1);
  std::pair<std::string, int> dev_addr2 (dev2, 2);
  devices.push_back(dev_addr1);
  devices.push_back(dev_addr2);
  cache.set_devices(scene_name, devices);

  // pull the devices out of the cache
  std::vector<std::pair<std::string, int>> found_devices = cache.get_devices(scene_name);
  REQUIRE(found_devices.size() == 2);
  REQUIRE(found_devices[0].first == dev1);
  REQUIRE(found_devices[1].first == dev2);
}

void update_cache(DeviceCache *cache, int port) {
  std::vector<std::pair<std::string, int>> devices;
  std::string scene_name = "testScene";
  std::string dev1 = "test1";
  std::pair<std::string, int> dev_addr1 (dev1, port);
  devices.push_back(dev_addr1);
  cache->set_devices(scene_name, devices);
}

void check_cache(DeviceCache *cache) {
  std::string dev1 = "test1";
  std::string scene_name = "testScene";
  std::vector<std::pair<std::string, int>> found_devices = cache->get_devices(scene_name);
  REQUIRE(found_devices.size() == 1);
}

TEST_CASE( "Test Device Cache - Multi Thread", "[unit]" ) {
  // Setup a cache
  DeviceCache cache;
  std::string scene_name = "testScene";
  cache.add_scene(scene_name);
  std::vector<std::pair<std::string, int>> devices;
  std::string dev1 = "test1";
  std::pair<std::string, int> dev_addr1 (dev1, 1);
  devices.push_back(dev_addr1);
  cache.set_devices(scene_name, devices);

  // Start some threads accessing the cache
  std::thread write_thread(update_cache, &cache, 2);
  std::thread read_thread(check_cache, &cache);

  // Get value out of the cache
  std::vector<std::pair<std::string, int>> found_devices1 = cache.get_devices(scene_name);
  REQUIRE(found_devices1.size() == 1);
  REQUIRE(found_devices1[0].first == dev1);

  // Wait for the threads to finish
  write_thread.join();
  read_thread.join();

  // Start a whole bunch of threads
  std::thread write_thread1(update_cache, &cache, 2);
  std::thread read_thread1(check_cache, &cache);
  std::thread write_thread2(update_cache, &cache, 3);
  std::thread read_thread2(check_cache, &cache);
  std::thread write_thread3(update_cache, &cache, 4);
  std::thread read_thread3(check_cache, &cache);
  std::thread write_thread4(update_cache, &cache, 5);
  std::thread read_thread4(check_cache, &cache);

  // Get value out of the cache
  std::vector<std::pair<std::string, int>> found_devices2 = cache.get_devices(scene_name);
  REQUIRE(found_devices2.size() == 1);
  REQUIRE(found_devices2[0].first == dev1);

  // Wait for the threads to finish
  write_thread1.join();
  read_thread1.join();
  write_thread2.join();
  read_thread2.join();
  write_thread3.join();
  read_thread3.join();
  write_thread4.join();
  read_thread4.join();
}

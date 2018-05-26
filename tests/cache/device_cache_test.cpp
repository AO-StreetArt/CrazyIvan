#include <string>
#include <vector>

#include "include/device_cache.h"

#include "catch.hpp"

TEST_CASE( "Test Device Cache - Single Thread", "[cache]" ) {
  // Setup a cache
  DeviceCache cache;
  std::string scene_name = "testScene";
  cache.add_scene(scene_name);
  std::vector<std::string> devices;
  std::string dev1 = "test1";
  std::string dev2 = "test2";
  devices.push_back(dev1);
  devices.push_back(dev2);
  cache.set_devices(scene_name, devices);

  // pull the devices out of the cache
  std::vector<std::string> found_devices = cache.get_devices(scene_name);
  REQUIRE(found_devices.size() == 2);
  REQUIRE(found_devices[0] == dev1);
  REQUIRE(found_devices[1] == dev2);
}

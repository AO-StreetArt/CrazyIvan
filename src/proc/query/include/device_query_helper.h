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

#include <string>

#include "model/include/transform_interface.h"
#include "model/include/transform_factory.h"
#include "model/include/user_device_interface.h"
#include "model/include/user_device_factory.h"
#include "model/include/scene_interface.h"
#include "model/include/scene_factory.h"

#include "api/include/scene_list_interface.h"
#include "api/include/scene_list_factory.h"

#include "neocpp/connection/interface/neo4j_interface.h"
#include "neocpp/connection/impl/libneo4j_factory.h"

#include "aossl/core/include/kv_store_interface.h"
#include "aossl/uuid/include/uuid_interface.h"

#include "base_query_helper.h"
#include "query_helper_interface.h"

#include "Poco/Logger.h"

#ifndef SRC_PROC_QUERY_INCLUDE_DEVICE_QUERY_HELPER_H_
#define SRC_PROC_QUERY_INCLUDE_DEVICE_QUERY_HELPER_H_

// The class contains helper methods for working with
// Scene Data in Neo4j
// Debice Query Helper deals with registrations
class DeviceQueryHelper : public BaseQueryHelper {
 public:
  inline DeviceQueryHelper(Neocpp::Neo4jInterface *neo, Neocpp::LibNeo4jFactory *nf, \
    AOSSL::KeyValueStoreInterface *con) : BaseQueryHelper(neo, nf, con) {}
  virtual ~DeviceQueryHelper() {}

// -----------------------Scene-Device Links--------------------------------- //

  // Determine if the given user device is registered to the given scene
  bool is_ud_registered(std::string inp_string, std::string inp_device);

  // Get scenes that this user device is registered to
  // Collect the User Device and Transformation within the scenes returned
  SceneListInterface* get_registrations(std::string inp_device);

  // Update the transformation between scene and device
  bool update_device_registration(std::string dev_id, std::string scene_id, \
    TransformInterface *transform);

  // Create a registration link in the DB
  void register_device_to_scene(std::string device_id, std::string scene_id, \
    TransformInterface *transform, bool device_exists, \
    std::string ud_conn_str, std::string ud_host, int ud_port);

  // Remove a device from a scene
  void remove_device_from_scene(std::string device_id, std::string scene_id);
};

#endif  // SRC_PROC_QUERY_INCLUDE_DEVICE_QUERY_HELPER_H_

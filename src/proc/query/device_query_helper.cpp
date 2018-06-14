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

#include "include/device_query_helper.h"

// Determine if the given user device is registered to the given scene
bool DeviceQueryHelper::is_ud_registered(std::string inp_string, \
  std::string inp_device) {
  BaseQueryHelper::logger().debug("Checking the User Devices registered to Scene %s", \
    inp_string);
  Neocpp::ResultsIteratorInterface *results = NULL;
  Neocpp::ResultTreeInterface *tree = NULL;
  Neocpp::DbObjectInterface *obj = NULL;
  Neocpp::DbMapInterface *map = NULL;
  Neocpp::Neo4jQueryParameterInterface *key_param = NULL;
  // Create the query string
  std::string q_string =
    "MATCH (scn:Scene {key: {inp_key}})-[tr:TRANSFORM]->(ud:UserDevice)"
    " RETURN ud";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neocpp::Neo4jQueryParameterInterface*> q_params;
  key_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(inp_string);
  q_params.emplace("inp_key", key_param);

  // Execute the query
  bool ret_val = false;
  bool has_exception = false;
  std::string exc_string = "";
  try {
    results = \
      BaseQueryHelper::get_neo4j_interface()->execute(q_string, q_params);
  }
  catch (std::exception& e) {
    BaseQueryHelper::logger().error("{\"Query\": \"%s\", \"Error\": \"%s\"", \
      q_string, e.what());
    has_exception = true;
    exc_string.assign(e.what());
  }

  if (!results) {
    BaseQueryHelper::logger().debug("User Device not found registered to scene");
    return false;
    // If we are registering the first device, then we may have a created scene
  } else {
    // Check if the registering user device is already registered
    BaseQueryHelper::logger().debug("User Devices detected");
    tree = results->next();
    while (tree) {
      // Get the first DB Object (Node)
      obj = tree->get(0);
      BaseQueryHelper::logger().debug("Query Result: %s", obj->to_string());

      if (!(obj->is_node())) break;

      // Pull the node properties and assign them to the new
      // Scene object
      map = obj->properties();
      std::string db_key = "";
      if (map->element_exists("key")) {
        db_key = map->get_string_element("key");
      }

      if (db_key == inp_device) {
        BaseQueryHelper::logger().debug("Existing registration detected");
        ret_val = true;
      }

      // Cleanup
      if (map) {delete map; map = NULL;}
      if (obj) {delete obj; obj = NULL;}
      if (tree) {delete tree; tree = NULL;}

      // Iterate to the next result
      tree = results->next();
    }
  }
  if (results) delete results;
  if (key_param) delete key_param;
  if (has_exception) throw QueryException(exc_string);
  return ret_val;
}

// Get scenes that this user device is registered to
// Collect the User Device and Transformation within the scenes returned
SceneListInterface* \
  DeviceQueryHelper::get_registrations(std::string inp_device) {
  BaseQueryHelper::logger().debug("Checking registrations for device %s", inp_device);
  Neocpp::ResultsIteratorInterface *results = NULL;
  Neocpp::ResultTreeInterface *tree = NULL;
  Neocpp::DbObjectInterface *obj = NULL;
  Neocpp::DbObjectInterface *edge = NULL;
  Neocpp::DbObjectInterface *device = NULL;
  Neocpp::DbMapInterface *edge_props = NULL;
  Neocpp::DbMapInterface *dev_props = NULL;
  Neocpp::Neo4jQueryParameterInterface *udkey_param = NULL;
  // Create the return object
  SceneListInterface *sc = BaseQueryHelper::create_json_scene();
  // Create the query string
  std::string udq_string =
    "MATCH (scn:Scene)-[tr:TRANSFORM]->(ud:UserDevice {key: {inp_key}})"
    " RETURN scn, tr, ud";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neocpp::Neo4jQueryParameterInterface*> udq_params;
  udkey_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(inp_device);
  udq_params.emplace("inp_key", udkey_param);

  // Execute the query
  try {
    results = \
      BaseQueryHelper::get_neo4j_interface()->execute(udq_string, udq_params);
  }
  catch (std::exception& e) {
    BaseQueryHelper::logger().error("{\"Query\": \"%s\", \"Error\": \"%s\"", \
      udq_string, e.what());
    return NULL;
  }

  int num_records = 0;
  if (!results) {
    BaseQueryHelper::logger().debug("No Scenes found for the given device");
    return NULL;
  } else {
    // Iterate through the results
    // Build the scene list
    tree = results->next();
    while (tree) {
      // Get the first DB Object (Node)
      obj = tree->get(0);
      if (!(obj->is_node()) && !(obj->is_edge())) break;
      edge = tree->get(1);
      if (!(edge->is_node()) && !(edge->is_edge())) break;
      device = tree->get(2);
      if (!(device->is_node()) && !(device->is_edge())) break;

      SceneInterface *new_data = BaseQueryHelper::create_scene();
      num_records = num_records + 1;
      BaseQueryHelper::logger().debug("{\"Query\": \"%s\", Result\": {\"object\": \"%s\", \"edge\": \"%s\", \"device\": \"%s\"}}", \
        obj->to_string(), edge->to_string(), device->to_string());

      // Pull the node properties and assign them to the new Scene object
      BaseQueryHelper::assign_scene_properties(obj, new_data);

      // Get the transform and device info
      double translation_x = -999.0;
      double translation_y = -999.0;
      double translation_z = -999.0;
      double rotation_x = -999.0;
      double rotation_y = -999.0;
      double rotation_z = -999.0;
      if (edge->is_edge())  {
        BaseQueryHelper::logger().debug("Getting Edge Properties");
        edge_props = edge->properties();
        // Get the transform attributes
        if (edge_props->element_exists("translation_x")) {
          translation_x = edge_props->get_float_element("translation_x");
        }
        if (edge_props->element_exists("translation_y")) {
          translation_y = edge_props->get_float_element("translation_y");
        }
        if (edge_props->element_exists("translation_z")) {
          translation_z = edge_props->get_float_element("translation_z");
        }
        if (edge_props->element_exists("rotation_x")) {
          rotation_x = edge_props->get_float_element("rotation_x");
        }
        if (edge_props->element_exists("rotation_y")) {
          rotation_y = edge_props->get_float_element("rotation_y");
        }
        if (edge_props->element_exists("rotation_z")) {
          rotation_z = edge_props->get_float_element("rotation_z");
        }
      }

      if (device->is_node()) {
        BaseQueryHelper::logger().debug("Getting Device Properties");
        // Get the transform and device properties
        dev_props = device->properties();

        if (dev_props) {
          if (dev_props->element_exists("key")) {
            BaseQueryHelper::logger().debug("Device Key Found");
            // Add the device related data to the scene
            TransformInterface *new_transform = \
              BaseQueryHelper::create_transform();
            UserDeviceInterface *new_dev = BaseQueryHelper::create_device(\
              dev_props->get_string_element("key"), new_transform);
            new_dev->set_translation(0, translation_x);
            new_dev->set_translation(1, translation_y);
            new_dev->set_translation(2, translation_z);
            new_dev->set_rotation(0, rotation_x);
            new_dev->set_rotation(1, rotation_y);
            new_dev->set_rotation(2, rotation_z);
            BaseQueryHelper::logger().debug("Adding device to scene data");
            new_data->add_device(new_dev);
          }
        } else {
          BaseQueryHelper::logger().error("Null map returned for device properties");
        }
      }

      BaseQueryHelper::logger().debug("Adding Scene data to scene");
      sc->add_scene(new_data);

      BaseQueryHelper::logger().debug("Cleanup");
      if (edge_props) {delete edge_props; edge_props = NULL;}
      if (dev_props) {delete dev_props; dev_props = NULL;}
      if (obj) {delete obj; obj = NULL;}
      if (edge) {delete edge; edge = NULL;}
      if (device) {delete device; device = NULL;}
      if (tree) {delete tree; tree = NULL;}

      BaseQueryHelper::logger().debug("Getting next result");
      tree = results->next();
    }
  }
  if (results) delete results;
  if (udkey_param) delete udkey_param;
  if (num_records == 0) {
    BaseQueryHelper::logger().error("No Scenes found for the given device");
    return NULL;
  }
  return sc;
}

// Create a registration link in the DB
// TO-DO: Add connectivity information to the Device
void DeviceQueryHelper::register_device_to_scene(std::string device_id, \
  std::string scene_id, TransformInterface *transform, bool device_exists, \
  std::string ud_conn_str, std::string ud_host, int ud_port) {
  BaseQueryHelper::logger().debug("Creating Device Registration link");
  Neocpp::ResultsIteratorInterface *results = NULL;
  Neocpp::Neo4jQueryParameterInterface *skey_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *udkey_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *locx_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *locy_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *locz_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *rotx_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *roty_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *rotz_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *conns_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *host_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *port_param = NULL;

  // Create the query string
  std::string udq_string = "MATCH (scn:Scene {key: {inp_key}})";
  if (device_exists) {
    udq_string = udq_string + ", (ud:UserDevice {key: {inp_ud_key}})"
    " CREATE (scn)-[trans:TRANSFORM {translation_x: {loc_x}, "
      "translation_y: {loc_y}, translation_z: {loc_z}, rotation_x: {rot_x}, "
      "rotation_y: {rot_y}, rotation_z: {rot_z}}]->(ud) ";
  } else {
    udq_string = udq_string +
      " CREATE (scn)-[trans:TRANSFORM {translation_x: {loc_x}, "
        "translation_y: {loc_y}, translation_z: {loc_z}, rotation_x: {rot_x}, "
        "rotation_y: {rot_y}, rotation_z: {rot_z}}]->(ud:UserDevice "
        "{key: {inp_ud_key}, connection_string: {ud_conn_str}, "
        "hostname: {ud_host}, port: {ud_port}}) ";
  }
  udq_string = udq_string + "RETURN scn, trans, ud";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neocpp::Neo4jQueryParameterInterface*> q_params;

  // Insert the scene key into the query list
  skey_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(scene_id);
  BaseQueryHelper::logger().debug("Scene Key: %s", scene_id);
  q_params.emplace("inp_key", skey_param);

  // Insert the device key into the query list
  udkey_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(device_id);
  BaseQueryHelper::logger().debug("Device Key: %s", device_id);
  q_params.emplace("inp_ud_key", udkey_param);

  // Insert translation
  locx_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->translation(0));
  locy_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->translation(1));
  locz_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->translation(2));
  q_params.emplace("loc_x", locx_param);
  q_params.emplace("loc_y", locy_param);
  q_params.emplace("loc_z", locz_param);

  // Insert rotation
  rotx_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->rotation(0));
  roty_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->rotation(1));
  rotz_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->rotation(2));
  q_params.emplace("rot_x", rotx_param);
  q_params.emplace("rot_y", roty_param);
  q_params.emplace("rot_z", rotz_param);

  // Insert connectivity information
  conns_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(ud_conn_str);
  host_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(ud_host);
  port_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(ud_port);
  q_params.emplace("ud_conn_str", conns_param);
  q_params.emplace("ud_host", host_param);
  q_params.emplace("ud_port", port_param);

  // Execute the query
  BaseQueryHelper::logger().debug("Executing Registration Query: %s", udq_string);
  try {
    results = \
      BaseQueryHelper::get_neo4j_interface()->execute(udq_string, q_params);
  }
  catch (std::exception& e) {
    BaseQueryHelper::logger().error("{\"Query\": \"%s\", \"Error\": \"%s\"", \
      udq_string, e.what());
  }

  if (!results) {
    BaseQueryHelper::logger().error("No Links created");
  } else {
    delete results;
  }
  if (skey_param) delete skey_param;
  if (udkey_param) delete udkey_param;
  if (locx_param) delete locx_param;
  if (locy_param) delete locy_param;
  if (locz_param) delete locz_param;
  if (rotx_param) delete rotx_param;
  if (roty_param) delete roty_param;
  if (rotz_param) delete rotz_param;
  if (conns_param) delete conns_param;
  if (host_param) delete host_param;
  if (port_param) delete port_param;
}

// Remove a registration link in the DB between the specified device and scene
void DeviceQueryHelper::remove_device_from_scene(std::string device_id, \
  std::string scene_id) {
  // Set up the Cypher Query for device deletion
  Neocpp::ResultsIteratorInterface *results = NULL;
  Neocpp::Neo4jQueryParameterInterface* skey_param = NULL;
  Neocpp::Neo4jQueryParameterInterface* udkey_param = NULL;
  std::string query_string =
    "MATCH (scn:Scene {key: {inp_key}})-[trans:TRANSFORM]->"
      "(ud:UserDevice {key: {inp_ud_key}}) "
    "DETACH DELETE ud RETURN scn";
  BaseQueryHelper::logger().debug("Executing Delete Query");

  // Set up the query parameters for query
  std::unordered_map<std::string, Neocpp::Neo4jQueryParameterInterface*> q_params;

  // Insert the scene key into the query list
  skey_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(scene_id);
  BaseQueryHelper::logger().debug("Scene Key: %s", scene_id);
  q_params.emplace("inp_key", skey_param);

  // Insert the device key into the query list
  udkey_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(device_id);
  BaseQueryHelper::logger().debug("Device Key: %s", device_id);
  q_params.emplace("inp_ud_key", udkey_param);

  // Execute the query
  try {
    results = \
      BaseQueryHelper::get_neo4j_interface()->execute(query_string, q_params);
  }
  catch (std::exception& e) {
    BaseQueryHelper::logger().error("{\"Query\": \"%s\", \"Error\": \"%s\"", \
      query_string, e.what());
  }

  if (!results) BaseQueryHelper::logger().error("No Links destroyed");
  else
    {delete results;}
  if (skey_param) delete skey_param;
  if (udkey_param) delete udkey_param;
}

// Update the device registration
// Return true if the update was successful
// False if we were unable to find an existing link to update
bool DeviceQueryHelper::update_device_registration(std::string dev_id, \
  std::string scene_id, TransformInterface *transform) {
  BaseQueryHelper::logger().debug("Updating Device Registration link");
  Neocpp::ResultsIteratorInterface *results = NULL;
  Neocpp::ResultTreeInterface *tree = NULL;
  Neocpp::DbObjectInterface *obj = NULL;
  Neocpp::Neo4jQueryParameterInterface *skey_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *udkey_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *locx_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *locy_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *locz_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *rotx_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *roty_param = NULL;
  Neocpp::Neo4jQueryParameterInterface *rotz_param = NULL;
  // Create the query string
  std::string udq_string =
    "MATCH (scn:Scene {key: {inp_key}})-[trans:TRANSFORM]->"
      "(ud:UserDevice  {key: {inp_ud_key}}) "
    "SET trans.translation_x = {loc_x}, trans.translation_y = {loc_y}, "
      "trans.translation_z = {loc_z}, trans.rotation_x = {rot_x}, "
      "trans.rotation_y = {rot_y}, trans.rotation_z = {rot_z} "
    "RETURN scn, trans, ud";

  // Set up the query parameters for query
  std::unordered_map<std::string, Neocpp::Neo4jQueryParameterInterface*> q_params;

  // Insert the scene key into the query list
  skey_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(scene_id);
  BaseQueryHelper::logger().debug("Scene Key: %s", scene_id);
  q_params.emplace("inp_key", skey_param);

  // Insert the device key into the query list
  udkey_param = \
    BaseQueryHelper::get_neo4j_factory()->get_neo4j_query_parameter(dev_id);
  BaseQueryHelper::logger().debug("Device Key: %s", dev_id);
  q_params.emplace("inp_ud_key", udkey_param);

  // Insert translation
  locx_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->translation(0));
  locy_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->translation(1));
  locz_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->translation(2));
  q_params.emplace("loc_x", locx_param);
  q_params.emplace("loc_y", locy_param);
  q_params.emplace("loc_z", locz_param);

  // Insert rotation
  rotx_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->rotation(0));
  roty_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->rotation(1));
  rotz_param = BaseQueryHelper::get_neo4j_factory()->\
    get_neo4j_query_parameter(transform->rotation(2));
  q_params.emplace("rot_x", rotx_param);
  q_params.emplace("rot_y", roty_param);
  q_params.emplace("rot_z", rotz_param);

  // Execute the query
  bool has_exception = false;
  bool result_found = true;
  std::string exc_string = "";
  try {
    results = \
      BaseQueryHelper::get_neo4j_interface()->execute(udq_string, q_params);
  }
  catch (std::exception& e) {
    BaseQueryHelper::logger().error("{\"Query\": \"%s\", \"Error\": \"%s\"", \
      udq_string, e.what());
    has_exception = true;
    std::string e_string(e.what());
    exc_string = e_string;
  }

  if (!results) {
    BaseQueryHelper::logger().error("No Links created");
    result_found = false;
  } else if ((!has_exception) && result_found) {
    BaseQueryHelper::logger().debug("Query Executed Successfully");
    tree = results->next();
    if (!tree) {
      BaseQueryHelper::logger().error("Query Returned no result tree");
      result_found = false;
    } else {
      obj = tree->get(0);
      if (!(obj->is_node())) {
        BaseQueryHelper::logger().error("Query Returned no values");
        result_found = false;
      } else {BaseQueryHelper::logger().debug(obj->to_string());}
    }
  }
  if (obj) delete obj;
  if (tree) delete tree;
  if (results) delete results;
  if (skey_param) delete skey_param;
  if (udkey_param) delete udkey_param;
  if (locx_param) delete locx_param;
  if (locy_param) delete locy_param;
  if (locz_param) delete locz_param;
  if (rotx_param) delete rotx_param;
  if (roty_param) delete roty_param;
  if (rotz_param) delete rotz_param;
  if (has_exception) throw QueryException(exc_string);
  return result_found;
}

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

#include "include/ivan_log.h"
#include "include/ivan_utils.h"

#include "include/transform_interface.h"
#include "include/transform_factory.h"
#include "include/user_device_interface.h"
#include "include/user_device_factory.h"
#include "include/scene_interface.h"
#include "include/scene_factory.h"

#include "include/scene_list_interface.h"
#include "include/scene_list_factory.h"

#include "aossl/neo4j/include/neo4j_interface.h"
#include "aossl/neo4j/include/factory_neo4j.h"

#ifndef SRC_PROC_QUERY_INCLUDE_BASE_QUERY_HELPER_H_
#define SRC_PROC_QUERY_INCLUDE_BASE_QUERY_HELPER_H_

// The class contains helper methods for working with
// Scene Data in Neo4j
class BaseQueryHelper {
  ConfigurationManager *config = NULL;
  Neo4jInterface *n = NULL;
  Neo4jComponentFactory *neo_factory = NULL;
  SceneFactory sfactory;
  SceneListFactory slfactory;
  TransformFactory tfactory;
  UserDeviceFactory udfactory;

 public:
  inline BaseQueryHelper(Neo4jInterface *neo, \
    Neo4jComponentFactory *nf, ConfigurationManager *con) {
    n = neo;
    neo_factory = nf;
    config = con;
  }
  virtual ~BaseQueryHelper() {}

  // Create new transformations
  inline TransformInterface* create_transform() \
    {return tfactory.build_transform();}
  inline TransformInterface* \
    create_transform(protoScene::SceneList_Transformation data) \
    {return tfactory.build_transform(data);}

  // Create new User Devices
  inline UserDeviceInterface* create_device() {return udfactory.build_device();}
  inline UserDeviceInterface* \
    create_device(protoScene::SceneList_UserDevice ud_data) \
    {return udfactory.build_device(ud_data);}
  inline UserDeviceInterface* \
    create_device(std::string inp_key, TransformInterface *t) \
    {return udfactory.build_device(inp_key, t);}
  inline UserDeviceInterface* \
    create_device(std::string inp_key) \
    {return udfactory.build_device(inp_key);}

  // Create new Scenes
  inline SceneInterface* create_scene() {return sfactory.build_scene();}
  inline SceneInterface* create_scene(protoScene::SceneList_Scene scn_data) \
    {return sfactory.build_scene(scn_data);}

  // Create new SceneLists
  inline SceneListInterface* create_json_scene() \
    {return slfactory.build_json_scene();}
  inline SceneListInterface* create_protobuf_scene() \
    {return slfactory.build_protobuf_scene();}
  inline SceneListInterface* create_scene(const rapidjson::Document& d) \
    {return slfactory.build_scene(d);}
  inline SceneListInterface* create_scene(protoScene::SceneList buffer) \
    {return slfactory.build_scene(buffer);}

  // Get other basic pointers
  ConfigurationManager* get_config_manager() {return config;}
  Neo4jInterface* get_neo4j_interface() {return n;}
  Neo4jComponentFactory* get_neo4j_factory() {return neo_factory;}

  // Utility Methods

  // Assign the properties from a DB Scene to a Scene Interface
  inline void assign_scene_properties(DbObjectInterface *db_scene, \
    SceneInterface *data) {
    processor_logging->debug("Assigning Scene Properties");
    DbMapInterface* map = db_scene->properties();
    if (map->element_exists("key")) {
      data->set_key(map->get_string_element("key"));
    }
    if (map->element_exists("name")) {
      data->set_name(map->get_string_element("name"));
    }
    if (map->element_exists("region")) {
      data->set_region(map->get_string_element("region"));
    }
    if (map->element_exists("latitude")) {
      data->set_latitude(map->get_float_element("latitude"));
    }
    if (map->element_exists("active")) {
      data->set_active(map->get_bool_element("active"));
    }
    if (map->element_exists("longitude")) {
      data->set_longitude(map->get_float_element("longitude"));
    }
    if (map->element_exists("assets")) {
      DbListInterface *asset_list = map->get_list_element("assets");
      for (unsigned int i = 0; i < asset_list->size(); i++) {
        data->add_asset(asset_list->get_string_element(i));
      }
      delete asset_list;
    }
    if (map->element_exists("tags")) {
      DbListInterface *tag_list = map->get_list_element("tags");
      for (unsigned int j = 0; j < tag_list->size(); j++) {
        data->add_tag(tag_list->get_string_element(j));
      }
      delete tag_list;
    }
    delete map;
  }

  // Assign the properties from a DB Device in a scene to a Device Interface
  inline void assign_device_properties(DbObjectInterface *db_device, \
      UserDeviceInterface *data) {
    processor_logging->debug("Assigning Device Properties");
    DbMapInterface* map = db_device->properties();
    if (map->element_exists("connection_string")) {
      data->set_connection_string(map->get_string_element("connection_string"));
    }
    if (map->element_exists("hostname")) {
      data->set_hostname(map->get_string_element("hostname"));
    }
    if (map->element_exists("port")) {
      data->set_port(map->get_int_element("port"));
    }
    delete map;
  }

  // Generate a scene Crud query
  inline void generate_scene_crud_query(std::string key, int crud_op, \
      int op_type, SceneInterface *scn, std::string &query_str) {
    bool key_query = false;
    // Start the query
    if (crud_op == CREATE_QUERY_TYPE) {
      query_str.append("CREATE (scn:Scene");
    } else {
      query_str.append("MATCH (scn:Scene");
    }
    // Add the key
    if (!(key.empty())) {
      query_str.append(" {key: {inp_key}");
      key_query = true;
      if (crud_op == GET_QUERY_TYPE) {query_str.append("})");}
    }

    // Build an update query
    if (crud_op == UPDATE_QUERY_TYPE) {
      query_str.append("}) SET scn.active = {inp_active}");
      if (!(scn->get_name().empty())) {
        query_str.append(", scn.name = {inp_name}");
      }
      if (!(scn->get_latitude() == -9999.0)) {
        query_str.append(", scn.latitude = {inp_lat}");
      }
      if (!(scn->get_longitude() == -9999.0)) {
        query_str.append(", scn.longitude = {inp_long}");
      }
      if (!(scn->get_region().empty())) {
        query_str.append(", scn.region = {inp_region}");
      }
      if (scn->num_assets() > 0) {
        if (op_type == APPEND) {
          query_str.append(", scn.assets = coalesce(scn.assets, []) + {inp_assets}");
        } else if (op_type == REMOVE) {
          query_str.append(", scn.assets = FILTER(asset IN scn.assets WHERE NOT (asset IN {inp_assets}))");
        }
      }
      if (scn->num_tags() > 0) {
        if (op_type == APPEND) {
          query_str.append(", scn.tags = coalesce(scn.tags, []) + {inp_tags}");
        } else if (op_type == REMOVE) {
          query_str.append(", scn.tags = FILTER(tag IN scn.tags WHERE NOT (tag IN {inp_tags}))");
        }
      }

    // Build a delete query
    } else if (crud_op == DELETE_QUERY_TYPE) {
      query_str.append("}) DETACH DELETE scn");

    // Build a get query
    } else if (crud_op == GET_QUERY_TYPE && !key_query) {
      bool inp_name_empty = scn->get_name().empty();
      bool inp_region_empty = scn->get_region().empty();
      if ((!inp_name_empty) || (!inp_region_empty)) {
        query_str.append(" {");
        // Start by checking for a name
        if (!inp_name_empty) {
          query_str.append("name: {inp_name}");
        }
        if ((!inp_name_empty) && (!inp_region_empty)) {
          query_str.append(", ");
        }
        // Check for Region
        if (!inp_region_empty) {
          query_str.append("region: {inp_region}");
        }
        query_str.append("}");
      }
      query_str.append(")");
      bool is_started = false;
      // Check for Tags
      // We will only query for the first tag supplied
      if (scn->num_tags() > 0) {
        query_str.append(" WHERE {inp_tag} in scn.tags");
        is_started = true;
      }
      // Check for a distance-based query
      if (!(scn->get_latitude() == -9999.0 || \
        scn->get_longitude() == -9999.0 || \
        scn->get_distance() < 0.0)) {
        if (is_started) {query_str.append(" AND");}
        else {query_str.append(" WHERE");}
        // Query for distance
        // Assumes distance supplied is in meters
        // Haversine formula (https://en.wikipedia.org/wiki/Haversine_formula)
        std::string where_clause = \
              " ( "
                "12742000 * asin("
                  "sqrt("
                    "haversin(radians({inp_lat} - scn.latitude)) + "
                    "cos(radians({inp_lat})) * cos(radians(scn.latitude)) * "
                      "haversin(radians(scn.longitude - {inp_long}))"
                  ")"
                ")"
              ") < {inp_distance}";
        query_str.append(where_clause);
      }

    // Build a create query
    } else if (crud_op == CREATE_QUERY_TYPE) {
      query_str.append(", active: {inp_active}");
      if (!(scn->get_name().empty())) {
        query_str.append(", name: {inp_name}");
      }
      if (!(scn->get_latitude() == -9999.0)) {
        query_str.append(", latitude: {inp_lat}");
      }
      if (!(scn->get_longitude() == -9999.0)) {
        query_str.append(", longitude: {inp_long}");
      }
      if (!(scn->get_region().empty())) {
        query_str.append(", region: {inp_region}");
      }
      if (scn->num_assets() > 0) {
        query_str.append(", assets: {inp_assets}");
      }
      if (scn->num_tags() > 0) {
        query_str.append(", tags: {inp_tags}");
      }
      query_str.append("})");
    }
    query_str.append(" RETURN scn");
  }

  // Generate a map of query parameters from a scene
  inline void generate_scene_query_parameters(std::string key, SceneInterface *scn, \
      std::unordered_map<std::string, Neo4jQueryParameterInterface*> &scene_params) {
    // Key
    if (!(key.empty())) {
      Neo4jQueryParameterInterface *key_param = neo_factory->get_neo4j_query_parameter(key);
      processor_logging->debug("Key:");
      processor_logging->debug(key);
      scene_params.emplace("inp_key", key_param);
    }
    if (scn) {
      // Active
      Neo4jQueryParameterInterface *active_param = neo_factory->get_neo4j_query_parameter(scn->active());
      scene_params.emplace("inp_active", active_param);
      // Name
      if (!(scn->get_name().empty())) {
        std::string qname = scn->get_name();
        Neo4jQueryParameterInterface *name_param = neo_factory->get_neo4j_query_parameter(qname);
        processor_logging->debug("Name:");
        processor_logging->debug(qname);
        scene_params.emplace("inp_name", name_param);
      }
      // Latitude
      if (!(scn->get_latitude() == -9999.0)) {
        double qlat = scn->get_latitude();
        Neo4jQueryParameterInterface *lat_param = neo_factory->get_neo4j_query_parameter(qlat);
        scene_params.emplace("inp_lat", lat_param);
      }
      // Longitude
      if (!(scn->get_longitude() == -9999.0)) {
        double qlong = scn->get_longitude();
        Neo4jQueryParameterInterface *long_param = neo_factory->get_neo4j_query_parameter(qlong);
        scene_params.emplace("inp_long", long_param);
      }
      // Region
      if (!(scn->get_region().empty())) {
        Neo4jQueryParameterInterface *region_param = neo_factory->get_neo4j_query_parameter(scn->get_region());
        processor_logging->debug("Region:");
        processor_logging->debug(scn->get_region());
        scene_params.emplace("inp_region", region_param);
      }
      // Assets
      if (scn->num_assets() > 0) {
        Neo4jQueryParameterInterface *asset_param = neo_factory->get_neo4j_query_parameter();
        // Add the assets from the object message to the parameter
        for (int i = 0; i < scn->num_assets(); i++) {
          asset_param->add_value(scn->get_asset(i));
          processor_logging->debug("Asset:");
          processor_logging->debug(scn->get_asset(i));
        }
        scene_params.emplace("inp_assets", asset_param);
      }
      // Tags
      if (scn->num_tags() > 0) {
        Neo4jQueryParameterInterface *tag_param = neo_factory->get_neo4j_query_parameter();
        // Add the assets from the object message to the parameter
        for (int j = 0; j < scn->num_tags(); j++) {
          tag_param->add_value(scn->get_tag(j));
          processor_logging->debug("Tag:");
          processor_logging->debug(scn->get_tag(j));
        }
        scene_params.emplace("inp_tags", tag_param);
      }
    }
  }
};

#endif  // SRC_PROC_QUERY_INCLUDE_BASE_QUERY_HELPER_H_

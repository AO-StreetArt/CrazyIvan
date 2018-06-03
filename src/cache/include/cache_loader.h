#include <string>

#include "Poco/Logger.h"

#include "device_cache.h"

#include "neocpp/connection/interface/neo4j_interface.h"
#include "neocpp/connection/impl/libneo4j_factory.h"

#ifndef SRC_CACHE_INCLUDE_CACHE_LOADER_H_
#define SRC_CACHE_INCLUDE_CACHE_LOADER_H_

// The Loder is responsible for moving data from Neo4j into the Device Cache
// It needs to be passed in a device cache and Neo4j interface, but does not
// take over responsibility for destructing those objects.
class DeviceCacheLoader {
  DeviceCache *cache = NULL;
  Neocpp::Neo4jInterface *neo = NULL;
public:
  // Constructors & Destructors
  DeviceCacheLoader(DeviceCache *dcache, Neocpp::Neo4jInterface *neoconn) \
      {cache = dcache; neo = neoconn;}
  ~DeviceCacheLoader() {}
  DeviceCacheLoader(const DeviceCacheLoader& other) \
      {set_cache(other.get_cache()); set_neo4j(other.get_neo4j());}
  DeviceCache* get_cache() const {return cache;}
  Neocpp::Neo4jInterface* get_neo4j() const {return neo;}
  void set_cache(DeviceCache *new_cache) {cache = new_cache;}
  void set_neo4j(Neocpp::Neo4jInterface *new_neo) {neo = new_neo;}
  // Cache Loading
  inline void load_scene(std::string scene_key) {
    Neocpp::LibNeo4jFactory neo_factory;
    // Build a new vector of devices to load into the cache
    std::vector<std::pair<std::string, int>> new_devices;
    // Set up the neo4j query
    std::string query = \
        "MATCH (scn:Scene {key: {inp_key}})-[tr:TRANSFORM]->(ud:UserDevice) RETURN ud";
    // Set up the neo4j query parameter map
    std::unordered_map<std::string, Neocpp::Neo4jQueryParameterInterface*> query_params;
    Neocpp::Neo4jQueryParameterInterface *key_param = \
        neo_factory.get_neo4j_query_parameter(scene_key);
    query_params.emplace("inp_key", key_param);
    // Execute the query
    Neocpp::ResultsIteratorInterface *results = NULL;
    try {
      results = neo->execute(query, query_params);
    }
    catch (std::exception& e) {
      Poco::Logger::get("Cache").error("{\"Query\": \"%s\", \"Error\": \"%s\"", \
        query, std::string(e.what()));
    }
    // Get the query results
    if (!results) {
      Poco::Logger::get("Cache").error("No Results from Query: %s", query);
    } else {
      Neocpp::ResultTreeInterface *tree = NULL;
      Neocpp::DbObjectInterface* obj = NULL;
      Neocpp::DbMapInterface *map = NULL;

      // Loop through the returned devices
      tree = results->next();
      while (tree) {
        // Get the first DB Object (Node)
        obj = tree->get(0);
        Poco::Logger::get("MessageProcessor").debug("Query Result: %s", obj->to_string());

        // Break out if we're returned something not an object node
        if (!(obj->is_node())) break;

        // Get the connection off the db object
        map = obj->properties();
        std::string conn_host = "";
        if (map->element_exists("hostname")) {
          conn_host = map->get_string_element("hostname");
        }
        int conn_port = 0;
        if (map->element_exists("port")) {
          conn_port = map->get_int_element("port");
        }

        // Store the connection object in the new scene vector
        // does the handling of the int here create issues.  It's stored
        // cache after the underying db object is deleted, need to make
        // sure it's copied not passed by reference.
        std::pair<std::string, int> new_device(conn_host, conn_port);
        new_devices.push_back(new_device);

        // Cleanup
        if (map) {delete map; map = NULL;}
        if (obj) {delete obj; obj = NULL;}
        if (tree) {delete tree; tree = NULL;}

        // Get the next result from the tree
        tree = results->next();
      }

      // Load the new device vector into the cache
      cache->set_devices(scene_key, new_devices);

      // Cleanup
      if (map) delete map;
      if (obj) delete obj;
      if (tree) delete tree;
      delete results;
    }
  }
  // Re-load all scenes currently in the cache
  inline void load_scenes() {
    std::vector<std::string> scene_keys = cache->get_scenes();
    for (std::string key : scene_keys) {
      load_scene(key);
    }
  }
};

#endif  // SRC_CACHE_INCLUDE_CACHE_LOADER_H_

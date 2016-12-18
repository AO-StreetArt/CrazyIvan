#include "ivan_log.h"
#include "ivan_utils.h"
#include "redis_locking.h"
#include "configuration_manager.h"
#include "scene.h"

#include "rapidjson/document.h"

#include "aossl/factory/redis_interface.h"
#include "aossl/factory/neo4j_interface.h"
#include "aossl/factory/zmq_interface.h"

//The class containing core logic for CrazyIvan
//Accepts an Scene (assuming it is an inbound message)
//And performs any and all processing on it,
//Includes Redis Locks, DB Interactions, and OB ZMQ messaging

#ifndef MSG_PROCESSOR
#define MSG_PROCESSOR

class MessageProcessor {

Neo4jInterface *n = NULL;
Neo4jComponentFactory *neo_factory = NULL;
RedisInterface *r = NULL;
ConfigurationManager *config = NULL;
RedisLocker *redis_locks = NULL;
uuidInterface *ugen = NULL;

inline std::string create_uuid() {
  UuidContainer id_container;
  id_container = ugen->generate();
  if (!id_container.err.empty()) {
    uuid_logging->error(id_container.err);
  }
  return id_container.id;
}

inline void get_mutex_lock(std::string obj_key) {
  std::string key = "IVAN-" + obj_key;
  std::string val = "IVAN-" + config->get_nodeid();
  redis_locks->get_lock(key, val);
}

inline void release_mutex_lock(std::string obj_key) {
  std::string key = "IVAN-" + obj_key;
  std::string val = "IVAN-" + config->get_nodeid();
  redis_locks->release_lock(key, val);
}

//Create a new scene
inline std::string process_create_message(Scene *obj_msg) {
  ResultsIteratorInterface *results = NULL;
  processor_logging->debug("Processing Scene Creation message");

  //Generate a new key
  std::string new_key = create_uuid();

  //Set up the Cypher Query for scene creation
  std::string scene_query = "CREATE (scn:Scene {key: {inp_key}, name: {inp_name}, latitude: {inp_lat}, longitude: {inp_long}}) RETURN scn";

  //Set up the query parameters for scene creation
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
  Neo4jQueryParameterInterface* key_param = neo_factory->get_neo4j_query_parameter(new_key);
  processor_logging->debug("Key:");
  processor_logging->debug(new_key);
  scene_params.emplace("inp_key", key_param);
  if ( !(obj_msg->get_scene(0)->get_name().empty()) ) {
    std::string qname = obj_msg->get_scene(0)->get_name();
    Neo4jQueryParameterInterface* name_param = neo_factory->get_neo4j_query_parameter(qname);
    processor_logging->debug("Name:");
    processor_logging->debug(qname);
    scene_params.emplace("inp_name", name_param);
  }
  if ( !(obj_msg->get_scene(0)->get_latitude() == -9999.0) ) {
    double qlat = obj_msg->get_scene(0)->get_latitude();
    Neo4jQueryParameterInterface* lat_param = neo_factory->get_neo4j_query_parameter(qlat);
    scene_params.emplace("inp_lat", lat_param);
  }
  if ( !(obj_msg->get_scene(0)->get_longitude() == -9999.0) ) {
    double qlong = obj_msg->get_scene(0)->get_longitude();
    Neo4jQueryParameterInterface* long_param = neo_factory->get_neo4j_query_parameter(qlong);
    scene_params.emplace("inp_long", long_param);
  }

  //Execute the query
  try {
    results = n->execute(scene_query, scene_params);
    if (!results) {
      processor_logging->error("No results returned from creation query");
      return "-1";
    }
    else {
      return new_key;
    }
  }
  catch (std::exception& e) {
    processor_logging->error("Error running Query:");
    processor_logging->error(e.what());
    return "-1";
  }
  if (results) {
    delete results;
  }
}

//Update the details of a scene entry
inline std::string process_update_message(Scene *obj_msg) {
  get_mutex_lock(obj_msg->get_scene(0)->get_key());
  ResultsIteratorInterface *results = NULL;
  processor_logging->debug("Processing Scene Update message");
  bool is_started = false;

  if ((obj_msg->get_scene(0)->get_name().empty() && obj_msg->get_scene(0)->get_latitude() == -9999.0 && obj_msg->get_scene(0)->get_longitude() == -9999.0) || obj_msg->get_scene(0)->get_key().empty()) {
    processor_logging->error("No fields found in update message");
    return "-1";
  }
  else {
    //Set up the Cypher Query for scene update
    std::string scene_query = "MATCH (scn:Scene {key: {inp_key}}) SET ";
    if ( !(obj_msg->get_scene(0)->get_name().empty()) ) {
      scene_query = scene_query + "scn.name = {inp_name}";
      is_started = true;
    }

    if ( !(obj_msg->get_scene(0)->get_latitude() == -9999.0) ) {
      if (is_started) {
        scene_query = scene_query + ", ";
      }
      scene_query = scene_query + "scn.latitude = {inp_lat}";
      is_started = true;
    }

    if ( !(obj_msg->get_scene(0)->get_longitude() == -9999.0) ) {
      if (is_started) {
        scene_query = scene_query + ", ";
      }
      scene_query = scene_query + "scn.longitude = {inp_long}";
      is_started = true;
    }

    scene_query = scene_query + " RETURN scn";

    processor_logging->debug("Executing Query:");
    processor_logging->debug(scene_query);

    //Set up the query parameters for scene update
    std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
    std::string qkey = obj_msg->get_scene(0)->get_key();
    Neo4jQueryParameterInterface* key_param = neo_factory->get_neo4j_query_parameter(qkey);
    processor_logging->debug("Key:");
    processor_logging->debug(qkey);
    scene_params.emplace("inp_key", key_param);
    if ( !(obj_msg->get_scene(0)->get_name().empty()) ) {
      std::string qname = obj_msg->get_scene(0)->get_name();
      Neo4jQueryParameterInterface* name_param = neo_factory->get_neo4j_query_parameter(qname);
      processor_logging->debug("Name:");
      processor_logging->debug(qname);
      scene_params.emplace("inp_name", name_param);
    }
    if ( !(obj_msg->get_scene(0)->get_latitude() == -9999.0) ) {
      double qlat = obj_msg->get_scene(0)->get_latitude();
      Neo4jQueryParameterInterface* lat_param = neo_factory->get_neo4j_query_parameter(qlat);
      scene_params.emplace("inp_lat", lat_param);
    }
    if ( !(obj_msg->get_scene(0)->get_longitude() == -9999.0) ) {
      double qlong = obj_msg->get_scene(0)->get_longitude();
      Neo4jQueryParameterInterface* long_param = neo_factory->get_neo4j_query_parameter(qlong);
      scene_params.emplace("inp_long", long_param);
    }

    //Execute the query
    try {
      results = n->execute(scene_query, scene_params);
      if (!results) {
        processor_logging->error("No results returned from update query");
        return "-1";
      }
      else {
        return qkey;
      }
    }
    catch (std::exception& e) {
      processor_logging->error("Error running Query:");
      processor_logging->error(e.what());
      return "-1";
    }
  }
  if (results) {
    delete results;
  }
  release_mutex_lock(obj_msg->get_scene(0)->get_key());
}

//Query for scene data
inline std::string process_retrieve_message(Scene *obj_msg) {
  ResultsIteratorInterface *results = NULL;
  processor_logging->debug("Processing Scene Update message");
  bool is_started = false;

  if (obj_msg->get_scene(0)->get_name().empty() && obj_msg->get_scene(0)->get_latitude() == -9999.0 && obj_msg->get_scene(0)->get_longitude() == -9999.0 && obj_msg->get_scene(0)->get_key().empty()) {
    processor_logging->error("No fields found in update message");
    return "-1";
  }
  else {
    //Set up the Cypher Query for scene creation
    std::string scene_query = "MATCH (scn:Scene {";
    if ( !(obj_msg->get_scene(0)->get_name().empty()) ) {
      scene_query = scene_query + "name = {inp_name}";
      is_started = true;
    }

    if ( !(obj_msg->get_scene(0)->get_key().empty()) ) {
      if (is_started) {
        scene_query = scene_query + ", ";
      }
      scene_query = scene_query + "key = {inp_key}";
      is_started = true;
    }

    if ( !(obj_msg->get_scene(0)->get_latitude() == -9999.0) ) {
      if (is_started) {
        scene_query = scene_query + ", ";
      }
      scene_query = scene_query + "latitude = {inp_lat}";;
      is_started = true;
    }

    if ( !(obj_msg->get_scene(0)->get_longitude() == -9999.0) ) {
      if (is_started) {
        scene_query = scene_query + ", ";
      }
      scene_query = scene_query + "longitude = {inp_long}";
      is_started = true;
    }

    scene_query = scene_query + "}) RETURN scn";

    processor_logging->debug("Executing Query:");
    processor_logging->debug(scene_query);

    //Set up the query parameters for scene retrieval
    std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
    if ( !(obj_msg->get_scene(0)->get_key().empty()) ) {
      std::string qkey = obj_msg->get_scene(0)->get_key();
      Neo4jQueryParameterInterface* key_param = neo_factory->get_neo4j_query_parameter(qkey);
      processor_logging->debug("Key:");
      processor_logging->debug(qkey);
      scene_params.emplace("inp_key", key_param);
    }
    if ( !(obj_msg->get_scene(0)->get_name().empty()) ) {
      std::string qname = obj_msg->get_scene(0)->get_name();
      Neo4jQueryParameterInterface* name_param = neo_factory->get_neo4j_query_parameter(qname);
      processor_logging->debug("Name:");
      processor_logging->debug(qname);
      scene_params.emplace("inp_name", name_param);
    }
    if ( !(obj_msg->get_scene(0)->get_latitude() == -9999.0) ) {
      double qlat = obj_msg->get_scene(0)->get_latitude();
      Neo4jQueryParameterInterface* lat_param = neo_factory->get_neo4j_query_parameter(qlat);
      scene_params.emplace("inp_lat", lat_param);
    }
    if ( !(obj_msg->get_scene(0)->get_longitude() == -9999.0) ) {
      double qlong = obj_msg->get_scene(0)->get_longitude();
      Neo4jQueryParameterInterface* long_param = neo_factory->get_neo4j_query_parameter(qlong);
      scene_params.emplace("inp_long", long_param);
    }

    //Execute the query
    try {
      results = n->execute(scene_query, scene_params);
      if (!results) {
        processor_logging->error("No results returned from update query");
        return "-1";
      }
      else {
        //Pull results and return
        Scene sc;
        sc.set_err_code(NO_ERROR);
        sc.set_msg_type(SCENE_GET);
        sc.set_transaction_id(obj_msg->get_transaction_id());
        ResultTreeInterface *tree = results->next();
        while (tree) {

          SceneData * data = sc.add_scene();

          //Get the first DB Object (Node)
          DbObjectInterface* obj = tree->get(0);
          processor_logging->debug("Query Result:");
          processor_logging->debug(obj->to_string());

          //Pull the node properties and assign them to the new
          //Scene object
          DbMapInterface* map = obj->properties();
          if (map->element_exists("key")) {
            data->set_key( map->get_string_element("key") );
          }
          if (map->element_exists("name")) {
            data->set_name( map->get_string_element("name") );
          }
          if (map->element_exists("latitude")) {
            data->set_latitude( map->get_float_element("latitude") );
          }
          if (map->element_exists("longitude")) {
            data->set_longitude( map->get_float_element("longitude") );
          }

          //Iterate to the next result
          if (tree) {
            delete tree;
          }
          if (obj) {
            delete obj;
          }
          if (map) {
            delete map;
          }
          tree = results->next();
        }
        return sc.to_protobuf();
      }
    }
    catch (std::exception& e) {
      processor_logging->error("Error running Query:");
      processor_logging->error(e.what());
      return "-1";
    }
  }
  if (results) {
    delete results;
  }
}

//Delete a scene
inline std::string process_delete_message(Scene *obj_msg) {
  get_mutex_lock(obj_msg->get_scene(0)->get_key());
  processor_logging->debug("Processing Scene Update message");
  ResultsIteratorInterface *results = NULL;

  if ( obj_msg->get_scene(0)->get_key().empty() ) {
    processor_logging->error("No fields found in delete message");
    return "-1";
  }
  else {
    //Set up the Cypher Query for scene delete
    std::string scene_query = "MATCH (scn:Scene {key: {inp_key}}) DETACH DELETE scn";
    processor_logging->debug("Executing Delete Query");

    //Set up the query parameters for scene delete
    std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
    std::string qkey = obj_msg->get_scene(0)->get_key();
    Neo4jQueryParameterInterface* key_param = neo_factory->get_neo4j_query_parameter(qkey);
    processor_logging->debug("Key:");
    processor_logging->debug(qkey);
    scene_params.emplace("inp_key", key_param);

    //Execute the query
    try {
      results = n->execute(scene_query, scene_params);
      if (!results) {
        processor_logging->error("No results returned from update query");
        return "-1";
      }
      else {
        return qkey;
      }
    }
    catch (std::exception& e) {
      processor_logging->error("Error running Query:");
      processor_logging->error(e.what());
      return "-1";
    }
  }
  release_mutex_lock(obj_msg->get_scene(0)->get_key());
}

//Register a device to a scene
//Predict a coordinate transform for the device
//and pass it back in the response
inline std::string process_registration_message(Scene *obj_msg) {
  return "Not Implemented";
}

//Remove a device from a scene
//If the object is a member of other scenes:
//  -If a coordinate system transformation doesn't exist, then create one
inline std::string process_deregistration_message(Scene *obj_msg) {
  return "Not Implemented";
}

//Align an objects transformation with a scene
//If the object is a member of other scenes:
//  -If a coordinate system transformation exists, then update it with a correction
inline std::string process_device_alignment_message(Scene *obj_msg) {
  return "Not Implemented";
}

//Ping the Crazy Ivan Instance
inline std::string process_ping_message(Scene *obj_msg) {
  processor_logging->debug("Ping Pong");
  return "";
}

//Kill the Crazy Ivan Instance
inline std::string process_kill_message(Scene *obj_msg) {
  return "";
}
public:

  //Constructor & Destructor
  inline MessageProcessor(Neo4jComponentFactory *nf, Neo4jInterface *neo4j, RedisInterface *rd, ConfigurationManager *con, uuidInterface *u) {
    n=neo4j;
    r = rd;
    config = con;
    redis_locks = new RedisLocker(rd);
    ugen=u;
    neo_factory = nf;
  }
  ~MessageProcessor() {delete redis_locks;}

  //Process a message in the form of an Scene
  //In the case of a get message, return the retrieved document back to the main method
  //In the case of a create message, return the key of the created object back to the main method
  //Otherwise, return a blank string in the case of success, and "-1" in the case of failure
  inline std::string process_message(Scene *obj_msg) {

    //Determine what type of message we have, and act accordingly
    int msg_type = obj_msg->get_msg_type();
    processor_logging->debug("Message Type: " + std::to_string(msg_type));
    std::string process_return;

    if (msg_type == SCENE_CRT) {
      process_return = process_create_message(obj_msg);
    }
    else if (msg_type == SCENE_UPD) {
      process_return = process_update_message(obj_msg);
    }
    else if (msg_type == SCENE_GET) {
      process_return = process_retrieve_message(obj_msg);
    }
    else if (msg_type == SCENE_DEL) {
      process_return = process_delete_message(obj_msg);
    }
    else if (msg_type == SCENE_ENTER) {
      process_return = process_registration_message(obj_msg);
    }
    else if (msg_type == SCENE_LEAVE) {
      process_return = process_deregistration_message(obj_msg);
    }
    else if (msg_type == DEVICE_ALIGN) {
      process_return = process_device_alignment_message(obj_msg);
    }
    else if (msg_type == KILL) {
      process_return = process_kill_message(obj_msg);
    }
    else if (msg_type == PING) {
      process_return = process_ping_message(obj_msg);
    }
    else {
      process_return = "-1";
    }

    return process_return;
  }
};

#endif

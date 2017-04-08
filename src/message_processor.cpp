#include "message_processor.h"

//----------------------------------------------------------------------------//
//---------------------------Internal Methods---------------------------------//
//----------------------------------------------------------------------------//

std::string MessageProcessor::create_uuid() {
  UuidContainer id_container;
  id_container = ugen->generate();
  if (!id_container.err.empty()) {
    uuid_logging->error(id_container.err);
  }
  return id_container.id;
}

void MessageProcessor::get_mutex_lock(std::string obj_key) {
  std::string key = "IVAN-" + obj_key;
  std::string val = "IVAN-" + config->get_nodeid();
  redis_locks->get_lock(key, val);
}

void MessageProcessor::release_mutex_lock(std::string obj_key) {
  std::string key = "IVAN-" + obj_key;
  std::string val = "IVAN-" + config->get_nodeid();
  redis_locks->release_lock(key, val);
}

//----------------------------------------------------------------------------//
//------------------------------Ping & Kill-----------------------------------//
//----------------------------------------------------------------------------//

//Ping the Crazy Ivan Instance
std::string MessageProcessor::process_ping_message(Scene *obj_msg) {
  processor_logging->debug("Ping Pong");
  return "";
}

//Kill the Crazy Ivan Instance
std::string MessageProcessor::process_kill_message(Scene *obj_msg) {
  return "";
}

//----------------------------------------------------------------------------//
//-----------------------------CRUD Support-----------------------------------//
//----------------------------------------------------------------------------//

//Create a new scene
std::string MessageProcessor::process_create_message(Scene *obj_msg) {
  if (obj_msg->num_scenes() > 0) {
    ResultsIteratorInterface *results = NULL;
    Neo4jQueryParameterInterface *name_param = NULL;
    Neo4jQueryParameterInterface *lat_param = NULL;
    Neo4jQueryParameterInterface *long_param = NULL;
    Neo4jQueryParameterInterface *key_param = NULL;
    ResultTreeInterface *tree = NULL;
    DbObjectInterface* obj = NULL;
    ret_val = "";
    double qlat;
    double qlong;
    processor_logging->info("Processing Scene Creation message");

    //Generate a new key
    UuidContainer id_container;
    id_container = ugen->generate();
    if (!id_container.err.empty()) {
      uuid_logging->error(id_container.err);
    }
    if (id_container.id.empty()) {
      processor_logging->error("Unknown error generating new key for scene");
      return "-1";
    }

    //Set up the Cypher Query for scene creation
    std::string scene_query = "CREATE (scn:Scene {key: {inp_key}";

    //Set up the query parameters for scene creation
    std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
    key_param = neo_factory->get_neo4j_query_parameter(id_container.id);
    processor_logging->info("Key:");
    processor_logging->info(id_container.id);
    scene_params.emplace("inp_key", key_param);
    std::string qname = obj_msg->get_scene(0).get_name();
    if ( !(qname.empty()) ) {
      name_param = neo_factory->get_neo4j_query_parameter(qname);
      processor_logging->debug("Name:");
      processor_logging->debug(qname);
      scene_params.emplace("inp_name", name_param);
      scene_query = scene_query + ", name: {inp_name}";
    }
    if ( !(obj_msg->get_scene(0).get_latitude() == -9999.0) ) {
      qlat = obj_msg->get_scene(0).get_latitude();
      lat_param = neo_factory->get_neo4j_query_parameter(qlat);
      processor_logging->debug("Latitude:");
      processor_logging->debug(std::to_string(qlat));
      scene_params.emplace("inp_lat", lat_param);
      scene_query = scene_query + ", latitude: {inp_lat}";
    }
    if ( !(obj_msg->get_scene(0).get_longitude() == -9999.0) ) {
      qlong = obj_msg->get_scene(0).get_longitude();
      long_param = neo_factory->get_neo4j_query_parameter(qlong);
      processor_logging->debug("Longitude:");
      processor_logging->debug(std::to_string(qlong));
      scene_params.emplace("inp_long", long_param);
      scene_query = scene_query + ", longitude: {inp_long}";
    }

    scene_query = scene_query + "}) RETURN scn";
    processor_logging->debug(scene_query);

    //Execute the query
    try {
      results = n->execute(scene_query, scene_params);
    }
    catch (std::exception& e) {
      processor_logging->error("Error running Query:");
      processor_logging->error(e.what());
      ret_val = "-1";
    }

    if (!results) {
      processor_logging->error("No results returned from create query");
      ret_val = "-1";
    }
    else {
      processor_logging->info("Scene Successfully added");
      ret_val = id_container.id;
      tree = results->next();
      if (tree) {
        obj = tree->get(0);
        if (obj) {
          if ( !(obj->is_node()) ) ret_val = "-2";
          delete obj;
        }
        delete tree;
      }
    }

    if (results) delete results;
    if (name_param) delete name_param;
    if (lat_param) delete lat_param;
    if (long_param) delete long_param;
    if (key_param) delete key_param;
    return ret_val;
  }
  ret_val = "-1";
  return ret_val;
}

//Update the details of a scene entry
std::string MessageProcessor::process_update_message(Scene *obj_msg) {

  //Retrieve a Redis Mutex Lock on the scene
  if (config->get_atomictransactions()) {
    get_mutex_lock(obj_msg->get_scene(0).get_key());
  }

  //Declare base variables
  ret_val = "";
  bool is_started = false;
  ResultsIteratorInterface *results = NULL;
  Neo4jQueryParameterInterface *name_param = NULL;
  Neo4jQueryParameterInterface *lat_param = NULL;
  Neo4jQueryParameterInterface *long_param = NULL;
  Neo4jQueryParameterInterface *key_param = NULL;
  processor_logging->debug("Processing Scene Update message");

  //Ensure that we have fields in the query
  if ((obj_msg->get_scene(0).get_name().empty() && obj_msg->get_scene(0).get_latitude() == -9999.0 && obj_msg->get_scene(0).get_longitude() == -9999.0) || obj_msg->get_scene(0).get_key().empty()) {
    processor_logging->error("No fields found in update message");
    ret_val = "-1";
  }
  else {
    //Set up the Cypher Query for scene update
    std::string scene_query = "MATCH (scn:Scene {key: {inp_key}}) SET ";
    if ( !(obj_msg->get_scene(0).get_name().empty()) ) {
      scene_query = scene_query + "scn.name = {inp_name}";
      is_started = true;
    }

    if ( !(obj_msg->get_scene(0).get_latitude() == -9999.0) ) {
      if (is_started) {
        scene_query = scene_query + ", ";
      }
      scene_query = scene_query + "scn.latitude = {inp_lat}";
      is_started = true;
    }

    if ( !(obj_msg->get_scene(0).get_longitude() == -9999.0) ) {
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
    std::string qkey = obj_msg->get_scene(0).get_key();
    key_param = neo_factory->get_neo4j_query_parameter(qkey);
    processor_logging->debug("Key:");
    processor_logging->debug(qkey);
    scene_params.emplace("inp_key", key_param);
    if ( !(obj_msg->get_scene(0).get_name().empty()) ) {
      std::string qname = obj_msg->get_scene(0).get_name();
      name_param = neo_factory->get_neo4j_query_parameter(qname);
      processor_logging->debug("Name:");
      processor_logging->debug(qname);
      scene_params.emplace("inp_name", name_param);
    }
    if ( !(obj_msg->get_scene(0).get_latitude() == -9999.0) ) {
      double qlat = obj_msg->get_scene(0).get_latitude();
      lat_param = neo_factory->get_neo4j_query_parameter(qlat);
      scene_params.emplace("inp_lat", lat_param);
    }
    if ( !(obj_msg->get_scene(0).get_longitude() == -9999.0) ) {
      double qlong = obj_msg->get_scene(0).get_longitude();
      long_param = neo_factory->get_neo4j_query_parameter(qlong);
      scene_params.emplace("inp_long", long_param);
    }

    //Execute the query
    ResultTreeInterface *tree = NULL;
    DbObjectInterface* obj = NULL;
    try {
      results = n->execute(scene_query, scene_params);
    }
    catch (std::exception& e) {
      if (obj) delete obj;
      if (tree) delete tree;
      processor_logging->error("Error running Query:");
      processor_logging->error(e.what());
      ret_val = "-1";
    }

    if (!results) {
      processor_logging->error("No results returned from update query");
      ret_val = "-1";
    }
    else {
      ret_val = qkey;
      tree = results->next();
      if (tree) {
        obj = tree->get(0);
        if (obj) {
          if ( !(obj->is_node()) ) ret_val = "-2";
          delete obj;
        }
        delete tree;
      }
    }
  }

  //Cleanup
  if (results) delete results;
  if (name_param) delete name_param;
  if (lat_param) delete lat_param;
  if (long_param) delete long_param;
  if (key_param) delete key_param;

  //Release the Redis Mutex Lock
  if (config->get_atomictransactions()) {
    release_mutex_lock(obj_msg->get_scene(0).get_key());
  }
  return ret_val;
}

//Query for scene data
std::string MessageProcessor::process_retrieve_message(Scene *obj_msg) {
  ResultsIteratorInterface *results = NULL;
  Neo4jQueryParameterInterface *name_param = NULL;
  Neo4jQueryParameterInterface *lat_param = NULL;
  Neo4jQueryParameterInterface *long_param = NULL;
  Neo4jQueryParameterInterface *key_param = NULL;
  Neo4jQueryParameterInterface *dist_param = NULL;
  processor_logging->debug("Processing Scene Retrieve message");
  bool is_started = false;
  ret_val = "";

  if (obj_msg->get_scene(0).get_name().empty() && obj_msg->get_scene(0).get_latitude() == -9999.0 && obj_msg->get_scene(0).get_longitude() == -9999.0 && obj_msg->get_scene(0).get_key().empty()) {
    processor_logging->error("No fields found in get message");
    ret_val = "-1";
  }
  else {
    //Set up the Cypher Query for scene retrieval
    std::string scene_query = "MATCH (scn:Scene";
    if ( !(obj_msg->get_scene(0).get_name().empty()) ) {

      scene_query = scene_query + " {name: {inp_name}";
      is_started = true;
    }

    if ( !(obj_msg->get_scene(0).get_key().empty()) ) {
      if (is_started) {
        scene_query = scene_query + ", ";
      }
      else {
        scene_query = scene_query + " {";
      }
      scene_query = scene_query + "key: {inp_key}";
      is_started = true;
    }

    if (is_started) {
      scene_query = scene_query + "}";
    }

    scene_query = scene_query + ")";

    if ( !(obj_msg->get_scene(0).get_latitude() == -9999.0 || obj_msg->get_scene(0).get_longitude() == -9999.0 || obj_msg->get_scene(0).get_distance() < 0.0 ) ) {
      //Query for distance
      //Assumes distance supplied is in meters
      //Haversine formula (https://en.wikipedia.org/wiki/Haversine_formula)
      std::string where_clause = "WHERE ( 12742000 * asin((((sin((scn.longitude -"
                                  " {inp_long}) / 2) ^ 2) + (cos({inp_long}) * "
                                  "cos(scn.longitude) * (sin((scn.latitude - "
                                  "{inp_lat}) / 2) ^ 2))) ^ (1/2)))) < {inp_distance}";
      scene_query = scene_query + where_clause;
    }

    scene_query = scene_query + " RETURN scn";

    processor_logging->debug("Executing Query:");
    processor_logging->debug(scene_query);

    //Set up the query parameters for scene retrieval
    std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
    if ( !(obj_msg->get_scene(0).get_key().empty()) ) {
      std::string qkey = obj_msg->get_scene(0).get_key();
      key_param = neo_factory->get_neo4j_query_parameter(qkey);
      processor_logging->debug("Key:");
      processor_logging->debug(qkey);
      scene_params.emplace("inp_key", key_param);
    }
    if ( !(obj_msg->get_scene(0).get_name().empty()) ) {
      std::string qname = obj_msg->get_scene(0).get_name();
      name_param = neo_factory->get_neo4j_query_parameter(qname);
      processor_logging->debug("Name:");
      processor_logging->debug(qname);
      scene_params.emplace("inp_name", name_param);
    }
    if ( !(obj_msg->get_scene(0).get_latitude() == -9999.0 || obj_msg->get_scene(0).get_longitude() == -9999.0 || obj_msg->get_scene(0).get_distance() < 0.0) ) {
      double qlat = obj_msg->get_scene(0).get_latitude();
      lat_param = neo_factory->get_neo4j_query_parameter(qlat);
      scene_params.emplace("inp_lat", lat_param);
      double qlong = obj_msg->get_scene(0).get_longitude();
      long_param = neo_factory->get_neo4j_query_parameter(qlong);
      scene_params.emplace("inp_long", long_param);
      double qdist = obj_msg->get_scene(0).get_distance();
      dist_param = neo_factory->get_neo4j_query_parameter(qdist);
      scene_params.emplace("inp_distance", dist_param);
    }

    //Execute the query
    try {
      results = n->execute(scene_query, scene_params);
      if (!results) {
        processor_logging->error("No results returned from update query");
        ret_val = "-1";
      }
      else {
        //Pull results and return
        Scene sc;
        sc.set_err_code(NO_ERROR);
        sc.set_msg_type(SCENE_GET);
        sc.set_transaction_id(obj_msg->get_transaction_id());
        ResultTreeInterface *tree = results->next();
        int num_results = 0;
        while (true) {

          SceneData data;

          //Get the first DB Object (Node)
          DbObjectInterface* obj = tree->get(0);
          processor_logging->debug("Query Result:");
          processor_logging->debug(obj->to_string());

          //Leave the loop if we don't have anything in this result tree
          if ( !(obj->is_node()) && !(obj->is_edge()) ) break;
          num_results=num_results+1;

          //Pull the node properties and assign them to the new
          //Scene object
          DbMapInterface* map = obj->properties();
          if (map->element_exists("key")) {
            data.set_key( map->get_string_element("key") );
          }
          if (map->element_exists("name")) {
            data.set_name( map->get_string_element("name") );
          }
          if (map->element_exists("latitude")) {
            data.set_latitude( map->get_float_element("latitude") );
          }
          if (map->element_exists("longitude")) {
            data.set_longitude( map->get_float_element("longitude") );
          }

          sc.add_scene(data);

          //Iterate to the next result
          if (tree) delete tree;
          if (obj) delete obj;
          if (map) delete map;
          tree = results->next();
        }
        if (num_results>0) {
          ret_val = sc.to_protobuf();
        }
        else {
          ret_val = "-2";
        }
      }
    }
    catch (std::exception& e) {
      processor_logging->error("Error running Query:");
      processor_logging->error(e.what());
      ret_val = "-1";
    }
  }
  if (results) delete results;
  if (name_param) delete name_param;
  if (lat_param) delete lat_param;
  if (long_param) delete long_param;
  if (key_param) delete key_param;
  if (dist_param) delete dist_param;
  return ret_val;
}

//Delete a scene
std::string MessageProcessor::process_delete_message(Scene *obj_msg) {
  ret_val = "";
  processor_logging->debug("Processing Scene Delete message");
  ResultsIteratorInterface *results = NULL;
  Neo4jQueryParameterInterface *key_param = NULL;

  if ( obj_msg->get_scene(0).get_key().empty() ) {
    processor_logging->error("No fields found in delete message");
    return "-1";
  }
  else {
    //Set up the Cypher Query for scene delete
    std::string scene_query = "MATCH (scn:Scene {key: {inp_key}}) DETACH DELETE scn RETURN scn";
    processor_logging->debug("Executing Delete Query");

    //Set up the query parameters for scene delete
    std::unordered_map<std::string, Neo4jQueryParameterInterface*> scene_params;
    std::string qkey = obj_msg->get_scene(0).get_key();
    key_param = neo_factory->get_neo4j_query_parameter(qkey);
    processor_logging->debug("Key:");
    processor_logging->debug(qkey);
    scene_params.emplace("inp_key", key_param);

    //Execute the query
    ResultTreeInterface *tree = NULL;
    DbObjectInterface* obj = NULL;
    try {
      results = n->execute(scene_query, scene_params);
    }
    catch (std::exception& e) {
      processor_logging->error("Error running Query:");
      processor_logging->error(e.what());
      ret_val = "-1";
    }
    if (!results) {
      processor_logging->error("No results returned from delete query");
      ret_val = "-1";
    }
    else {
      ret_val = qkey;
      tree = results->next();
      if (tree) {
        obj = tree->get(0);
        if (obj) {
          if ( !(obj->is_node()) ) ret_val = "-2";
          delete obj;
        }
        delete tree;
      }
    }
  }
  if (results) delete results;
  if (key_param) delete key_param;
  return ret_val;
}

//----------------------------------------------------------------------------//
//-----------------------Strings & Serialization------------------------------//

//Build a protocol buffer serialized string
std::string MessageProcessor::build_proto_response(int msg_type, int err_code, std::string err_msg, std::string tran_id, std::string scene_id) {
  //Set up the scene list
  Scene scn;
  scn.set_msg_type(msg_type);
  scn.set_err_code(err_code);
  scn.set_err_msg(err_msg);
  scn.set_transaction_id(tran_id);
  //Set up the scene
  SceneData data1;
  data1.set_key(scene_id);
  scn.add_scene(data1);
  //Return the serialized string
  return scn.to_protobuf();
}

//Build a protocol buffer serialized string
std::string MessageProcessor::build_proto_response(int msg_type, int err_code, std::string err_msg, std::string tran_id, std::string scene_id, std::string dev_id, Transform &t) {
  //Set up the scene list
  Scene scn;
  scn.set_msg_type(msg_type);
  scn.set_err_code(err_code);
  scn.set_err_msg(err_msg);
  scn.set_transaction_id(tran_id);
  //Set up the scene
  SceneData data1;
  data1.set_key(scene_id);
  //Set up the user device
  UserDevice ud1;
  ud1.set_key(dev_id);
  //Add the transformation and key data to the user device
  ud1.set_transform(&t);
  data1.add_device(ud1);
  scn.add_scene(data1);
  //Return the serialized string
  return scn.to_protobuf();
}

//----------------------------------------------------------------------------//
//--------------------------Device Registration-------------------------------//
//Register a device to a scene
//Predict a coordinate transform for the device
//and pass it back in the response
std::string MessageProcessor::process_registration_message(Scene *obj_msg) {

  processor_logging->debug("Processing Registration Message");
  //Is this the first device being registered to the scene
  bool is_first_device = false;
  //Does the scene exist in the DB?
  bool does_scene_exist = true;
  //Is the device already registered to the scene
  bool already_registered = false;
  //Is the device already registered to another scene?
  bool previously_registered = false;
  //Current error information
  int current_err_code = NO_ERROR;
  std::string current_err_msg = "";

  //Start by getting a mutex lock against the scene
  processor_logging->debug("Getting mutex lock");
  if (config->get_atomictransactions()) {
    try {
      get_mutex_lock(obj_msg->get_scene(0).get_key());
    }
    catch (std::exception& e) {
      processor_logging->error("Error getting mutex lock");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //Determine if the scene exists in the DB
  if (current_err_code == NO_ERROR) {
    try {
      does_scene_exist = qh->scene_exists(obj_msg->get_scene(0).get_key());
    }
    catch (std::exception& e) {
      processor_logging->error("Error checking for scene existence");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //Determine if this is the first device being registered to the scene
  if (current_err_code == NO_ERROR) {
    if ( !(does_scene_exist) ) {
      is_first_device = true;
      //Create the scene
      processor_logging->debug("Creating Scene in database");
      std::string create_resp = process_create_message(obj_msg);
      if (create_resp == "-1") {
        processor_logging->error("Scene not found and creation failed");
        current_err_code = PROCESSING_ERROR;
        current_err_msg = "Scene not found and creation failed";
      }
    }
    else {
      try {
        already_registered = qh->is_ud_registered(obj_msg->get_scene(0).get_key(),\
          obj_msg->get_scene(0).get_device(0).get_key());
      }
      catch (std::exception& e) {
        processor_logging->error("Error checking if User Device is already registered");
        processor_logging->error(e.what());
        current_err_code = PROCESSING_ERROR;
        current_err_msg = e.what();
      }
    }
  }

  //Find other scenes that the object is registered to
  Scene *registered_scenes = NULL;
  if (current_err_code == NO_ERROR) {
    try {
      registered_scenes = qh->get_registrations(obj_msg->get_scene(0).get_device(0).get_key());
    }
    catch (std::exception& e) {
      processor_logging->error("Error getting registrations");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }
  if (!registered_scenes) {
    previously_registered = false;
  }

  //The transform matrix calculated for the given device
  //If we are registering the first device or cannot find
  //any paths from the device's previous scene(s), then we will
  //leave this as the identity matrix
  Transform new_transform;

  //If we are not registering the first device, the scene exists,
  //the device is not already registered to the scene in question but
  //is registered to other scenes
  if ( !(is_first_device) && does_scene_exist && !(already_registered) && current_err_code == NO_ERROR && previously_registered ) {
    processor_logging->debug("Attempting to calculate UD Transform from Existing Registrations");
    //Iterate through the scenes the device is already registered to
    //Try to find a path from these scenes to the current one in order
    //to establish a transform
    for (int i = 0; i < registered_scenes->num_scenes(); i++) {

      try {
        SceneTransformResult st_result = qh->calculate_scene_scene_transform(\
          registered_scenes->get_scene(i).get_key(), obj_msg->get_scene(0).get_key());
        if (st_result.result_flag) {
          new_transform.add_transform(st_result.transform);
        }
      }
      catch (std::exception& e) {
        processor_logging->error("Error calculating Scene-Scene Transform");
        processor_logging->error(e.what());
        current_err_code = PROCESSING_ERROR;
        current_err_msg = e.what();
      }
    }
  }

  if ( !(already_registered) && current_err_code == NO_ERROR ) {
    //Create the registration link in the DB
    processor_logging->debug("Registering Device in the DB");
    try {
      qh->register_device_to_scene(obj_msg->get_scene(0).get_device(0).get_key(),\
        obj_msg->get_scene(0).get_key(), new_transform);
    }
    catch (std::exception& e) {
      processor_logging->error("Error Registering Device");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //If we are registering to another scene and that scene is new, then
  //we can put in place a coordinate system transformation equal to the device
  //transform from the old scene, as the transform from the new scene is the identity matrix
  if (previously_registered && is_first_device && current_err_code == NO_ERROR) {
    qh->process_UDUD_transformation(registered_scenes, obj_msg);
  }

  //Build a protocol buffer response
  processor_logging->debug("Creating Response message");
  proto_resp = build_proto_response(SCENE_ENTER, current_err_code,\
    current_err_msg, obj_msg->get_transaction_id(), obj_msg->get_scene(0).get_key());

  if (registered_scenes) {
    delete registered_scenes;
  }

  if (config->get_atomictransactions()) {
    release_mutex_lock(obj_msg->get_scene(0).get_key());
  }
  return proto_resp;
}

//Remove a device from a scene
//If we are removing the last user device from a scene,
//then remove the scene and try to move any paths that pass through the scene
std::string MessageProcessor::process_deregistration_message(Scene *obj_msg) {

  processor_logging->debug("Processing Deregistration Message");
  //Current error information
  int current_err_code = NO_ERROR;
  std::string current_err_msg = "";

  //Start by getting a mutex lock against the scene
  processor_logging->debug("Getting mutex lock");
  try {
    get_mutex_lock(obj_msg->get_scene(0).get_key());
  }
  catch (std::exception& e) {
    processor_logging->error("Error Getting Mutex Lock");
    processor_logging->error(e.what());
    current_err_code = PROCESSING_ERROR;
    current_err_msg = e.what();
  }

  //Remove the User Device
  if (current_err_code == NO_ERROR) {
    processor_logging->debug("Removing Device from Scene");
    try {
      qh->remove_device_from_scene(obj_msg->get_scene(0).get_device(0).get_key(),\
        obj_msg->get_scene(0).get_key());
    }
    catch (std::exception& e) {
      processor_logging->error("Error Removing Device From Scene");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }
  //Build a protocol buffer response
  processor_logging->debug("Creating Response message");
  proto_resp = build_proto_response(SCENE_LEAVE, current_err_code,\
    current_err_msg, obj_msg->get_transaction_id(), obj_msg->get_scene(0).get_key());

  release_mutex_lock(obj_msg->get_scene(0).get_key());
  return proto_resp;
}

//----------------------------------------------------------------------------//
//----------------------------Device Alignment--------------------------------//

//Align an objects transformation with a scene
//If the object is a member of other scenes:
//  -If a coordinate system transformation doesn't exist, then create one
//  -If a coordinate system transformation exists, then update it with a correction
std::string MessageProcessor::process_device_alignment_message(Scene *obj_msg) {
  processor_logging->debug("Processing Alignment Message");
  //Current error information
  int current_err_code = NO_ERROR;
  std::string current_err_msg = "";

  //Start by getting a mutex lock against the scene
  processor_logging->debug("Getting mutex lock");
  try {
    get_mutex_lock(obj_msg->get_scene(0).get_key());
  }
  catch (std::exception& e) {
    processor_logging->error("Error Getting Mutex Lock");
    processor_logging->error(e.what());
    current_err_code = PROCESSING_ERROR;
    current_err_msg = e.what();
  }

  //Update the transformation between the scene and user device
  if (current_err_code == NO_ERROR) {
    try {
      qh->update_device_registration( obj_msg->get_scene(0).get_device(0).get_key(),\
        obj_msg->get_scene(0).get_key(), *(obj_msg->get_scene(0).get_device(0).get_transform()) );
    }
    catch (std::exception& e) {
      processor_logging->error("Error Updating Device Registration");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //Find scenes that the object is registered to
  Scene *registered_scenes = NULL;
  if (current_err_code == NO_ERROR) {
    processor_logging->debug("Retrieving Scenes already registered to");
    try {
      registered_scenes = qh->get_registrations(obj_msg->get_scene(0).get_device(0).get_key());
    }
    catch (std::exception& e) {
      processor_logging->error("Error Retrieving Device Registrations");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //Iterate through the scenes the devices is registered with &
  //Correct/Create scene-scene transforms
  Transform new_transform;
  if (current_err_code == NO_ERROR) {
    processor_logging->debug("Updating Scene-Scene Transforms");
    try {
      qh->process_UDUD_transformation(registered_scenes, obj_msg);
    }
    catch (std::exception& e) {
      processor_logging->error("Error Correcting/Creating Scene-Scene Transforms");
      processor_logging->error(e.what());
      current_err_code = PROCESSING_ERROR;
      current_err_msg = e.what();
    }
  }

  //Build a protocol buffer response
  processor_logging->debug("Creating Response message");
  proto_resp = build_proto_response(DEVICE_ALIGN, current_err_code,\
    current_err_msg, obj_msg->get_transaction_id(), obj_msg->get_scene(0).get_key());

  if (registered_scenes) {
    delete registered_scenes;
  }

  release_mutex_lock(obj_msg->get_scene(0).get_key());
  return proto_resp;
}

#include "query_helper.h"

bool QueryHelper::scene_exists(std::string inp_key) {
  //Determine if the scene exists
  processor_logging->debug("Determine if the scene exists");
  ResultsIteratorInterface *results = NULL;
  //Create the query string
  std::string query_string =
    "MATCH (scn:Scene {key: {inp_key}})"
    " RETURN scn";

  //Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> query_params;
  Neo4jQueryParameterInterface* query_param = neo_factory->get_neo4j_query_parameter(inp_key);
  processor_logging->debug("Key:");
  processor_logging->debug(inp_key);
  query_params.emplace("inp_key", query_param);

  //Execute the query
  results = n->execute(query_string, query_params);
  if (!results) {
    processor_logging->debug("No Scenes found for the given key");
    return false;
  }
  else {
    delete results;
  }
  if (query_param) {delete query_param;}
  return true;
}

//----------------------------------------------------------------------------//
//------------------------Scene-Device Links----------------------------------//
//----------------------------------------------------------------------------//

//Determine if the given user device is registered to the given scene
bool QueryHelper::is_ud_registered(std::string inp_string, std::string inp_device) {
  processor_logging->debug("Checking the existing User Devices registered to the Scene");
  ResultsIteratorInterface *results = NULL;
  //Create the query string
  std::string q_string =
    "MATCH (scn:Scene {key: {inp_key}})-[tr:TRANSFORM]->(ud:UserDevice)"
    " RETURN ud";

  //Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;
  std::string qkey = inp_string;
  Neo4jQueryParameterInterface* key_param = neo_factory->get_neo4j_query_parameter(qkey);
  processor_logging->debug("Key:");
  processor_logging->debug(qkey);
  q_params.emplace("inp_key", key_param);

  //Execute the query
  bool ret_val = false;
  results = n->execute(q_string, q_params);
  if (!results) {
    processor_logging->debug("User Device not found registered to the given scene");
    return false;
    //If we are registering the first device, then we have a freshly created scene
  }
  else {
    //Check if the registering user device is already registered
    processor_logging->debug("User Devices detected, starting iterative checking to see if the current registration has already been processed");
    ResultTreeInterface *tree = results->next();
    while (tree) {

      //Get the first DB Object (Node)
      DbMapInterface* map = NULL;
      DbObjectInterface* obj = tree->get(0);
      processor_logging->debug("Query Result:");
      processor_logging->debug(obj->to_string());

      if ( !(obj->is_node()) ) break;

      //Pull the node properties and assign them to the new
      //Scene object
      map = obj->properties();
      std::string db_key = "";
      if (map->element_exists("key")) {
        db_key = map->get_string_element("key");
      }

      if (db_key == inp_device) {
        processor_logging->debug("Existing registration detected");
        ret_val = true;
      }

      //Cleanup
      if (tree) {
        delete tree;
      }
      if (obj) {
        delete obj;
      }
      if (map) {
        delete map;
      }

      //Iterate to the next result
      tree = results->next();
    }
  }
  if (results) {
    delete results;
  }
  if (key_param) {delete key_param;}
  return ret_val;
}

//Get scenes that this user device is registered to
//Collect the User Device and Transformation within the scenes returned
Scene* QueryHelper::get_registrations(std::string inp_device) {
  processor_logging->debug("Checking for other scenes the user device is registered to");
  ResultsIteratorInterface *results = NULL;
  //Create the return object
  Scene *sc = new Scene;
  //Create the query string
  std::string udq_string =
    "MATCH (scn:Scene)-[tr:TRANSFORM]->(ud:UserDevice {key: {inp_key}})"
    " RETURN scn, tr, ud";

  //Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> udq_params;
  std::string udqkey = inp_device;
  Neo4jQueryParameterInterface* udkey_param = neo_factory->get_neo4j_query_parameter(udqkey);
  processor_logging->debug("Key:");
  processor_logging->debug(udqkey);
  udq_params.emplace("inp_key", udkey_param);

  //Execute the query
  try {
    results = n->execute(udq_string, udq_params);
    if (!results) {
      processor_logging->debug("No Scenes found for the given device");
      return NULL;
    }
    else {
      //Iterate through the results
      //Build the scene list
      ResultTreeInterface *tree = results->next();
      while (tree) {

        SceneData new_data;

        //Get the first DB Object (Node)
        DbObjectInterface* obj = tree->get(0);
        if ( !(obj->is_node()) ) break;
        processor_logging->debug("Query Result:");
        processor_logging->debug(obj->to_string());

        //Pull the node properties and assign them to the new
        //Scene object
        DbMapInterface* map = obj->properties();
        if (map->element_exists("key")) {
          new_data.set_key( map->get_string_element("key") );
        }
        if (map->element_exists("name")) {
          new_data.set_name( map->get_string_element("name") );
        }
        if (map->element_exists("latitude")) {
          new_data.set_latitude( map->get_float_element("latitude") );
        }
        if (map->element_exists("longitude")) {
          new_data.set_longitude( map->get_float_element("longitude") );
        }

        //Get the transform and device info
        DbMapInterface *edge_props = NULL;
        DbObjectInterface *edge = tree->get(1);
        double translation_x = -999.0;
        double translation_y = -999.0;
        double translation_z = -999.0;
        double rotation_x = -999.0;
        double rotation_y = -999.0;
        double rotation_z = -999.0;
        if ( obj->is_edge() )  {
          edge_props = edge->properties();
          //Get the transform attributes
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

        DbObjectInterface *device = tree->get(2);
        if ( obj->is_node() ) {
          //Get the transform and device properties
          DbMapInterface *dev_props = device->properties();

          if (dev_props->element_exists("key")) {
            //Add the device related data to the scene
            UserDevice new_dev (dev_props->get_string_element("key"));
            new_dev.set_translation( 0, translation_x );
            new_dev.set_translation( 1, translation_y );
            new_dev.set_translation( 2, translation_z );
            new_dev.set_rotation(0, rotation_x);
            new_dev.set_rotation(1, rotation_y);
            new_dev.set_rotation(2, rotation_z);
            new_data.add_device(new_dev);
          }
        }

        sc->add_scene(new_data);

        if (map) {
          delete map;
        }
        if (obj) {
          delete obj;
        }
        if (tree) {
          delete tree;
        }
        tree = results->next();
      }
    }
  }
  catch (std::exception& e) {
    processor_logging->error("Error running Query:");
    processor_logging->error(udq_string);
    processor_logging->error(e.what());
    return NULL;
  }
  if (results) {
    delete results;
  }
  if (udkey_param) {delete udkey_param;}
  return sc;
}

//Create a registration link in the DB with the given device, scene, and transform matrix
void QueryHelper::register_device_to_scene(std::string device_id, std::string scene_id, Transform &transform) {
  processor_logging->debug("Creating Device Registration link");
  ResultsIteratorInterface *results = NULL;
  //Create the query string
  std::string udq_string =
    "MATCH (scn:Scene {key: {inp_key}})"
    " CREATE (scn)-[trans:TRANSFORM {translation_x: {loc_x}, translation_y: {loc_y}, translation_z: {loc_z},"
      " rotation_x: {rot_x}, rotation_y: {rot_y}, rotation_z: {rot_z}}]->(ud:UserDevice {key: {inp_ud_key}})"
    " RETURN scn, trans, ud";

  //Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  //Insert the scene key into the query list
  Neo4jQueryParameterInterface* skey_param = neo_factory->get_neo4j_query_parameter(scene_id);
  processor_logging->debug("Scene Key:");
  processor_logging->debug(scene_id);
  q_params.emplace("inp_key", skey_param);

  //Insert the device key into the query list
  Neo4jQueryParameterInterface* udkey_param = neo_factory->get_neo4j_query_parameter(device_id);
  processor_logging->debug("Device Key:");
  processor_logging->debug(device_id);
  q_params.emplace("inp_ud_key", udkey_param);

  //Insert translation
  Neo4jQueryParameterInterface* locx_param = neo_factory->get_neo4j_query_parameter(transform.translation(0));
  Neo4jQueryParameterInterface* locy_param = neo_factory->get_neo4j_query_parameter(transform.translation(1));
  Neo4jQueryParameterInterface* locz_param = neo_factory->get_neo4j_query_parameter(transform.translation(2));
  q_params.emplace("loc_x", locx_param);
  q_params.emplace("loc_y", locy_param);
  q_params.emplace("loc_z", locz_param);

  //Insert rotation
  Neo4jQueryParameterInterface* rotx_param = neo_factory->get_neo4j_query_parameter(transform.rotation(0));
  Neo4jQueryParameterInterface* roty_param = neo_factory->get_neo4j_query_parameter(transform.rotation(1));
  Neo4jQueryParameterInterface* rotz_param = neo_factory->get_neo4j_query_parameter(transform.rotation(2));
  q_params.emplace("rot_x", rotx_param);
  q_params.emplace("rot_y", roty_param);
  q_params.emplace("rot_z", rotz_param);

  //Execute the query
  results = n->execute(udq_string, q_params);
  if (!results) {
    processor_logging->error("No Links created");
  }
  else {
    delete results;
  }
  if (skey_param) {delete skey_param;}
  if (udkey_param) {delete udkey_param;}
  if (locx_param) {delete locx_param;}
  if (locy_param) {delete locy_param;}
  if (locz_param) {delete locz_param;}
  if (rotx_param) {delete rotx_param;}
  if (roty_param) {delete roty_param;}
  if (rotz_param) {delete rotz_param;}
}

//Remove a registration link in the DB between the specified device and scene
void QueryHelper::remove_device_from_scene(std::string device_id, std::string scene_id) {
  //Set up the Cypher Query for device deletion
  ResultsIteratorInterface *results = NULL;
  std::string query_string =
    "MATCH (scn:Scene {key: {inp_key}})-[trans:TRANSFORM]->(ud:UserDevice {key: {inp_ud_key}})"
    " DETACH DELETE ud RETURN scn";
  processor_logging->debug("Executing Delete Query");

  //Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  //Insert the scene key into the query list
  Neo4jQueryParameterInterface* skey_param = neo_factory->get_neo4j_query_parameter(scene_id);
  processor_logging->debug("Scene Key:");
  processor_logging->debug(scene_id);
  q_params.emplace("inp_key", skey_param);

  //Insert the device key into the query list
  Neo4jQueryParameterInterface* udkey_param = neo_factory->get_neo4j_query_parameter(device_id);
  processor_logging->debug("Device Key:");
  processor_logging->debug(device_id);
  q_params.emplace("inp_ud_key", udkey_param);

  //Execute the query
  results = n->execute(query_string, q_params);
  if (!results) {
    processor_logging->error("No Links destroyed");
  }
  else {delete results;}
  if (skey_param) {delete skey_param;}
  if (udkey_param) {delete udkey_param;}
}

//Update the device registration
void QueryHelper::update_device_registration(std::string dev_id, std::string scene_id, Transform &transform) {
  processor_logging->debug("Updating Device Registration link");
  ResultsIteratorInterface *results = NULL;
  ResultTreeInterface *tree = NULL;
  DbObjectInterface* obj = NULL;
  //Create the query string
  std::string udq_string =
    "MATCH (scn:Scene {key: {inp_key}})-[trans:TRANSFORM]->(ud:UserDevice {key: {inp_ud_key}})"
    " SET trans.translation_x = {loc_x}, trans.translation_y = {loc_y}, trans.translation_z = {loc_z}, "
      "trans.rotation_x = {rot_x}, trans.rotation_y = {rot_y}, trans.rotation_z = {rot_z}"
    " RETURN scn, trans, ud";

  //Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  //Insert the scene key into the query list
  Neo4jQueryParameterInterface* skey_param = neo_factory->get_neo4j_query_parameter(scene_id);
  processor_logging->debug("Scene Key:");
  processor_logging->debug(scene_id);
  q_params.emplace("inp_key", skey_param);

  //Insert the device key into the query list
  Neo4jQueryParameterInterface* udkey_param = neo_factory->get_neo4j_query_parameter(dev_id);
  processor_logging->debug("Device Key:");
  processor_logging->debug(dev_id);
  q_params.emplace("inp_ud_key", udkey_param);

  //Insert translation
  Neo4jQueryParameterInterface* locx_param = neo_factory->get_neo4j_query_parameter(transform.translation(0));
  Neo4jQueryParameterInterface* locy_param = neo_factory->get_neo4j_query_parameter(transform.translation(1));
  Neo4jQueryParameterInterface* locz_param = neo_factory->get_neo4j_query_parameter(transform.translation(2));
  q_params.emplace("loc_x", locx_param);
  q_params.emplace("loc_y", locy_param);
  q_params.emplace("loc_z", locz_param);

  //Insert rotation
  Neo4jQueryParameterInterface* rotx_param = neo_factory->get_neo4j_query_parameter(transform.rotation(0));
  Neo4jQueryParameterInterface* roty_param = neo_factory->get_neo4j_query_parameter(transform.rotation(1));
  Neo4jQueryParameterInterface* rotz_param = neo_factory->get_neo4j_query_parameter(transform.rotation(2));
  q_params.emplace("rot_x", rotx_param);
  q_params.emplace("rot_y", roty_param);
  q_params.emplace("rot_z", rotz_param);

  //Execute the query
  results = n->execute(udq_string, q_params);
  if (!results) {
    processor_logging->error("No Links created");
  }
  else {
    tree = results->next();
    obj = tree->get(0);
    if ( !(obj->is_node()) ) {
      processor_logging->debug("Query Returned no values");
      std::string exc_str = "Query Returned no values: ";
      exc_str = exc_str + udq_string;
      throw QueryException(exc_str);
    }
    delete results;
  }
  if (tree) {delete tree;}
  if (obj) {delete obj;}
  if (skey_param) {delete skey_param;}
  if (udkey_param) {delete udkey_param;}
  if (locx_param) {delete locx_param;}
  if (locy_param) {delete locy_param;}
  if (locz_param) {delete locz_param;}
  if (rotx_param) {delete rotx_param;}
  if (roty_param) {delete roty_param;}
  if (rotz_param) {delete rotz_param;}
}

//----------------------------------------------------------------------------//
//-------------------------Scene-Scene Links----------------------------------//
//----------------------------------------------------------------------------//

//Get the link between two scenes
//Return:
//--0: If no link is found
//--1: If a forward link is found
//--2: If a backward link is found
int QueryHelper::get_scene_link(std::string scene1_key, std::string scene2_key) {
  //Query the DB for an existing connection between the two scenes
  processor_logging->debug("Querying DB for existing connections between scenes");
  ResultsIteratorInterface *results = NULL;
  ResultTreeInterface *tree = NULL;
  DbObjectInterface* obj = NULL;
  DbMapInterface* map = NULL;
  //Create the query string
  std::string query_string =
    "MATCH (scn:Scene)-[trans:TRANSFORM]->(scn2:Scene)"
    " WHERE (scn.key = {inp_key1} AND scn2.key = {inp_key2})"
    " OR (scn.key = {inp_key2} AND scn2.key = {inp_key1})"
    " RETURN trans";

  //Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  //Insert the first scene key into the query list
  Neo4jQueryParameterInterface* skey1_param = neo_factory->get_neo4j_query_parameter(scene1_key);
  processor_logging->debug("Scene 1 Key:");
  processor_logging->debug(scene1_key);
  q_params.emplace("inp_key1", skey1_param);

  //Insert the second scene key into the query list
  Neo4jQueryParameterInterface* skey2_param = neo_factory->get_neo4j_query_parameter(scene2_key);
  processor_logging->debug("Scene 2 Key:");
  processor_logging->debug(scene2_key);
  q_params.emplace("inp_key2", skey2_param);

  //Execute the query
  results = n->execute(query_string, q_params);
  if (results) {
    //Find if the first result is forward or backward
    tree = results->next();
    obj = tree->get(0);
    if ( !(obj->is_edge()) ) {
      processor_logging->debug("Non-Edge value returned from query");
    }
    else {
      map = obj->properties();
      if (map->element_exists("key")) {
        if ( scene1_key == map->get_string_element("key") ) {
          return 1;
        }
        else if ( scene2_key == map->get_string_element("key") ) {
          return 2;
        }
      }
    }
  }

  if (skey1_param) {delete skey1_param;}
  if (skey2_param) {delete skey2_param;}
  if (tree) {delete tree;}
  if (obj) {delete obj;}
  if (map) {delete map;}

  return 0;
}

//Create the scene-scene link
void QueryHelper::create_scene_link(std::string s1_key, std::string s2_key, Transform new_trans) {
  processor_logging->debug("Creating Device Registration link");
  ResultsIteratorInterface *results = NULL;
  //Create the query string
  std::string udq_string =
    "MATCH (scn:Scene {key: {inp_key1}}), (scn2:Scene {key: {inp_key2}}) "
    "CREATE (scn)-[trans:TRANSFORM {translation_x: {loc_x}, translation_y: {loc_y}, translation_z: {loc_z}, "
      "rotation_x: {rot_x}, rotation_y: {rot_y}, rotation_z: {rot_z}}]->(scn2) "
    "RETURN scn, trans, scn2";

  //Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  //Insert the scene key into the query list
  Neo4jQueryParameterInterface* s1key_param = neo_factory->get_neo4j_query_parameter(s1_key);
  processor_logging->debug("Scene 1 Key:");
  processor_logging->debug(s1_key);
  q_params.emplace("inp_key1", s1key_param);

  //Insert the device key into the query list
  Neo4jQueryParameterInterface* s2key_param = neo_factory->get_neo4j_query_parameter(s2_key);
  processor_logging->debug("Scene 2 Key:");
  processor_logging->debug(s2_key);
  q_params.emplace("inp_key2", s2key_param);

  //Insert translation
  Neo4jQueryParameterInterface* locx_param = neo_factory->get_neo4j_query_parameter(new_trans.translation(0));
  Neo4jQueryParameterInterface* locy_param = neo_factory->get_neo4j_query_parameter(new_trans.translation(1));
  Neo4jQueryParameterInterface* locz_param = neo_factory->get_neo4j_query_parameter(new_trans.translation(2));
  q_params.emplace("loc_x", locx_param);
  q_params.emplace("loc_y", locy_param);
  q_params.emplace("loc_z", locz_param);

  //Insert rotation
  Neo4jQueryParameterInterface* rotx_param = neo_factory->get_neo4j_query_parameter(new_trans.rotation(0));
  Neo4jQueryParameterInterface* roty_param = neo_factory->get_neo4j_query_parameter(new_trans.rotation(1));
  Neo4jQueryParameterInterface* rotz_param = neo_factory->get_neo4j_query_parameter(new_trans.rotation(2));
  q_params.emplace("rot_x", rotx_param);
  q_params.emplace("rot_y", roty_param);
  q_params.emplace("rot_z", rotz_param);

  //Execute the query
  ResultTreeInterface *tree = NULL;
  DbObjectInterface* obj = NULL;
  results = n->execute(udq_string, q_params);
  if (!results) {
    processor_logging->error("No Links created");
  }
  else {
    tree = results->next();
    obj = tree->get(0);
    if ( !(obj->is_node()) ) {
      processor_logging->debug("Query Returned no values");
      std::string exc_str = "Query Returned no values: ";
      exc_str = exc_str + udq_string;
      throw QueryException(exc_str);
    }
    delete results;
  }
  if (tree) {delete tree;}
  if (obj) {delete obj;}
  if (s1key_param) {delete s1key_param;}
  if (s2key_param) {delete s2key_param;}
  if (locx_param) {delete locx_param;}
  if (locy_param) {delete locy_param;}
  if (locz_param) {delete locz_param;}
  if (rotx_param) {delete rotx_param;}
  if (roty_param) {delete roty_param;}
  if (rotz_param) {delete rotz_param;}
}

//TO-DO: Update the scene-scene link
void QueryHelper::update_scene_link(std::string s1_key, std::string s2_key, Transform new_trans) {
  processor_logging->debug("Updating Scene link");
  ResultsIteratorInterface *results = NULL;
  ResultTreeInterface *tree = NULL;
  DbObjectInterface* obj = NULL;
  //Create the query string
  std::string udq_string =
    "MATCH (scn:Scene {key: {inp_key1}})-[trans:TRANSFORM]->(scn2:Scene {key: {inp_key2}})"
    " SET trans.translation_x = {loc_x}, trans.translation_y = {loc_y}, trans.translation_z = {loc_z},"
    " trans.rotation_x: {rot_x}, trans.rotation_y: {rot_y}, trans.rotation_z: {rot_z}"
    " RETURN scn, trans, scn2";

  //Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> q_params;

  //Insert the scene key into the query list
  Neo4jQueryParameterInterface* s1key_param = neo_factory->get_neo4j_query_parameter(s1_key);
  processor_logging->debug("Scene 1 Key:");
  processor_logging->debug(s1_key);
  q_params.emplace("inp_key1", s1key_param);

  //Insert the device key into the query list
  Neo4jQueryParameterInterface* s2key_param = neo_factory->get_neo4j_query_parameter(s2_key);
  processor_logging->debug("Scene 2 Key:");
  processor_logging->debug(s2_key);
  q_params.emplace("inp_key2", s2key_param);

  //Insert translation
  Neo4jQueryParameterInterface* locx_param = neo_factory->get_neo4j_query_parameter(new_trans.translation(0));
  Neo4jQueryParameterInterface* locy_param = neo_factory->get_neo4j_query_parameter(new_trans.translation(1));
  Neo4jQueryParameterInterface* locz_param = neo_factory->get_neo4j_query_parameter(new_trans.translation(2));
  q_params.emplace("loc_x", locx_param);
  q_params.emplace("loc_y", locy_param);
  q_params.emplace("loc_z", locz_param);

  //Insert rotation
  Neo4jQueryParameterInterface* rotx_param = neo_factory->get_neo4j_query_parameter(new_trans.rotation(0));
  Neo4jQueryParameterInterface* roty_param = neo_factory->get_neo4j_query_parameter(new_trans.rotation(1));
  Neo4jQueryParameterInterface* rotz_param = neo_factory->get_neo4j_query_parameter(new_trans.rotation(2));
  q_params.emplace("rot_x", rotx_param);
  q_params.emplace("rot_y", roty_param);
  q_params.emplace("rot_z", rotz_param);

  //Execute the query
  results = n->execute(udq_string, q_params);
  if (!results) {
    processor_logging->error("No Links created");
  }
  else {
    tree = results->next();
    obj = tree->get(0);
    if ( !(obj->is_node()) ) {
      processor_logging->debug("Query Returned no values");
      std::string exc_str = "Query Returned no values: ";
      exc_str = exc_str + udq_string;
      throw QueryException(exc_str);
    }
    delete results;
  }
  if (tree) {delete tree;}
  if (obj) {delete obj;}
  if (s1key_param) {delete s1key_param;}
  if (s2key_param) {delete s2key_param;}
  if (locx_param) {delete locx_param;}
  if (locy_param) {delete locy_param;}
  if (locz_param) {delete locz_param;}
  if (rotx_param) {delete rotx_param;}
  if (roty_param) {delete roty_param;}
  if (rotz_param) {delete rotz_param;}
}

//----------------------------------------------------------------------------//
//------------------------------Algorithms------------------------------------//

//Use a Device registered to multiple scenes to create Scene-Scene
//Transform Links in the DB
//As an input, we expect a set of Scenes and User Device pairs
//which are then processed to generate the Scene-Scene links
void QueryHelper::process_UDUD_transformation(Scene *registered_scenes, Scene *obj_msg) {

  processor_logging->debug("Processing Scene-Scene Links");

  //Get the number of scenes
  int num_scenes = registered_scenes->num_scenes();
  int results = -1;
  std::string scene2_key = obj_msg->get_scene(0).get_key();

  //Iterate through pairs of scenes registered to
  for (int i=0;i<num_scenes;i++) {

    //Get the scene ID's
    std::string scene1_key = registered_scenes->get_scene(i).get_key();

    if (scene1_key != scene2_key) {

      //Get the user device transforms for each scene, calculate the correct transform
      processor_logging->debug("Calculating Transform");
      Transform new_trans;
      for (int k=0;k<3;k++) {
        //translation
        double new_translation = 0.0;
        if (obj_msg->get_scene(0).get_device(0).get_transform()->has_translation()) {
          new_translation = new_translation - obj_msg->get_scene(0).get_device(0).get_transform()->translation(k);
        }
        if (registered_scenes->get_scene(i).get_device(0).get_transform()->has_translation()) {
          new_translation = new_translation + registered_scenes->get_scene(i).get_device(0).get_transform()->translation(k);
        }
        new_trans.translate( k, new_translation );

        //rotation
        double new_rotation = 0.0;
        if (obj_msg->get_scene(0).get_device(0).get_transform()->has_rotation()) {
          new_rotation = new_rotation - obj_msg->get_scene(0).get_device(0).get_transform()->rotation(k);
        }
        if (registered_scenes->get_scene(i).get_device(0).get_transform()->has_rotation()) {
          new_rotation = new_rotation + registered_scenes->get_scene(i).get_device(0).get_transform()->rotation(k);
        }
        new_trans.rotate(k, new_rotation );
      }

      //Get any existing scene links
      processor_logging->debug("Retrieving existing scene links");
      results = get_scene_link(scene1_key, scene2_key);
      if (results == 0) {
        //No links found, create new one
        create_scene_link(scene1_key, scene2_key, new_trans);
      }
      else if (results == 1){
        //forward link found, update with correction
        update_scene_link(scene1_key, scene2_key, new_trans);
      }
      else if (results == 2) {
        //backward link found, update with correction
        new_trans.invert();
        update_scene_link(scene2_key, scene1_key, new_trans);
      }
    }
  }
}

//Try to find a path from one scene to the next and calculate
//the resulting transform
SceneTransformResult QueryHelper::calculate_scene_scene_transform(std::string scene_id1, std::string scene_id2) {
  processor_logging->debug("Checking for existing paths between scenes in the database");

  Transform new_tran;
  ResultsIteratorInterface *results = NULL;
  ResultTreeInterface *tree = NULL;
  DbObjectInterface* obj = NULL;
  DbObjectInterface* path_obj = NULL;
  DbMapInterface *map = NULL;
  DbListInterface *tran_list = NULL;
  DbListInterface *rot_list = NULL;

  //Find the shortest path

  //The query string to find the path
  std::string path_q_string = "MATCH (base:Scene {key: {inp_key_start}}), (next:Scene {key: {inp_key_end}}), p = shortestPath((base)-[r*]-(next)) WHERE ALL (x IN nodes(p) WHERE (x:Scene)) RETURN p";

  //Set up the query parameters for query
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> path_q_params;
  Neo4jQueryParameterInterface* pkey1_param = neo_factory->get_neo4j_query_parameter(scene_id1);
  processor_logging->debug("Start Key:");
  processor_logging->debug(scene_id1);
  path_q_params.emplace("inp_key_start", pkey1_param);

  Neo4jQueryParameterInterface* pkey2_param = neo_factory->get_neo4j_query_parameter(scene_id2);
  processor_logging->debug("End Key:");
  processor_logging->debug(scene_id2);
  path_q_params.emplace("inp_key_start", pkey2_param);

  //Execute the query
  results = n->execute(path_q_string, path_q_params);
  if (!results) {
    processor_logging->error("No Path between scenes found");
  }
  else {
    //If we have a path, then calculate the correct transform for the user device

    //Get the path object from the query return
    tree = results->next();
    obj = tree->get(0);

    //Determine if we have a path in the return value
    if (obj->is_path()) {
      processor_logging->debug("Path object detected in return values from Database");

      //Iterate through the path
      int path_size = obj->size();
      for (int i=0;i<path_size;i++) {

        //Retrieve the path element
        path_obj = obj->get_path_element(i);

        //Are we dealing with an edge?
        if (path_obj->is_edge()) {

          //Transform values
          double trnx = 0.0;
          double trny = 0.0;
          double trnz = 0.0;
          double rotx = 0.0;
          double roty = 0.0;
          double rotz = 0.0;

          //Get the property values
          map = path_obj->properties();
          if (map->element_exists("translation_x")) {
            trnx = map->get_float_element("translation_x");
          }
          if (map->element_exists("translation_y")) {
            trny = map->get_float_element("translation_y");
          }
          if (map->element_exists("translation_z")) {
            trnz = map->get_float_element("translation_z");
          }
          if (map->element_exists("rotation_x")) {
            rotx = map->get_float_element("rotation_x");
          }
          if (map->element_exists("rotation_y")) {
            roty = map->get_float_element("rotation_y");
          }
          if (map->element_exists("rotation_z")) {
            rotz = map->get_float_element("rotation_z");
          }

          //Is our edge backward?
          if ( !(path_obj->forward()) ) {
            trnx = (-1) * trnx;
            trny = (-1) * trny;
            trnz = (-1) * trnz;
            rotx = (-1) * rotx;
            roty = (-1) * roty;
            rotz = (-1) * rotz;
          }

          //Update the new transformation with the edge
          new_tran.translate(0, trnx);
          new_tran.translate(1, trny);
          new_tran.translate(2, trnz);
          new_tran.rotate(0, rotx);
          new_tran.rotate(1, roty);
          new_tran.rotate(2, rotz);

        }

        //Cleanup at the end of the loop
        if (path_obj) {
          delete path_obj;
          path_obj = NULL;
        }
        if (map) {
          delete map;
          map = NULL;
        }
        if (tran_list) {
          delete tran_list;
          tran_list = NULL;
        }
        if (rot_list) {
          delete rot_list;
          rot_list = NULL;
        }
      }
    }
    delete results;
  }
  if (tree) {
    delete tree;
  }
  if (obj) {
    delete obj;
  }
  if (path_obj) {
    delete path_obj;
  }
  if (map) {
    delete map;
  }
  if (tran_list) {
    delete tran_list;
  }
  if (rot_list) {
    delete rot_list;
  }
  SceneTransformResult str;
  str.transform = new_tran;
  str.result_flag = true;
  return str;
}

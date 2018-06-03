#include <string>

#include "neocpp/connection/interface/neo4j_interface.h"
#include "neocpp/connection/impl/libneo4j_factory.h"

#include "include/base_query_helper.h"

#include "catch.hpp"

TEST_CASE( "Test Scene CRUD Query Generation", "[unit]" ) {

  Neocpp::LibNeo4jFactory neo_factory;
  // Setup a transform to test with
  TransformFactory tfactory;
  TransformInterface *trans = tfactory.build_transform();
  TransformInterface *scene_trans = tfactory.build_transform();

  trans->translate(0, 1.0);
  trans->translate(1, 2.0);
  trans->translate(2, 3.0);
  trans->rotate(0, 45.0);
  trans->rotate(1, 90.0);
  trans->rotate(2, 135.0);

  scene_trans->translate(0, 1.0);
  scene_trans->translate(1, 1.0);
  scene_trans->translate(2, 1.0);
  scene_trans->rotate(0, 10.0);
  scene_trans->rotate(1, 20.0);
  scene_trans->rotate(2, 30.0);

  // Setup a device to test with
  UserDeviceFactory udfactory;
  std::string udk = "testing";
  UserDeviceInterface *ud = udfactory.build_device(udk, trans);

  // Basic Tests
  SceneFactory sfactory;
  SceneInterface *scene = sfactory.build_scene();

  std::string sk = "12345";
  std::string sname = "Test Name";
  std::string asset1 = "First Asset";
  std::string asset2 = "Second Asset";
  std::string tag1 = "First Tag";
  std::string tag2 = "Second Tag";
  std::string region = "USA";
  scene->set_key(sk);
  scene->set_name(sname);
  scene->add_asset(asset1);
  scene->add_asset(asset2);
  scene->add_tag(tag1);
  scene->add_tag(tag2);
  scene->set_region(region);
  scene->set_latitude(100.0);
  scene->set_longitude(120.0);
  scene->set_distance(20.0);

  scene->add_device(ud);
  scene->set_transform(scene_trans);

  scene->print();

  // Build a query helper.  We pass in null for the neo4j interface and config
  // manager because we aren't going to call any methods that need these
  // interfaces.
  BaseQueryHelper qh(NULL, &neo_factory, NULL);

  // Test the query builder methods from the Base Query Helper
  std::string query_string;

  // Build a create query
  qh.generate_scene_crud_query(sk, CREATE_QUERY_TYPE, APPEND, scene, query_string);
  // Validate the query
  main_logging->debug(query_string);
  REQUIRE(query_string == "CREATE (scn:Scene {key: {inp_key}, active: {inp_active}, name: {inp_name}, latitude: {inp_lat}, longitude: {inp_long}, region: {inp_region}, assets: {inp_assets}, tags: {inp_tags}}) RETURN scn");

  query_string.clear();

  // Build a get query (by key)
  qh.generate_scene_crud_query(sk, GET_QUERY_TYPE, APPEND, NULL, query_string);
  // Validate the query
  main_logging->debug(query_string);
  REQUIRE(query_string == "MATCH (scn:Scene {key: {inp_key}}) RETURN scn");

  query_string.clear();

  // Build an update query
  qh.generate_scene_crud_query(sk, UPDATE_QUERY_TYPE, APPEND, scene, query_string);
  // Validate the query
  main_logging->debug(query_string);
  REQUIRE(query_string == "MATCH (scn:Scene {key: {inp_key}}) SET scn.active = {inp_active}, scn.name = {inp_name}, scn.latitude = {inp_lat}, scn.longitude = {inp_long}, scn.region = {inp_region}, scn.assets = coalesce(scn.assets, []) + {inp_assets}, scn.tags = coalesce(scn.tags, []) + {inp_tags} RETURN scn");

  query_string.clear();

  // Build a get query
  std::string blank_key = "";
  qh.generate_scene_crud_query(blank_key, GET_QUERY_TYPE, APPEND, scene, query_string);
  // Validate the query
  main_logging->debug(query_string);
  REQUIRE(query_string == "MATCH (scn:Scene {name: {inp_name}, region: {inp_region}}) WHERE {inp_tag} in scn.tags AND ( 12742000 * asin(sqrt(haversin(radians({inp_lat} - scn.latitude)) + cos(radians({inp_lat})) * cos(radians(scn.latitude)) * haversin(radians(scn.longitude - {inp_long}))))) < {inp_distance} RETURN scn");

  query_string.clear();

  // Build an update query
  qh.generate_scene_crud_query(sk, DELETE_QUERY_TYPE, APPEND, NULL, query_string);
  // Validate the query
  main_logging->debug(query_string);
  REQUIRE(query_string == "MATCH (scn:Scene {key: {inp_key}}) DETACH DELETE scn RETURN scn");

  // Build & validate lists of query parameters
  std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
    scene_params;
  qh.generate_scene_query_parameters(sk, CREATE_QUERY_TYPE, scene, scene_params);
  REQUIRE(scene_params.size() == 8);

  std::unordered_map<std::string, Neo4jQueryParameterInterface*> \
    scene_params2;
  qh.generate_scene_query_parameters(sk, CREATE_QUERY_TYPE, NULL, scene_params2);
  REQUIRE(scene_params2.size() == 1);

  for (std::pair<std::string, Neo4jQueryParameterInterface*> element : scene_params) {
    delete element.second;
  }

  for (std::pair<std::string, Neo4jQueryParameterInterface*> element2 : scene_params2) {
    delete element2.second;
  }

  delete scene;
}

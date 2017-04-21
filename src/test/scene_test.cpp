#include "../scene.h"
#include "../ivan_log.h"
#include <iostream>
#include <Eigen/Dense>
#include <assert.h>
#include <string>

#include "aossl/logging/include/logging_interface.h"
#include "aossl/logging/include/factory_logging.h"

int main() {

  LoggingComponentFactory *factory = new LoggingComponentFactory;

  std::string initFileName = "log4cpp.properties";
  logging = factory->get_logging_interface(initFileName);

  start_logging_submodules();

  //Basic tests
  Scene scene1;
  SceneData *data1 = new SceneData;

  //Basic properties
  scene1.set_msg_type(1);
  scene1.set_err_code(10);
  scene1.set_err_msg("Testing");
  scene1.set_transaction_id("ghijklmno");

  data1->set_name("TestName");
  data1->set_key("abcdef");
  data1->set_latitude(137.0034);
  data1->set_longitude(89.443);

  //List properties
  UserDevice *ud1 = new UserDevice ("xyzabcdef1");
  UserDevice *ud2 = new UserDevice ("xyzabcdef2");
  data1->add_device(ud1);
  data1->add_device(ud2);

  scene1.add_scene(data1);

  assert( scene1.get_msg_type() == 1 );
  assert( scene1.get_err_code() == 10 );
  assert( scene1.get_err() == "Testing" );
  assert( scene1.get_transaction_id() == "ghijklmno" );

  assert( scene1.get_scene(0)->get_name() == "TestName" );
  assert( scene1.get_scene(0)->get_key() == "abcdef" );
  assert( scene1.get_scene(0)->get_latitude() == 137.0034 );
  assert( scene1.get_scene(0)->get_longitude() == 89.443 );

  assert( scene1.get_scene(0)->num_devices() == 2 );
  assert( scene1.get_scene(0)->get_device(0)->get_key() == "xyzabcdef1" );
  assert( scene1.get_scene(0)->get_device(1)->get_key() == "xyzabcdef2" );

  //Transform tests

  //JSON tests
  rapidjson::Document d;

  std::string json_string = scene1.to_json();
  const char * json_cstr = json_string.c_str();
  std::cout << json_cstr << std::endl;

  d.Parse(json_cstr);
  Scene jsonScene (d);
  jsonScene.print();

  assert( jsonScene.get_msg_type() == 1 );
  assert( jsonScene.get_err_code() == 10 );
  assert( jsonScene.get_transaction_id() == "ghijklmno" );
  assert( jsonScene.get_err() == "Testing" );

  assert( jsonScene.get_scene(0)->get_key() == "abcdef" );
  assert( jsonScene.get_scene(0)->get_latitude() == 137.0034 );
  assert( jsonScene.get_scene(0)->get_longitude() == 89.443 );
  assert( jsonScene.get_scene(0)->num_devices() == 2 );
  assert( jsonScene.get_scene(0)->get_device(0)->get_key() == "xyzabcdef1" );
  assert( jsonScene.get_scene(0)->get_device(1)->get_key() == "xyzabcdef2" );

  //Protocol Buffer tests
  std::string proto_string = scene1.to_protobuf();
  protoScene::SceneList new_proto;
  new_proto.ParseFromString(proto_string);
  Scene scene3 (new_proto);

  assert( scene3.get_msg_type() == 1 );
  assert( scene3.get_err_code() == 10 );
  assert( scene3.get_transaction_id() == "ghijklmno" );
  assert( scene3.get_err() == "Testing" );

  assert( scene3.get_scene(0)->get_key() == "abcdef" );
  assert( scene3.get_scene(0)->get_latitude() == 137.0034 );
  assert( scene3.get_scene(0)->get_longitude() == 89.443 );
  assert( scene3.get_scene(0)->num_devices() == 2 );
  assert( scene3.get_scene(0)->get_device(0)->get_key() == "xyzabcdef1" );
  assert( scene3.get_scene(0)->get_device(1)->get_key() == "xyzabcdef2" );

  scene3.print();

  shutdown_logging_submodules();

  delete logging;
  delete factory;

  return 0;
}

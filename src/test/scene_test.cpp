#include "../scene.h"
#include "../ivan_log.h"
#include <iostream>
#include <Eigen/Dense>
#include <assert.h>
#include <string>

#include "aossl/factory/logging_interface.h"
#include "aossl/factory_logging.h"

int main() {

  LoggingComponentFactory *factory = new LoggingComponentFactory;

  std::string initFileName = "log4cpp.properties";
  logging = factory->get_logging_interface(initFileName);

  start_logging_submodules();

  //Basic tests
  Scene scene1;

  SceneData * data1 = scene1.add_scene();

  //Basic properties
  scene1.set_msg_type(1);
  scene1.set_err_code(10);
  scene1.set_err_msg("Testing");
  scene1.set_transaction_id("ghijklmno");

  data1->set_name("TestName");
  data1->set_key("abcdef");
  data1->set_latitude(137.0034);
  data1->set_longitude(89.443);

  assert( scene1.get_msg_type() == 1 );
  assert( scene1.get_err_code() == 10 );
  assert( scene1.get_err() == "Testing" );
  assert( scene1.get_transaction_id() == "ghijklmno" );

  assert( scene1.get_scene(0)->get_name() == "TestName" );
  assert( scene1.get_scene(0)->get_key() == "abcdef" );
  assert( scene1.get_scene(0)->get_latitude() == 137.0034 );
  assert( scene1.get_scene(0)->get_longitude() == 89.443 );

  //List properties
  UserDevice *ud1 = scene1.get_scene(0)->add_device("xyzabcdef1");
  UserDevice *ud2 = scene1.get_scene(0)->add_device("xyzabcdef2");

  assert( scene1.get_scene(0)->num_devices() == 2 );
  assert( scene1.get_scene(0)->get_device(0)->get_key() == "xyzabcdef1" );
  assert( scene1.get_scene(0)->get_device(1)->get_key() == "xyzabcdef2" );
  assert( !(scene1.get_scene(0)->has_transform()) );

  //Transform tests

  Eigen::Matrix4d transform_matrix = Eigen::Matrix4d::Identity(4, 4);
  transform_matrix(0,0) = 2.0;
  scene1.get_scene(0)->set_transform_matrix(transform_matrix);

  assert( scene1.get_scene(0)->has_transform() );
  assert( scene1.get_scene(0)->get_transform(0, 0) == 2.0 );
  assert( scene1.get_scene(0)->get_transform(1, 1) == 1.0 );
  assert( scene1.get_scene(0)->get_transform(2, 2) == 1.0 );
  assert( scene1.get_scene(0)->get_transform(3, 3) == 1.0 );
  assert( scene1.get_scene(0)->get_transform(1, 0) == 0.0 );
  assert( scene1.get_scene(0)->get_transform(0, 1) == 0.0 );

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
  assert( scene3.get_scene(0)->has_transform() );
  assert( scene3.get_scene(0)->get_transform(0, 0) == 2.0 );
  assert( scene3.get_scene(0)->get_transform(1, 1) == 1.0 );
  assert( scene3.get_scene(0)->get_transform(2, 2) == 1.0 );
  assert( scene3.get_scene(0)->get_transform(3, 3) == 1.0 );
  assert( scene3.get_scene(0)->get_transform(1, 0) == 0.0 );
  assert( scene3.get_scene(0)->get_transform(0, 1) == 0.0 );

  shutdown_logging_submodules();

  delete logging;
  delete factory;

  return 0;
}

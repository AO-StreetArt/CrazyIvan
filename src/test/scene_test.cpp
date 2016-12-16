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
  Scene scene1 ();

  //Basic properties
  scene1.set_msg_type(1);
  scene1.set_key("abcdef");
  scene1.set_err_msg("Testing");
  scene1.set_transaction_id("ghijklmno");
  scene1.set_latitude(137.0034);
  scene1.set_longitude(89.443);
  scene1.set_err_code(10);

  assert( scene1.get_msg_type() == 1 );
  assert( scene1.get_key() == "abcdef" );
  assert( scene1.get_err_msg() == "Testing" );
  assert( scene1.get_transaction_id() == "ghijklmno" );
  assert( scene1.get_latitude() == 137.0034 );
  assert( scene1.get_longitude() == 89.443 );
  assert( scene1.get_err_code() == 10 );

  //List properties
  scene1.add_object_id("pqrstuvw1");
  scene1.add_object_id("pqrstuvw2");
  scene1.add_device_id("xyzabcdef1");
  scene1.add_device_id("xyzabcdef2");

  assert( scene1.num_objects() == 2 );
  assert( scene1.num_devices() == 2 );
  assert( scene1.get_object_id(0) == "pqrstuvw1" );
  assert( scene1.get_object_id(1) == "pqrstuvw2" );
  assert( scene1.get_device_id(0) == "xyzabcdef1" );
  assert( scene1.get_device_id(1) == "xyzabcdef2" );
  assert( !(scene1.has_transform()) );

  //Transform tests

  Eigen::Matrix4d transform_matrix = Eigen::Matrix4d::Identity(4, 4);

  Scene scene2 (transform_matrix);

  assert( scene2.has_transform() );
  assert( scene2.get_transform(0, 0) == 1.0 );
  assert( scene2.get_transform(1, 1) == 1.0 );
  assert( scene2.get_transform(2, 2) == 1.0 );
  assert( scene2.get_transform(3, 3) == 1.0 );
  assert( scene2.get_transform(1, 0) == 0.0 );
  assert( scene2.get_transform(0, 1) == 0.0 );

  //Protocol Buffer tests
  scene2.set_msg_type(1);
  scene2.set_key("abcdef");
  scene2.set_err_msg("Testing");
  scene2.set_transaction_id("ghijklmno");
  scene2.set_latitude(137.0034);
  scene2.set_longitude(89.443);
  scene2.set_err_code(10);
  scene2.add_object_id("pqrstuvw1");
  scene2.add_object_id("pqrstuvw2");
  scene2.add_device_id("xyzabcdef1");
  scene2.add_device_id("xyzabcdef2");

  std::string proto_string = scene2.to_protobuf();
  protoScene::Scene new_proto;
  new_proto.ParseFromString(proto_string);
  Scene scene3 (new_proto);

  assert( scene3.get_msg_type() == 1 );
  assert( scene3.get_key() == "abcdef" );
  assert( scene3.get_err_msg() == "Testing" );
  assert( scene3.get_transaction_id() == "ghijklmno" );
  assert( scene3.get_latitude() == 137.0034 );
  assert( scene3.get_longitude() == 89.443 );
  assert( scene3.get_err_code() == 10 );
  assert( scene3.num_objects() == 2 );
  assert( scene3.num_devices() == 2 );
  assert( scene3.get_object_id(0) == "pqrstuvw1" );
  assert( scene3.get_object_id(1) == "pqrstuvw2" );
  assert( scene3.get_device_id(0) == "xyzabcdef1" );
  assert( scene3.get_device_id(1) == "xyzabcdef2" );
  assert( scene3.has_transform() );
  assert( scene3.get_transform(0, 0) == 1.0 );
  assert( scene3.get_transform(1, 1) == 1.0 );
  assert( scene3.get_transform(2, 2) == 1.0 );
  assert( scene3.get_transform(3, 3) == 1.0 );
  assert( scene3.get_transform(1, 0) == 0.0 );
  assert( scene3.get_transform(0, 1) == 0.0 );

  shutdown_logging_submodules();

  delete logging;
  delete factory;

  return 0;
}

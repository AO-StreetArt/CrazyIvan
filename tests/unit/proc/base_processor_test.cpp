#include <string>

#include "include/base_message_processor.h"

#include "include/scene_list_interface.h"
#include "include/scene_interface.h"
#include "include/transform_interface.h"
#include "include/transform_factory.h"

#include "aossl/uuid/include/uuid_interface.h"
#include "aossl/uuid/include/factory_uuid.h"

#include "catch.hpp"

TEST_CASE( "Test Message Processor Utility Functions", "[unit]" ) {
  // Build the base message processor
  AOSSL::UuidComponentFactory uuid_factory;
  AOSSL::UuidInterface *uuid = uuid_factory.get_uuid_interface();
  BaseMessageProcessor proc(NULL, NULL, NULL, uuid);

  // Test that we return a valid success response for kill and ping
  ProcessResult *ping_result = proc.process_ping_message(NULL);
  REQUIRE(ping_result->get_error_code() == 100);
  REQUIRE(ping_result->successful());
  REQUIRE(ping_result->get_error_description().empty());
  REQUIRE(ping_result->get_return_string().empty());

  ProcessResult *kill_result = proc.process_ping_message(NULL);
  REQUIRE(kill_result->get_error_code() == 100);
  REQUIRE(kill_result->successful());
  REQUIRE(kill_result->get_error_description().empty());
  REQUIRE(kill_result->get_return_string().empty());

  // Test that we return a valid string from our UUID function
  std::string uuid_str;
  proc.create_uuid(uuid_str);
  REQUIRE(!(uuid_str.empty()));

  // Test that we return valid return objects from the return helper
  SceneListInterface *list = proc.build_response_scene(1, 102, "test", "test1", "id1");
  REQUIRE(list->get_msg_type() == 1);
  REQUIRE(list->get_err_code() == 102);
  REQUIRE(list->get_err() == "test");
  REQUIRE(list->get_transaction_id() == "test1");
  REQUIRE(list->get_scene(0)->get_key() == "id1");

  // Test that we return valid strings from our return helpers
  std::string out_str1;
  proc.build_string_response(1, 102, "test", "test1", "id1", out_str1);
  REQUIRE(!(out_str1.empty()));
  std::string out_str2;
  TransformFactory tfactory;
  TransformInterface *trans = tfactory.build_transform();
  trans->translate(0, 1.0);
  trans->translate(1, 2.0);
  trans->translate(2, 3.0);
  trans->rotate(0, 45.0);
  trans->rotate(1, 90.0);
  trans->rotate(2, 135.0);
  proc.build_string_response(1, 102, "test", "test1", "id1", "id2", trans, out_str2);
  REQUIRE(!(out_str2.empty()));

  delete list;
  delete kill_result;
  delete ping_result;
  delete uuid;
}

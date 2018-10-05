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

// This implements the Configuration Manager

// This takes in a Command Line Interpreter, and based on the options provided,
// decides how the application needs to be configured.  It may configure either
// from a configuration file, or from a Consul agent

#include <iostream>
#include <boost/cstdint.hpp>

#include "scene_base_handler.h"

#include "user/include/account_manager_interface.h"

#include "model/include/scene_factory.h"
#include "model/include/scene_interface.h"

#include "api/include/scene_list_factory.h"
#include "api/include/scene_list_interface.h"

#include "proc/processor/include/processor_interface.h"

#include "aossl/core/include/kv_store_interface.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "app/include/ivan_utils.h"

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "Poco/Util/ServerApplication.h"

#include "Poco/Logger.h"

#ifndef SRC_CONTROLLER_INCLUDE_SCENE_KEY_HANDLER_H_
#define SRC_CONTROLLER_INCLUDE_SCENE_KEY_HANDLER_H_

class SceneKeyRequestHandler: public Poco::Net::HTTPRequestHandler {
  ProcessorInterface *proc = NULL;
  AOSSL::KeyValueStoreInterface *config = NULL;
  std::string input_key;
  SceneListFactory scene_list_factory;
  SceneFactory scene_factory;
  int msg_type = -1;
 public:
  SceneKeyRequestHandler(AOSSL::KeyValueStoreInterface *conf, ProcessorInterface *processor, std::string &inp_key, int mtype) \
    {config=conf;proc=processor;input_key.assign(inp_key);msg_type=mtype;}
  void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    Poco::Logger::get("Controller").debug("Responding to Scene Key Request");
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    SceneListInterface *response_body = scene_list_factory.build_json_scene();
    // Build the query to the message processor
    SceneListInterface *query_body = scene_list_factory.build_json_scene();
    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    SceneInterface *inp_doc = scene_factory.build_scene();
    inp_doc->set_key(input_key);
    query_body->add_scene(inp_doc);
    ProcessResult *result = nullptr;
    if (msg_type == SCENE_DEL) {
      query_body->set_msg_type(SCENE_DEL);
      result = proc->process_delete_message(query_body);
    } else if (msg_type == SCENE_GET) {
      query_body->set_msg_type(SCENE_GET);
      result = proc->process_query_message(query_body);
    }
    response_body->set_msg_type(msg_type);
    // Set up the response
    if (result->successful()) {
      SceneInterface *key_scn = scene_factory.build_scene();
      response_body->set_err_code(NO_ERROR);
      key_scn->set_key(result->get_return_string());
      response_body->add_scene(key_scn);
    } else {
      response_body->set_err_code(result->get_error_code());
      response_body->set_err_msg(result->get_error_description());
    }
    // Send the response
    std::ostream& ostr = response.send();
    std::string response_body_string;
    response_body->to_msg_string(response_body_string);
    ostr << response_body_string;
    ostr.flush();
    delete inp_doc;
    delete result;
    delete response_body;
  }
};

#endif  // SRC_CONTROLLER_INCLUDE_SCENE_KEY_HANDLER_H_

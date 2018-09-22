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

#include "app/include/ivan_utils.h"

#include "proc/processor/include/processor_interface.h"

#include "aossl/core/include/kv_store_interface.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "Poco/Util/ServerApplication.h"

#include "Poco/Logger.h"

#ifndef SRC_CONTROLLER_INCLUDE_ASSET_UPDATE_HANDLER_H_
#define SRC_CONTROLLER_INCLUDE_ASSET_UPDATE_HANDLER_H_

class AssetUpdateRequestHandler: public Poco::Net::HTTPRequestHandler {
  ProcessorInterface *proc = NULL;
  AOSSL::KeyValueStoreInterface *config = NULL;
  std::string scene_key;
  std::string asset_key;
  int msg_type = -1;
  SceneListFactory scene_list_factory;
  SceneFactory scene_factory;
 public:
  AssetUpdateRequestHandler(AOSSL::KeyValueStoreInterface *conf, ProcessorInterface *processor, std::string &scene, std::string& asset, int mtype) \
    {config=conf;proc=processor;scene_key.assign(scene);asset_key.assign(asset);msg_type=mtype;}
  void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    Poco::Logger::get("Controller").debug("Responding to Asset Update Request");
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    SceneListInterface *response_body = scene_list_factory.build_json_scene();

    // Set up the input for the message processor
    SceneListInterface *update_body = scene_list_factory.build_json_scene();
    update_body->set_msg_type(SCENE_UPD);
    if (msg_type == ASSET_ADD) {
      update_body->set_op_type(APPEND);
    } else if (msg_type == ASSET_DEL) {
      update_body->set_op_type(REMOVE);
    }
    SceneInterface *update_scene = scene_factory.build_scene();
    update_scene->set_key(scene_key);
    update_scene->add_asset(asset_key);
    update_body->add_scene(update_scene);

    // Process the scene update
    ProcessResult *result = proc->process_update_message(update_body);

    // Set up the response
    if (result->successful()) {
      SceneInterface *key_scn = scene_factory.build_scene();
      response_body->set_msg_type(SCENE_UPD);
      response_body->set_err_code(NO_ERROR);
      key_scn->set_key(result->get_return_string());
      response_body->add_scene(key_scn);
    } else {
      response_body->set_msg_type(SCENE_UPD);
      response_body->set_err_code(result->get_error_code());
      response_body->set_err_msg(result->get_error_description());
    }
    // Send the response
    std::ostream& ostr = response.send();
    std::string response_body_string;
    response_body->to_msg_string(response_body_string);
    ostr << response_body_string;
    ostr.flush();
    delete result;
    delete response_body;
    delete update_body;
  }
};

#endif  // SRC_CONTROLLER_INCLUDE_ASSET_UPDATE_HANDLER_H_

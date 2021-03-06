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

#ifndef SRC_CONTROLLER_INCLUDE_SCENE_UPDATE_HANDLER_H_
#define SRC_CONTROLLER_INCLUDE_SCENE_UPDATE_HANDLER_H_

class SceneUpdateRequestHandler: public Poco::Net::HTTPRequestHandler {
  ProcessorInterface *proc = NULL;
  AOSSL::KeyValueStoreInterface *config = NULL;
  std::string input_key;
  SceneListFactory scene_list_factory;
  SceneFactory scene_factory;
  std::string aesel_principal = "";
 public:
  SceneUpdateRequestHandler(AOSSL::KeyValueStoreInterface *conf, ProcessorInterface *processor, std::string &inp_key, std::string& principal_header) \
    {config=conf;proc=processor;input_key.assign(inp_key);aesel_principal.assign(principal_header);}
  void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    Poco::Logger::get("Controller").debug("Responding to Scene Update Request");
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    // parse the post input data into a Scene List
    rapidjson::Document doc;
    char *tmpStr = ivan_request_body_to_json_document(request, doc);
    SceneListInterface *response_body = scene_list_factory.build_json_scene();
    if (doc.HasParseError()) {
      // Set up parse error response
      response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      response_body->set_msg_type(SCENE_UPD);
      response_body->set_err_code(TRANSLATION_ERROR);
      response_body->set_err_msg(rapidjson::GetParseError_En(doc.GetParseError()));
      std::ostream& ostr = response.send();
      std::string response_body_string;
      response_body->to_msg_string(response_body_string);
      ostr << response_body_string;
      ostr.flush();
    } else {
      // Convert the rapidjson doc to a scene list
      response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
      SceneListInterface *inp_doc = scene_list_factory.build_scene(doc);
      // update the message type and process the post input data
      inp_doc->set_msg_type(SCENE_CRT);
      if (inp_doc->num_scenes() == 0) {
        Poco::Logger::get("Controller").warning("Update Request recieved with no body");
        SceneInterface *new_update_doc = scene_factory.build_scene();
        inp_doc->add_scene(new_update_doc);
      }
      inp_doc->get_scene(0)->set_key(input_key);
      ProcessResult *result = proc->process_update_message(inp_doc);
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
      if (response_body->get_err_code() == NOT_FOUND) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
      } else if (response_body->get_err_code() != NO_ERROR) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
      }
      std::ostream& ostr = response.send();
      std::string response_body_string;
      response_body->to_msg_string(response_body_string);
      ostr << response_body_string;
      ostr.flush();
      delete inp_doc;
      delete result;
    }
    delete response_body;
    delete[] tmpStr;
  }
};

#endif  // SRC_CONTROLLER_INCLUDE_SCENE_CREATE_HANDLER_H_

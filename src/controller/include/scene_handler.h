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

#include "user/include/account_manager_interface.h"

#include "model/include/scene_factory.h"
#include "model/include/scene_interface.h"

#include "api/include/scene_list_factory.h"
#include "api/include/scene_list_interface.h"

#include "proc/processor/include/processor_interface.h"

#include "aossl/core/include/kv_store_interface.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "Poco/Util/ServerApplication.h"

#include "Poco/Logger.h"

#ifndef SRC_CONTROLLER_INCLUDE_SCENE_HANDLER_H_
#define SRC_CONTROLLER_INCLUDE_SCENE_HANDLER_H_

// Returns a buffer containing the request body
static inline char* ivan_request_body_to_json_document(Poco::Net::HTTPServerRequest& request, rapidjson::Document& doc) {
  // Pull the request body out of a stream and into a character buffer
  int length = request.getContentLength();
  std::istream &request_stream = request.stream();
  char *buffer = new char[length];
  request_stream.read(buffer, length);

  // Parse the buffer containing the request body into the rapidjson document
  doc.Parse<rapidjson::kParseStopWhenDoneFlag>(buffer);

  // Return the buffer, as it needs to be valid while we're using the document
  return buffer;
}

class SceneCreateRequestHandler: public Poco::Net::HTTPRequestHandler {
  ProcessorInterface *proc = NULL;
  AOSSL::KeyValueStoreInterface *config = NULL;
  AccountManagerInterface *accounts = NULL;
  SceneListFactory scene_list_factory;
  SceneFactory scene_factory;
 public:
  SceneCreateRequestHandler(AOSSL::KeyValueStoreInterface *conf, ProcessorInterface *processor, AccountManagerInterface *accts) \
    {config=conf;proc=processor;accounts=accts;}
  void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    Poco::Logger::get("Controller").debug("Responding to Scene Create Request");
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    // parse the post input data into a Scene List
    rapidjson::Document doc;
    char *tmpStr = ivan_request_body_to_json_document(request, doc);
    SceneListInterface *response_body = scene_list_factory.build_json_scene();
    if (doc.HasParseError()) {
      // Set up parse error response
      response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      response_body->set_msg_type(SCENE_CRT);
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
      ProcessResult *result = proc->process_create_message(inp_doc);
      // Set up the response
      if (result->successful()) {
        SceneInterface *key_scn = scene_factory.build_scene();
        response_body->set_msg_type(SCENE_CRT);
        response_body->set_err_code(NO_ERROR);
        key_scn->set_key(result->get_return_string());
        response_body->add_scene(key_scn);
      } else {
        response_body->set_msg_type(SCENE_CRT);
        response_body->set_err_code(result->get_error_code());
        response_body->set_err_msg(result->get_error_description());
      }
      // Send the response
      std::ostream& ostr = response.send();
      std::string response_body_string;
      response_body->to_msg_string(response_body_string);
      ostr << response_body_string;
      ostr.flush();
    }
    delete response_body;
    delete[] tmpStr;
  }
};

#endif  // SRC_CONTROLLER_INCLUDE_SCENE_HANDLER_H_

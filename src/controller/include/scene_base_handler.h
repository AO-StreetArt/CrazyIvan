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

#include <iostream>
#include <boost/cstdint.hpp>

#include "app/include/ivan_utils.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#ifndef SRC_CONTROLLER_INCLUDE_SCENE_BASE_HANDLER_H_
#define SRC_CONTROLLER_INCLUDE_SCENE_BASE_HANDLER_H_

// Convert Request Contents into a Rapidjson Document
// Returns a char* which must be deleted only after using the Rapidjson Doc
static inline char* ivan_request_body_to_json_document(Poco::Net::HTTPServerRequest& request, \
    rapidjson::Document& doc) {
  // Pull the request body out of a stream and into a character buffer
  int length = request.getContentLength();
  std::istream &request_stream = request.stream();
  char *buffer = new char[length];
  request_stream.read(buffer, length);

  // Parse the buffer containing the request body into the rapidjson document
  doc.Parse<rapidjson::kParseStopWhenDoneFlag>(buffer);

  // Return the buffer, as it needs to be valid while we're using the document
  return buffer;
};

class SceneBaseRequestHandler: public Poco::Net::HTTPRequestHandler {
  ProcessorInterface *proc = NULL;
  AOSSL::KeyValueStoreInterface *config = NULL;
  int msg_type = -1;
  SceneListFactory scene_list_factory;
  SceneFactory scene_factory;
  std::string input_id = "";
  std::string aesel_principal = "";
 public:
  SceneBaseRequestHandler(AOSSL::KeyValueStoreInterface *conf, ProcessorInterface *processor, int mtype, std::string& principal_header) \
    {config=conf;proc=processor;msg_type=mtype;aesel_principal.assign(principal_header);}
  SceneBaseRequestHandler(AOSSL::KeyValueStoreInterface *conf, ProcessorInterface *processor, int mtype, std::string& principal_header, std::string& id) \
    {config=conf;proc=processor;msg_type=mtype;aesel_principal.assign(principal_header);input_id.assign(id);}
  void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
    Poco::Logger::get("Controller").debug("Responding to Scene Request");
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    // parse the post input data into a Scene List
    rapidjson::Document doc;
    char *tmpStr = ivan_request_body_to_json_document(request, doc);
    SceneListInterface *response_body = scene_list_factory.build_json_scene();
    response_body->set_msg_type(msg_type);
    if (doc.HasParseError()) {
      // Set up parse error response
      response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
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
      inp_doc->set_msg_type(msg_type);
      ProcessResult *result = NULL;
      if (msg_type == SCENE_ENTER) {
        result = proc->process_registration_message(inp_doc);
      } else if (msg_type == SCENE_LEAVE) {
        result = proc->process_deregistration_message(inp_doc);
      } else if (msg_type == SCENE_CRT) {
        if (!(input_id.empty()) && inp_doc->num_scenes() > 0) {
          inp_doc->get_scene(0)->set_key(input_id);
        }
        result = proc->process_create_message(inp_doc);
      } else if (msg_type == SCENE_GET) {
        result = proc->process_query_message(inp_doc);
      } else if (msg_type == DEVICE_ALIGN) {
        result = proc->process_device_alignment_message(inp_doc);
      } else if (msg_type == DEVICE_GET) {
        result = proc->process_device_get_message(inp_doc);
      }
      // Set up the response
      if (result->get_error_code() == NOT_FOUND) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
      } else if (result->get_error_code() != NO_ERROR) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
      }
      std::ostream& ostr = response.send();
      if (result->successful()) {
        if (msg_type == SCENE_GET || msg_type == SCENE_ENTER || \
            msg_type == SCENE_LEAVE || msg_type == DEVICE_ALIGN || msg_type == DEVICE_GET) {
          ostr << result->get_return_string();
          ostr.flush();
        } else {
          SceneInterface *key_scn = scene_factory.build_scene();
          response_body->set_err_code(NO_ERROR);
          key_scn->set_key(result->get_return_string());
          response_body->add_scene(key_scn);
          std::string response_body_string;
          response_body->to_msg_string(response_body_string);
          ostr << response_body_string;
          ostr.flush();
        }
      } else {
        response_body->set_err_code(result->get_error_code());
        response_body->set_err_msg(result->get_error_description());
        std::string response_body_string;
        response_body->to_msg_string(response_body_string);
        ostr << response_body_string;
        ostr.flush();
      }
      delete inp_doc;
      delete result;
    }
    delete response_body;
    delete[] tmpStr;
  }
};

#endif  // SRC_CONTROLLER_INCLUDE_SCENE_BASE_HANDLER_H_

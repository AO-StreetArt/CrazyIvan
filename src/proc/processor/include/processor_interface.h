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

#include <string>

#include "api/include/scene_list_interface.h"
#include "api/include/scene_list_factory.h"

#ifndef SRC_PROC_PROCESSOR_INCLUDE_PROCESSOR_INTERFACE_H_
#define SRC_PROC_PROCESSOR_INCLUDE_PROCESSOR_INTERFACE_H_

class ProcessResult {
  int err_code;
  std::string err_desc;
  const char * err_cdesc;
  std::string return_str;
  bool success;

 public:
  // Default construct returns a successful process result
  inline ProcessResult() {
    err_code = 100;
    err_desc = "";
    success = true;
    return_str = "";
  }

  // Error constructor
  inline ProcessResult(int ecode, std::string edesc) {
    err_code = ecode;
    err_desc.assign(edesc);
    success = false;
    return_str = "";
  }

  // Success constructor
  inline ProcessResult(std::string data) {
    return_str.assign(data);
    err_code = 100;
    err_desc = "";
    success = true;
  }

  // Set the error
  inline void set_error(int ecode, std::string edesc) {
    err_code = ecode;
    err_desc.assign(edesc);
    success = false;
    return_str = "";
  }

  // Set the error
  inline void set_error(int ecode, const char * edesc) {
    err_code = ecode;
    err_cdesc = edesc;
    err_desc.assign(err_cdesc);
    success = false;
    return_str = "";
  }

  int get_error_code() {return err_code;}
  std::string get_error_description() {return err_desc;}
  void set_return_string(std::string key) {return_str.assign(key);}
  std::string get_return_string() {return return_str;}
  bool successful() {return success;}
};

// The class containing core logic for CrazyIvan
// Accepts an Scene (assuming it is an inbound message)
// And performs any and all processing on it
class ProcessorInterface {
 public:
  // Destructor
  virtual ~ProcessorInterface() {}

  // Process a batch create message
  virtual ProcessResult* process_create_message(SceneListInterface *obj_msg) = 0;

  // Process a batch update message
  virtual ProcessResult* process_update_message(SceneListInterface *obj_msg) = 0;

  // Process a query message
  virtual ProcessResult* process_query_message(SceneListInterface *obj_msg) = 0;

  // Process a batch delete message
  virtual ProcessResult* process_delete_message(SceneListInterface *obj_msg) = 0;

  // Process a batch delete message
  virtual ProcessResult* process_registration_message(SceneListInterface *obj_msg) = 0;

  // Process a batch delete message
  virtual ProcessResult* process_deregistration_message(SceneListInterface *obj_msg) = 0;

  // Process a batch delete message
  virtual ProcessResult* process_device_alignment_message(SceneListInterface *obj_msg) = 0;

  // Process a device retrieval message
  virtual ProcessResult* process_device_get_message(SceneListInterface *obj_msg) = 0;
};

#endif  // SRC_PROC_PROCESSOR_INCLUDE_PROCESSOR_INTERFACE_H_

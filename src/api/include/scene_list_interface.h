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

// Message creator for SceneList Protcol Buffer object

#include <string>

#include "model/include/scene_interface.h"

#ifndef SRC_API_INCLUDE_SCENE_LIST_INTERFACE_H_
#define SRC_API_INCLUDE_SCENE_LIST_INTERFACE_H_

// A single scene message, may include data for multiple scene objects
class SceneListInterface {
 public:
  virtual ~SceneListInterface() {}
  // Convert the scene information into a message string
  virtual void to_msg_string(std::string &out_string) = 0;
  // Setters
  virtual void set_msg_type(int new_msg_type) = 0;
  virtual void set_op_type(int new_operation) = 0;
  virtual void set_err_msg(std::string new_err) = 0;
  virtual void set_event_destination_host(std::string new_host) = 0;
  virtual void set_event_destination_port(int new_port) = 0;
  virtual void set_decryption_password(std::string new_password) = 0;
  virtual void set_decryption_salt(std::string new_salt) = 0;
  virtual void set_decryption_key(std::string new_key) = 0;
  virtual void set_decryption_iv(std::string new_iv) = 0;
  virtual void set_encryption_password(std::string new_password) = 0;
  virtual void set_encryption_salt(std::string new_salt) = 0;
  virtual void set_encryption_key(std::string new_key) = 0;
  virtual void set_encryption_iv(std::string new_iv) = 0;
  virtual void set_transaction_id(std::string new_tran_id) = 0;
  virtual void set_err_code(int new_code) = 0;
  virtual void add_scene(SceneInterface *scn) = 0;
  virtual void set_num_records(int new_num) = 0;
  virtual void set_start_record(int new_num) = 0;
  // Getters
  virtual int get_msg_type() = 0;
  virtual int get_op_type() = 0;
  virtual std::string get_err() = 0;
  virtual std::string get_transaction_id() = 0;
  virtual std::string get_event_destination_host() = 0;
  virtual int get_event_destination_port() = 0;
  virtual std::string get_decryption_key() = 0;
  virtual std::string get_encryption_key() = 0;
  virtual std::string get_decryption_salt() = 0;
  virtual std::string get_encryption_salt() = 0;
  virtual int get_err_code() = 0;
  virtual SceneInterface* get_scene(unsigned int i) = 0;
  virtual int num_scenes() = 0;
  virtual int get_num_records() = 0;
  virtual int get_start_record() = 0;
  virtual void print() = 0;
};

#endif  // SRC_API_INCLUDE_SCENE_LIST_INTERFACE_H_

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
#include <vector>
#include <algorithm>

#include "model/include/scene_interface.h"
#include "model/include/user_device_factory.h"
#include "model/include/transform_factory.h"

#include "scene_list_interface.h"

#include "app/include/ivan_utils.h"

#include "Poco/Logger.h"

#ifndef SRC_API_INCLUDE_SCENE_LIST_H_
#define SRC_API_INCLUDE_SCENE_LIST_H_

// A single scene message, may include data for multiple scene objects
class SceneList : public SceneListInterface {
  // Factories
  TransformFactory tfactory;
  UserDeviceFactory udfactory;
  // Logger
  Poco::Logger& logr;
  // Core attributes
  int msg_type = -1;
  int operation = APPEND;
  std::string err_msg = "";
  int err_code = 100;
  std::string transaction_id = "";
  int start_record = 0;
  int num_records = 10;
  std::vector<SceneInterface*> data;
  // Attributes to store json string values
  std::string ret_string = "";
  const char* ret_val;
  // Returned during registration
  std::string event_destination_host;
  int event_destination_port;
  // Decryption info
  std::string decryption_password;
  std::string decryption_salt;
  std::string decryption_key;
  std::string decryption_iv;
  // Encryption info
  std::string encryption_password;
  std::string encryption_salt;
  std::string encryption_key;
  std::string encryption_iv;
 public:
  inline Poco::Logger& logger() {return logr;}
  inline TransformInterface* create_transform() \
    {return tfactory.build_transform();}
  inline UserDeviceInterface* create_device() {return udfactory.build_device();}
  inline UserDeviceInterface* \
    create_device(std::string inp_key, TransformInterface *t) \
    {return udfactory.build_device(inp_key, t);}
  inline UserDeviceInterface* create_device(std::string inp_key) \
    {return udfactory.build_device(inp_key);}
  // Constructor
  SceneList() : logr(Poco::Logger::get("Data")) {}
  // Destructor
  virtual inline ~SceneList() {
    for (unsigned int i = 0; i < data.size(); i++) {
      if (data[i]) delete data[i];
    }
  }
  // Convert the scene information into a message string
  virtual void to_msg_string(std::string &out_string) = 0;
  // Setters
  void set_msg_type(int new_msg_type) {msg_type = new_msg_type;}
  void set_op_type(int new_operation) {operation = new_operation;}
  void set_err_msg(std::string new_err) {err_msg = new_err;}
  void set_transaction_id(std::string new_tran_id) \
    {transaction_id = new_tran_id;}
  void set_event_destination_host(std::string new_host) {event_destination_host.assign(new_host);}
  void set_event_destination_port(int new_port) {event_destination_port=new_port;}
  void set_decryption_password(std::string new_password) {decryption_password.assign(new_password);}
  void set_decryption_salt(std::string new_salt) {decryption_salt.assign(new_salt);}
  void set_decryption_key(std::string new_key) {decryption_key.assign(new_key);}
  void set_decryption_iv(std::string new_iv) {decryption_iv.assign(new_iv);}
  void set_encryption_password(std::string new_password) {encryption_password.assign(new_password);}
  void set_encryption_salt(std::string new_salt) {encryption_salt.assign(new_salt);}
  void set_encryption_key(std::string new_key) {encryption_key.assign(new_key);}
  void set_encryption_iv(std::string new_iv) {encryption_iv.assign(new_iv);}
  void set_err_code(int new_code) {err_code = new_code;}
  void add_scene(SceneInterface *scn) {data.push_back(scn);}
  void set_num_records(int new_num) {num_records = new_num;}
  void set_start_record(int new_num) {start_record = new_num;}
  // Getters
  int get_msg_type() {return msg_type;}
  int get_op_type() {return operation;}
  std::string get_err() {return err_msg;}
  std::string get_transaction_id() {return transaction_id;}
  std::string get_event_destination_host() {return event_destination_host;}
  int get_event_destination_port() {return event_destination_port;}
  std::string get_decryption_password() {return decryption_password;}
  std::string get_decryption_salt() {return decryption_salt;}
  std::string get_decryption_key() {return decryption_key;}
  std::string get_decryption_iv() {return decryption_iv;}
  std::string get_encryption_password() {return encryption_password;}
  std::string get_encryption_salt() {return encryption_salt;}
  std::string get_encryption_key() {return encryption_key;}
  std::string get_encryption_iv() {return encryption_iv;}
  int get_err_code() {return err_code;}
  inline SceneInterface* get_scene(unsigned int i) {
    if (i < data.size()) return data[i];
    else
      {throw SceneException("Attempted to access invalid scene data");}
  }
  int num_scenes() {return data.size();}
  int get_num_records() {return num_records;}
  int get_start_record() {return start_record;}
  inline void print() {
    logr.debug("{\"SceneList\": {\"msg_type\": %d, \"operation\": %d, \"err_msg\": %s, \"err_code\": %d, \"transaction_id\": \"%s\", \"num_records\": %d}}",
      msg_type, operation, err_msg, err_code, transaction_id, num_records);
    for (unsigned int i = 0; i < data.size(); i++) {data[i]->print();}
  }
};

#endif  // SRC_API_INCLUDE_SCENE_LIST_H_

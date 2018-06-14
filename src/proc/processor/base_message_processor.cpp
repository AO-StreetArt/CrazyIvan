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

#include "include/base_message_processor.h"

void BaseMessageProcessor::create_uuid(std::string &out_string) {
  AOSSL::StringBuffer id_container;
  ugen->generate(id_container);
  if (!id_container.err_msg.empty()) {
    log.error(id_container.err_msg);
  }
  out_string.assign(id_container.val);
}

SceneListInterface* BaseMessageProcessor::build_response_scene(int msg_type, \
  int err_code, std::string err_msg, \
  std::string tran_id, std::string scene_id) {
  SceneListInterface *resp_scn = slfactory.build_json_scene();
  // Set up the scene list
  resp_scn->set_msg_type(msg_type);
  resp_scn->set_err_code(err_code);
  resp_scn->set_err_msg(err_msg);
  resp_scn->set_transaction_id(tran_id);
  // Set up the scene
  SceneInterface *data1 = sfactory.build_scene();
  data1->set_key(scene_id);
  resp_scn->add_scene(data1);
  return resp_scn;
}

void BaseMessageProcessor::build_string_response(int msg_type, int err_code, \
  std::string err_msg, std::string tran_id, \
  std::string scene_id, std::string &out_string) {
  //Construct the response scene
  SceneListInterface *resp_scn = \
    build_response_scene(msg_type, err_code, err_msg, tran_id, scene_id);
  // Convert the response scene to a response message
  resp_scn->to_msg_string(out_string);
  delete resp_scn;
}

void BaseMessageProcessor::build_string_response(int msg_type, int err_code, \
  std::string err_msg, std::string tran_id, std::string scene_id, \
  std::string dev_id, TransformInterface *t, std::string &out_string) {
  SceneListInterface *resp_scn = \
    build_response_scene(msg_type, err_code, err_msg, tran_id, scene_id);
  // Set up the user device
  UserDevice *ud1 = new UserDevice;
  // Add the transformation and key data to the user device
  ud1->set_key(dev_id);
  ud1->set_transform(t);
  // Add the user device to the scene data
  resp_scn->get_scene(0)->add_device(ud1);
  // store the serialized string
  resp_scn->to_msg_string(out_string);
  delete resp_scn;
}

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

#include "include/protobuf_scene_list.h"

// Constructor accepting Protocol Buffer
ProtobufSceneList::ProtobufSceneList(protoScene::SceneList buffer) {
  obj_logging->debug("Starting Conversion from Protocol Buffer");

  // Perform the conversion
  if (buffer.has_message_type()) {
    set_msg_type(buffer.message_type());
    obj_logging->debug(get_msg_type());
  } else {set_msg_type(-1);}
  if (buffer.has_transaction_id()) {
    set_transaction_id(buffer.transaction_id());
    obj_logging->debug(get_transaction_id());
  }
  if (buffer.has_err_msg()) {
    set_err_msg(buffer.err_msg());
    obj_logging->debug(get_err());
  }
  if (buffer.has_err_code()) {
    set_err_code(buffer.err_code());
    obj_logging->debug(get_err_code());
  } else {set_err_code(100);}
  if (buffer.has_num_records()) {
    set_num_records(buffer.num_records());
    obj_logging->debug(get_num_records());
  } else {set_num_records(10);}
  for (int a = 0; a < buffer.scenes_size(); a++) {
    SceneInterface *sc_data = sfactory.build_scene(buffer.scenes(a));
    add_scene(sc_data);
    obj_logging->debug("Scene added");
  }
}

// Convert to Protocol Buffer message
void ProtobufSceneList::to_msg_string(std::string &out_string) {
  // Build the new protocol buffer object
  protoScene::SceneList *new_proto = NULL;
  new_proto = new protoScene::SceneList;

  // Convert basic elements
  if (SceneList::get_msg_type() != -1) {
    new_proto->set_message_type(SceneList::get_msg_type());
  }
  if (!(SceneList::get_transaction_id().empty())) {
    new_proto->set_transaction_id(SceneList::get_transaction_id());
  }
  if (!(SceneList::get_err().empty())) {
    new_proto->set_err_msg(SceneList::get_err());
  }
  new_proto->set_err_code(SceneList::get_err_code());

  // Determine how many scenes we want to return in the message
  int num_return_scenes = SceneList::num_scenes();
  if ((SceneList::get_num_records() != -1) && \
    (SceneList::get_num_records() < num_return_scenes)) {
    num_return_scenes = SceneList::get_num_records();
  }
  new_proto->set_num_records(num_return_scenes);
  // Iterate through the data list and process each scene
  for (int a = 0; a < num_return_scenes; a++) {
    // Build a new scene object
    protoScene::SceneList_Scene *scn = new_proto->add_scenes();

    // Key
    std::string key = SceneList::get_scene(a)->get_key();
    obj_logging->info("Scene:To Proto message Called on object");
    obj_logging->info(key);
    if (!(key.empty())) {
      scn->set_key(key);
    }

    // Name
    std::string name = SceneList::get_scene(a)->get_name();
    if (!(name.empty())) {
      scn->set_name(name);
    }

    // Region
    std::string reg = SceneList::get_scene(a)->get_region();
    if (!(reg.empty())) {
      scn->set_region(reg);
    }

    // Lat/long
    scn->set_latitude(SceneList::get_scene(a)->get_latitude());
    scn->set_longitude(SceneList::get_scene(a)->get_longitude());

    // distance
    if (SceneList::get_scene(a)->get_distance() >= 0.0) {
      scn->set_distance(SceneList::get_scene(a)->get_distance());
    }

    // Asset IDs
    for (int m = 0; m < SceneList::get_scene(a)->num_assets(); m++) {
      scn->add_asset_ids(SceneList::get_scene(a)->get_asset(m));
    }

    // Tags
    for (int n = 0; n < SceneList::get_scene(a)->num_tags(); n++) {
      scn->add_tags(SceneList::get_scene(a)->get_tag(n));
    }

    // Convert transform
    if (SceneList::get_scene(a)->has_transform()) {
      protoScene::SceneList_Transformation *trn = scn->mutable_transform();
      protoScene::SceneList_Vertex3 *proto_translation = \
        trn->mutable_translation();
      protoScene::SceneList_Vertex3 *proto_rot = trn->mutable_rotation();
      proto_translation->set_x(SceneList::get_scene(a)->\
        get_scene_transform()->translation(0));
      proto_translation->set_y(SceneList::get_scene(a)->\
        get_scene_transform()->translation(1));
      proto_translation->set_z(SceneList::get_scene(a)->\
        get_scene_transform()->translation(2));
      proto_rot->set_x(SceneList::get_scene(a)->\
        get_scene_transform()->rotation(0));
      proto_rot->set_y(SceneList::get_scene(a)->\
        get_scene_transform()->rotation(1));
      proto_rot->set_z(SceneList::get_scene(a)->\
        get_scene_transform()->rotation(2));
    }

    // Iterate through the device list for the scene and write those
    for (int b = 0; b < SceneList::get_scene(a)->num_devices(); b++) {
      protoScene::SceneList_UserDevice *ud = scn->add_devices();
      std::string key = SceneList::get_scene(a)->get_device(b)->get_key();
      if (!(key.empty())) {
        ud->set_key(key);
      }

      if (SceneList::get_scene(a)->get_device(b)->has_transform()) {
        protoScene::SceneList_Transformation *trn = ud->mutable_transform();
        protoScene::SceneList_Vertex3 *proto_translation = \
          trn->mutable_translation();
        protoScene::SceneList_Vertex3 *proto_rot = trn->mutable_rotation();
        proto_translation->set_x(SceneList::get_scene(a)->get_device(b)->\
          get_transform()->translation(0));
        proto_translation->set_y(SceneList::get_scene(a)->get_device(b)->\
          get_transform()->translation(1));
        proto_translation->set_z(SceneList::get_scene(a)->get_device(b)->\
          get_transform()->translation(2));
        proto_rot->set_x(SceneList::get_scene(a)->\
          get_device(b)->get_transform()->rotation(0));
        proto_rot->set_y(SceneList::get_scene(a)->\
          get_device(b)->get_transform()->rotation(1));
        proto_rot->set_z(SceneList::get_scene(a)->\
          get_device(b)->get_transform()->rotation(2));
      }
    }
  }

  // Serialize into a std::string
  new_proto->SerializeToString(&out_string);
  obj_logging->debug("Protocol Buffer Serialized to String");
  obj_logging->debug(out_string);
  if (new_proto) {
    delete new_proto;
  }
}

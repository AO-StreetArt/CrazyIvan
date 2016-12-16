#include "scene.h"

//Constructor
Scene::Scene() {
  msg_type=-1;
  key = "";
  err_msg = "";
  transaction_id = "";
  transform_matrix = Eigen::Matrix4d::Identity(4, 4);
  trns_flag = false;
  latitude = 0.0;
  longitude = 0.0;
}

//Constructor accepting Protocol Buffer
Scene::Scene(protoScene::Scene buffer) {

  //Clear the vector data stored in the Scene
  object_ids.clear();
  device_ids.clear();
  geolabels.clear();

  //Fields to store the new variable values
  int new_message_type = -1;
  std::string new_key="";
  Eigen::Matrix4d new_transform=Eigen::Matrix4d::Identity(4, 4);
  trns_flag=false;
  double new_latitude=0.0;
  double new_longitude=0.0;
  std::string new_err_msg = "";
  int new_err_code = 100;
  std::string new_transaction_id = "";

  //Perform the conversion
  if (buffer.has_message_type()) {
		new_message_type = buffer.message_type();
	}
  if (buffer.has_key()) {
    new_key = buffer.key();
  }
  if (buffer.has_transaction_id()) {
    new_transaction_id = buffer.transaction_id();
  }
  if (buffer.has_err_msg()) {
    new_err_msg = buffer.err_msg();
  }
  if (buffer.has_err_code()) {
    new_err_code = buffer.err_code();
  }
  if (buffer.has_latitude()) {
		new_latitude = buffer.latitude();
	}
  if (buffer.has_longitude()) {
		new_longitude = buffer.longitude();
	}
  if (buffer.has_transform()) {
    protoScene::Scene_Matrix4 trn = buffer.transform();
    for (int i=0; i < trn.col_size(); i++) {
      protoScene::Scene_Vertex4 c = trn.col(i);
      new_transform(0, i) = c.w();
      new_transform(1, i) = c.x();
      new_transform(2, i) = c.y();
      new_transform(3, i) = c.z();
    }
    obj_logging->debug("Transform Matrix Parsed");
		transform_matrix=new_transform;
		trns_flag=true;
  }
  if (buffer.obj_ids_size() > 0) {
    for (int j=0; j< buffer.obj_ids_size(); j++) {
      object_ids.push_back(buffer.obj_ids(j));
    }
  }
  if (buffer.device_ids_size() > 0) {
    for (int k=0; k< buffer.device_ids_size(); k++) {
      device_ids.push_back(buffer.device_ids(k));
    }
  }
  if (buffer.geolabels_size() > 0) {
    for (int k=0; k< buffer.geolabels_size(); k++) {
      geolabels.push_back(buffer.geolabels(k));
    }
  }

  //Assign the new values
  msg_type = new_message_type;
  key = new_key;
  latitude = new_latitude;
  longitude = new_longitude;
  err_code = new_err_code;
  err_msg = new_err_msg;
  transaction_id = new_transaction_id;
}

//Convert to Protocol Buffer message
std::string Scene::to_protobuf() {

  obj_logging->info("Scene:To Proto message Called on object");
	obj_logging->info(key);
	protoScene::Scene *new_proto = NULL;
  new_proto = new protoScene::Scene;
  //Convert basic elements
	if (msg_type != -1) {
		new_proto->set_message_type(msg_type);
	}
	if (!transaction_id.empty()) {
		new_proto->set_transaction_id(transaction_id);
	}
  if (!key.empty()) {
		new_proto->set_key(key);
	}
  if (!err_msg.empty()) {
		new_proto->set_err_msg(err_msg);
	}
	new_proto->set_err_code(err_code);
	new_proto->set_latitude(latitude);
	new_proto->set_longitude(longitude);
  //Convert transform
  if (trns_flag) {
		protoScene::Scene_Matrix4 *trn = new_proto->mutable_transform();
		for (int i = 0; i < 4; i++) {
			protoScene::Scene_Vertex4* c1 = trn->add_col();
			c1->set_w(transform_matrix(0, i));
			c1->set_x(transform_matrix(1, i));
			c1->set_y(transform_matrix(2, i));
			c1->set_z(transform_matrix(3, i));
		}
	}
  //Convert object & device ID's
  for (int j = 0; j < num_objects(); j++) {
		new_proto->add_obj_ids(get_object_id(j));
	}
  for (int k = 0; k < num_devices(); k++) {
		new_proto->add_device_ids(get_device_id(k));
	}
  for (int m = 0; m < num_geolabels(); m++) {
		new_proto->add_geolabels(get_geolabel(m));
	}
  //Serialize into a std::string
  std::string wstr;
  new_proto->SerializeToString(&wstr);
	obj_logging->debug("Protocol Buffer Serialized to String");
	obj_logging->debug(wstr);
  if (new_proto) {
	   delete new_proto;
  }
	return wstr;
}

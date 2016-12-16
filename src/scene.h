#include <string>
#include <vector>
#include <algorithm>

#include <Eigen/Dense>

#include "ivan_log.h"
#include "ivan_utils.h"
#include "redis_locking.h"
#include "configuration_manager.h"
#include "Scene.pb.h"

#ifndef SCENE
#define SCENE

class Scene {
int msg_type;
std::string key;
std::vector<std::string> object_ids;
std::vector<std::string> device_ids;
std::vector<std::string> geolabels;
std::string err_msg;
int err_code;
std::string transaction_id;
Eigen::Matrix4d transform_matrix;
bool trns_flag;
double latitude;
double longitude;
public:
  //Constructor
  Scene();
  //Constructor with a transformation matrix
  Scene(Eigen::Matrix4d trans) {transform_matrix=trans;}
  //Constructor accepting Protocol Buffer serialized string
  //Here we parse the string and populate the scene object with the information
  Scene(protoScene::Scene buffer);
  //Destructor
  ~Scene() {}
  //Convert the scene information into a Protocol Buffer serialized message
  std::string to_protobuf();
  //Setters
  void set_msg_type(int new_msg_type) {msg_type=new_msg_type;}
  void set_key(std::string new_key) {key=new_key;}
  void set_err_msg(std::string new_err) {err_msg=new_err;}
  void set_transaction_id(std::string new_tran_id) {transaction_id=new_tran_id;}
  void add_object_id(std::string object_id) {object_ids.push_back(object_id);}
  void remove_object_id(std::string object_id) {object_ids.erase(std::remove(object_ids.begin(), object_ids.end(), object_id), object_ids.end());}
  void add_device_id(std::string device_id) {device_ids.push_back(device_id);}
  void remove_device_id(std::string device_id) {device_ids.erase(std::remove(device_ids.begin(), device_ids.end(), device_id), device_ids.end());}
  void add_geolabel(std::string label) {geolabels.push_back(label);}
  void remove_geolabel(std::string label) {geolabels.erase(std::remove(geolabels.begin(), geolabels.end(), label), geolabels.end());}
  void set_latitude(double new_lat) {latitude=new_lat;}
  void set_longitude(double new_long) {longitude=new_long;}
  void set_err_code(int new_code) {err_code=new_code;}
  //Getters
  int get_msg_type() {return msg_type;}
  std::string get_key() {return key;}
  std::string get_err() {return err_msg;}
  std::string get_transaction_id() {return transaction_id;}
  double get_transform(int row, int col) {return transform_matrix(row, col);}
  Eigen::Matrix4d get_transform_matrix() {return transform_matrix;}
  bool has_transform() {return trns_flag;}
  std::vector<std::string> get_object_ids() {return object_ids;}
  std::vector<std::string> get_device_ids() {return device_ids;}
  int num_objects() {return object_ids.size();}
  int num_devices() {return device_ids.size();}
  int num_geolabels() {return geolabels.size();}
  std::string get_object_id(int index) {return object_ids[index];}
  std::string get_device_id(int index) {return device_ids[index];}
  std::string get_geolabel(int index) {return geolabels[index];}
  double get_latitude() {return latitude;}
  double get_longitude() {return longitude;}
  int get_err_code() {return err_code;}

};

#endif

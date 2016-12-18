//Message accessor for SceneList Protocol Buffer object access
//Message creator for SceneList Protcol Buffer object

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

//User Device
//Stores a transform (relative to current scene) and key
class UserDevice {
Eigen::Matrix4d transform_matrix;
std::string key;
bool trns_flag;
public:
  UserDevice(protoScene::SceneList_UserDevice scn_data);
  UserDevice() {transform_matrix = Eigen::Matrix4d::Identity(4, 4);trns_flag=true;}
  UserDevice(Eigen::Matrix4d transform) {transform_matrix = transform;trns_flag=true;}
  UserDevice(std::string new_key) {key = new_key;trns_flag=false;}
  UserDevice(std::string new_key, Eigen::Matrix4d transform) {key = new_key;transform_matrix = transform;trns_flag=true;}
  ~UserDevice() {}
  void set_transform_matrix(Eigen::Matrix4d new_transform) {transform_matrix = new_transform;trns_flag=true;}
  double get_transform(int row, int col) {return transform_matrix(row, col);}
  Eigen::Matrix4d get_transform_matrix() {return transform_matrix;}
  bool has_transform() {return trns_flag;}
  void set_key(std::string new_key) {key = new_key;}
  std::string get_key() {return key;}
};

//Stores the data for a single scene
class SceneData {
std::string key;
std::string name;
double latitude;
double longitude;
bool trns_flag;
std::vector<UserDevice*> devices;
Eigen::Matrix4d transform_matrix;
public:
  SceneData() {key = "";name = "";latitude=0.0;longitude=0.0;trns_flag=false;}
  SceneData(protoScene::SceneList_Scene scn_data);
  ~SceneData() {for (unsigned int i = 0; i < devices.size(); i++) {if (devices[i]) {delete devices[i];}}}
  //Setters
  void set_key(std::string new_key) {key=new_key;}
  void set_name(std::string new_name) {name=new_name;}
  void set_latitude(double new_lat) {latitude=new_lat;}
  void set_longitude(double new_long) {longitude=new_long;}

  //Getters
  std::string get_key() {return key;}
  std::string get_name() {return name;}
  double get_latitude() {return latitude;}
  double get_longitude() {return longitude;}

  //List
  UserDevice* add_device(std::string device_id) {UserDevice *d = new UserDevice (device_id);devices.push_back(d); return d;}
  int num_devices() {return devices.size();}
  UserDevice* get_device(int index) {return devices[index];}
  std::vector<UserDevice*> get_devices() {return devices;}

  //Transform
  void set_transform_matrix(Eigen::Matrix4d new_transform) {transform_matrix = new_transform;trns_flag=true;}
  double get_transform(int row, int col) {return transform_matrix(row, col);}
  Eigen::Matrix4d get_transform_matrix() {return transform_matrix;}
  bool has_transform() {return trns_flag;}
};

//A single scene message, may include data for multiple scene objects
class Scene {
int msg_type;
std::string err_msg;
int err_code;
std::string transaction_id;
std::vector<SceneData*> data;
public:
  //Constructor
  Scene();
  //Constructor accepting Protocol Buffer serialized string
  //Here we parse the string and populate the scene object with the information
  Scene(protoScene::SceneList buffer);
  //Destructor
  ~Scene() {for (unsigned int i = 0; i < data.size(); i++) {if (data[i]) {delete data[i];}}}
  //Convert the scene information into a Protocol Buffer serialized message
  std::string to_protobuf();
  //Setters
  void set_msg_type(int new_msg_type) {msg_type=new_msg_type;}
  void set_err_msg(std::string new_err) {err_msg=new_err;}
  void set_transaction_id(std::string new_tran_id) {transaction_id=new_tran_id;}
  void set_err_code(int new_code) {err_code=new_code;}
  SceneData* add_scene() {SceneData *scn = new SceneData;data.push_back(scn);return scn;}
  //Getters
  int get_msg_type() {return msg_type;}
  std::string get_err() {return err_msg;}
  std::string get_transaction_id() {return transaction_id;}
  int get_err_code() {return err_code;}
  SceneData* get_scene(int i) {return data[i];}
  int num_scenes() {return data.size();}

};

#endif

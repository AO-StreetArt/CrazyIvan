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

#include <strings.h>
#include <string.h>
#include <stdint.h>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <vector>

#ifndef SRC_APP_INCLUDE_IVAN_UTILS_H_
#define SRC_APP_INCLUDE_IVAN_UTILS_H_

// Universal Transaction Types
const int SCENE_CRT = 0;
const int SCENE_UPD = 1;
const int SCENE_GET = 2;
const int SCENE_DEL = 3;
const int SCENE_ENTER = 4;
const int SCENE_LEAVE = 5;
const int DEVICE_ALIGN = 6;
const int DEVICE_GET = 7;
const int CACHE_ADD = 8;
const int CACHE_DEL = 9;
const int ASSET_ADD = 10;
const int ASSET_DEL = 11;
const int KILL = 999;
const int PING = 555;

// Error Response Codes
const int NO_ERROR = 100;
const int ERROR = 101;
const int NOT_FOUND = 102;
const int TRANSLATION_ERROR = 110;
const int PROCESSING_ERROR = 120;
const int BAD_MSG_TYPE_ERROR = 121;
const int INSUFF_DATA_ERROR = 122;
const int DEVICE_LOCK = 130;

// Operation Types
const int APPEND = 10;
const int REMOVE = 11;

// Query Types
const int CREATE_QUERY_TYPE = 0;
const int GET_QUERY_TYPE = 1;
const int UPDATE_QUERY_TYPE = 2;
const int DELETE_QUERY_TYPE = 3;

inline bool file_exists (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

// String Manipulation
inline void split_from_index(const std::string& input, std::vector<std::string>& output, char delim, int start) {
  std::stringstream ss(input.substr(start, input.size()));
  std::string item;
  while (std::getline(ss, item, delim)) {
    output.push_back(item);
  }
}
inline void split(const std::string& input, std::vector<std::string>& output, char delim) {
  if (input[0] == '/') {
    split_from_index(input, output, delim, 1);
  } else {
    split_from_index(input, output, delim, 0);
  }
}

// remove non-ascii characters
inline bool invalidChar(char c) {
    return !(c >= 0 && c < 128);
}
inline void stripUnicode(std::string &str) {
    str.erase(std::remove_if(str.begin(), str.end(), invalidChar), str.end());
}

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

// Reliable To String Method
#define SSTR(x) static_cast< std::ostringstream & >(\
        (std::ostringstream() << std::dec << x)).str()

#endif  // SRC_APP_INCLUDE_IVAN_UTILS_H_

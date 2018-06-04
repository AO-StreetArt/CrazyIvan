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

#include "Poco/Logger.h"
#include "Poco/HMACEngine.h"
#include "Poco/SHA1Engine.h"

#include "account_manager_interface.h"

#ifndef SRC_USER_INCLUDE_SINGLE_ACCOUNT_MANAGER_H_
#define SRC_USER_INCLUDE_SINGLE_ACCOUNT_MANAGER_H_

// The class responsible for validating against a single login
class SingleAccountManager: public AccountManagerInterface {
  std::string username;
  std::string password;
  std::string hash_pw;
  int type = IVAN_BASIC_AUTH;
  void hash_string(std::string& inp, std::string& out) {
    // compute an HMAC-SHA1
    Poco::HMACEngine<Poco::SHA1Engine> hmac(hash_pw);
    hmac.update(inp);
    const Poco::DigestEngine::Digest& digest = hmac.digest();
    // finish HMAC computation and obtain digest
    out.assign(Poco::DigestEngine::digestToHex(digest));
  }
 public:
  // Destructor
  ~SingleAccountManager() {}

  // Constructor
  SingleAccountManager(std::string& un, std::string& pw, std::string& sha_pw) {
    username.assign(un);
    hash_pw.assign(sha_pw);
    std::string final_pw;
    hash_string(pw, final_pw);
    password.assign(final_pw);
  }

  // Determine if a user is valid or not
  inline bool authenticate_user(std::string& user, std::string& passwd) {
    Poco::Logger::get("Auth").debug("Authenticating User: %s", user);
    Poco::Logger::get("Auth").debug("Comparison: %s", password);
    std::string final_pw;
    hash_string(passwd, final_pw);
    Poco::Logger::get("Auth").debug("Password: %s", final_pw);
    if (final_pw == password) {
      return true;
    }
    return false;
  }

  int auth_type() {return type;}
};

#endif  // SRC_USER_INCLUDE_SINGLE_ACCOUNT_MANAGER_H_

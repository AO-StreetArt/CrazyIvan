#include <string>

#include "include/account_manager_interface.h"
#include "include/account_manager.h"

#include "catch.hpp"

TEST_CASE( "Test User Account Manager", "[unit]" ) {
  std::string username = "test";
  std::string passwd = "password";
  std::string hash_passwd = "passwd";
  SingleAccountManager account_manager(username, passwd, hash_passwd);
  std::string good_un = "test";
  std::string good_pw = "password";
  std::string bad_un = "bad";
  std::string bad_pw = "uhoh";

  REQUIRE(account_manager.authenticate_user(good_un, good_pw));
  REQUIRE(!(account_manager.authenticate_user(bad_un, bad_pw)));
}

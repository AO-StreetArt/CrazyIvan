#include "globals.h"
#include "ivan_log.h"

#include <exception>

#include "aossl/uuid/include/uuid_interface.h"

#ifndef IVAN_UUID
#define IVAN_UUID

inline std::string generate_uuid()
{
  UuidContainer id_container;
  id_container = ua->generate();
  if (!id_container.err.empty()) {
    uuid_logging->error(id_container.err);
  }
  return id_container.id;
}

#endif

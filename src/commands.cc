#include "commands.h"

namespace swaggerfs {
  namespace commands {

    forget_service::forget_service(const std::string& n)
    :name(n)
    {}

    show_service::show_service(const std::string& n)
    :name(n)
    {}

    mount_service::mount_service(const std::string &n, const std::string &mp)
    :name(n), mount_point(mp)
    {}

  }
}
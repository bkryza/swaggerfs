#pragma once

#include <string>

namespace swaggerfs {
  namespace commands {

    struct command {
      virtual ~command() {}
    };


    struct noop : public command {
      virtual ~noop() {}
    };


    struct add_service : public command {
      std::string name;
      std::string swagger_input_source;
      std::string endpoint;
      std::string username;
      std::string password;
      std::string api_key;

      add_service() {}
      virtual ~add_service() {}
    };


    struct forget_service : public command {
      std::string name;

      forget_service() {}
      forget_service(const std::string& n);
      virtual ~forget_service() {}
    };

    struct show_service : public command {
      std::string name;

      show_service() {}
      show_service(const std::string& n);
      virtual ~show_service() {}
    };


    struct list_services : public command {
      list_services() {}
      virtual ~list_services() {}
    };

    struct mount_service : public command {
      std::string name;
      std::string mount_point;

      mount_service() {}
      mount_service(const std::string &n, const std::string &mp);
      virtual ~mount_service() {}
    };

  }
}
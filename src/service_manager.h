#pragma once


#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>


#include "commands.h"

namespace swaggerfs {

  /**
   * Namespace for authentication types
   */
  namespace authentication {

    /**
     * Abstract authentication class
     */
    struct auth_type { 
      virtual ~auth_type() {}
    };

    /**
     * Basic HTTP authentication
     */
    struct basic : public auth_type {
      std::string username;
      std::string password;

      basic();

      basic(const std::string &user, const std::string &pass);

      virtual ~basic() {}
    };

    /**
     * API key based authentication - the header key used in the request
     * will be automatically extracted from the Swagger specification
     */
    struct apikey : public auth_type {
      std::string key;

      apikey();

      apikey(const std::string &akey);

      virtual ~apikey() {}
    };

  }


  struct service {

    std::string name;

    std::string endpoint;
    
    std::shared_ptr<authentication::auth_type> credentials;

    std::string swagger_specification_file;

    boost::property_tree::ptree model; 


    service();

    service(const commands::add_service& cmd);

    std::string get_config_file_name();

    std::string get_swagger_file_name();

    void save(const std::string& folder);

    void load(const std::string& folder);

  };


  class service_manager {

  public:

    void initialize();

    // void handle_command(const commands::command& cmd);

    void add_service(const commands::add_service &cmd);

    std::vector<std::string> list_services();

    std::shared_ptr<service> get_service(const std::string& name);

    void forget_service(std::string name);

  };

}
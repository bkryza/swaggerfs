#include "service_manager.h"
#include "swagger.h"

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


namespace pt = boost::property_tree;
namespace bf = boost::filesystem;

namespace swaggerfs {

  namespace authentication {

      basic::basic() {}

      basic::basic(const std::string &user, const std::string &pass)
        :username(user), password(pass)
        {}
  

      apikey::apikey() {}

      apikey::apikey(const std::string &akey)
        :key(akey)
        {}

  }


  service::service() {}
    
  service::service(const commands::add_service& cmd) {

    name = cmd.name;
    endpoint = cmd.endpoint;
    swagger_specification_file = cmd.swagger_input_source;

    /**
     * Initialize the credentials which also determine the preferred method
     * of authentication (if any) by the user.
     */
    if(!cmd.api_key.empty()) {
      credentials 
        = std::shared_ptr<authentication::apikey>(
            new authentication::apikey(cmd.api_key));
    }
    else if(!cmd.username.empty()) {
      credentials 
        = std::shared_ptr<authentication::basic>(
            new authentication::basic(cmd.username, cmd.password));
    }

  }


  void service::save(const std::string& folder) {

    /**
     * Preparet the file paths for service definition and it's Swagger
     * specification cached copy.
     */
    boost::filesystem::path configuration_file_path 
      = boost::filesystem::path{folder} /= (name+".sfs");

    boost::filesystem::path swagger_cache_file_path 
      = boost::filesystem::path{folder} /= (name+".json");

    std::cerr << "Generating " << configuration_file_path.string() << std::endl;
    std::cerr << "Generating " << swagger_cache_file_path.string() << std::endl;

    /**
     * Prepare the property tree object with service settings
     */
    pt::ptree root;

    root.put("name", name);
    root.put("endpoint", endpoint);

    if(credentials.get()) {
      pt::ptree credentials_root;

      if(dynamic_cast<authentication::basic*>(credentials.get())) {
        credentials_root.put("username", 
          dynamic_cast<authentication::basic*>(credentials.get())->username);
        credentials_root.put("password", 
          dynamic_cast<authentication::basic*>(credentials.get())->password);
      }
      else {
        credentials_root.put("apikey", 
          dynamic_cast<authentication::apikey*>(credentials.get())->key);
      }

      root.add_child("credentials", credentials_root);
    }

    /**
     * Write the service settings to a file - since the file may contain
     * sensitive credentials - make sure it is by default saved with '0600'
     * access rights.
     */
    pt::json_parser::write_json(configuration_file_path.string(), root);
    boost::filesystem::permissions(configuration_file_path, 
                                      boost::filesystem::owner_read |
                                      boost::filesystem::owner_write);

    /**
     * Save the Swagger specification file from provided path
     * or from stdin (in case the name is equal to '-'.
     *
     * \todo Check that the input is valid json.
     */
    boost::filesystem::ofstream swagger_cache_ofstream{swagger_cache_file_path};


    std::shared_ptr<std::istream> swagger_input;

    if (swagger_specification_file == "-") {
      /**
       * Read the Swagger from the standard input
       */
      swagger_input.reset(&std::cin, [](...){});

      std::cerr << "Reading Swagger JSON from stdin..." << std::endl;
    }
    else {
      /**
       * Try to read the Swagger specification from the provided filename.
       */
      swagger_input.reset(new std::ifstream{swagger_specification_file.c_str()});

      std::cerr << "Reading Swagger JSON from file: " 
                << swagger_specification_file << std::endl;
    }

    std::string s{std::istreambuf_iterator<char>(*swagger_input), {}};

    std::cout << s << std::endl;

    swagger_cache_ofstream << s;
  }


  void service::load(const std::string& folder) {

  }


  void service_manager::initialize() {

    auto swaggerfs_home = boost::filesystem::path{getenv("HOME")}/".swaggerfs";

    /**
     * Create the ~/.swaggerfs directory if it doesn't exist yet
     */
    if(!boost::filesystem::is_directory(swaggerfs_home)) {
      boost::filesystem::create_directory(swaggerfs_home);
    }

  }


  void service_manager::add_service(const commands::add_service &cmd) {

    auto swaggerfs_home = boost::filesystem::path{getenv("HOME")}/".swaggerfs";

    /**
     * Create service instance from command arguments
     */
    service new_service(cmd);
    new_service.save(swaggerfs_home.string());

  }


  std::vector<std::string> service_manager::list_services() {

    auto swaggerfs_home = boost::filesystem::path{getenv("HOME")}/".swaggerfs";
    std::vector<std::string> result;

    boost::filesystem::directory_iterator it{swaggerfs_home};

    /**
     * Generate the list of services based on the list of '*.sfs' files
     * in ~/.swaggerfs folder.
     */
    while(it != boost::filesystem::directory_iterator()) {

        if( boost::filesystem::is_regular_file(*it) 
            && it->path().extension() == ".sfs") 
        {
          result.push_back(it->path().filename().stem().string());
        }
        ++it;

    }
    return result;

  }


  std::shared_ptr<service> service_manager::get_service(
                                                    const std::string& name) {
    std::shared_ptr<service> result;

    auto swaggerfs_home = boost::filesystem::path{getenv("HOME")}/".swaggerfs";
    auto service_definition = swaggerfs_home/(name+".sfs");
    auto service_swagger = swaggerfs_home/(name+".json");

    if(!boost::filesystem::is_regular_file(service_definition)) {
      return result;
    }

    pt::ptree service_definition_root;
    pt::read_json(service_definition.string(), service_definition_root);

    result.reset(new service{});
    result->name = service_definition_root.get<std::string>("name");
    result->endpoint = service_definition_root.get<std::string>("endpoint");
    swagger_parser::parse(result->model, service_swagger.string());

    return result;

  }


  void service_manager::forget_service(std::string name) {

    auto swaggerfs_home = boost::filesystem::path(getenv("HOME"))/".swaggerfs";
    
    boost::filesystem::path sfs_file = swaggerfs_home/(name+".sfs");
    boost::filesystem::path json_file = swaggerfs_home/(name+".json");   

    boost::filesystem::remove(sfs_file);
    boost::filesystem::remove(json_file);
     
  }

}
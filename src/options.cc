#include <memory>
#include <iostream>

#include "options.h"
#include <boost/tokenizer.hpp>
#include <boost/pointer_cast.hpp>

namespace swaggerfs {

void cli_options::print_usage(const po::options_description &global,
                              const po::positional_options_description &pos) {

  std::cout << std::endl << "SwaggerFS\t\t" << "v" << "1.0.0" << std::endl;
  std::cout << std::endl << "Usage: " << "swaggerfs" << " [options]" << std::endl;
  std::cout << "\n\tMount REST services in local filesystem" << std::endl;
  std::cout << global << std::endl;
  std::cout << std::endl;

}

std::shared_ptr<commands::command> cli_options::parse(int argc, char** argv) {
 
  po::options_description global("Global options");
  global.add_options()
      ("help,h", 
        "Print this help message") 
      ("debug,d", 
        "Turn on debug output")
      ("untrusted,u", 
        "Allow connections to untrusted servers")
      ("command", po::value<std::string>(), 
        "command to execute")
      ("subargs", po::value<std::vector<std::string> >(), 
        "Arguments for command");

  po::positional_options_description pos;
  pos.add("command", 1).
      add("subargs", -1);

  po::variables_map vm;

  po::parsed_options parsed = po::command_line_parser(argc, argv).
      options(global).
      positional(pos).
      allow_unregistered().
      run();

  try { 
    po::store(parsed, vm); 

    std::string cmd = vm["command"].as<std::string>();

    /**
     * Handle 'add' command
     */
    if (cmd == "add") {
      po::options_description add_options("Add service");
      add_options.add_options()
          ("help,h", "Prints this help message")
          ("name", "Name of the service instance for the local database")
          ("swagger", "Path or URL to a Swagger JSON specification")
          ("endpoint", po::value<std::string>(), 
             "Server endpoint (e.g. http://petstore.swagger.io/v2)")
          ("basic-auth", po::value<std::string>(), 
             "Username for Basic Auth (e.g. alice:secret)")
          ("api-key", po::value<std::string>(), 
             "ApiKey for Basic Auth (e.g. A1B2C3D4E5) - will be passed in the "
             "header specified in the security Swagger object");

      po::positional_options_description add_positional_options;
      add_positional_options.add("name",    1);
      add_positional_options.add("swagger", 1);
      add_positional_options.add("endpoint",1);

      std::vector<std::string> opts 
        = po::collect_unrecognized(parsed.options, po::include_positional);

      /**
       * Remove global arguments
       */
      opts.erase(opts.begin());

      /**
       * Parse 'add' command specific arguments
       */
      po::store(po::command_line_parser(opts).options(add_options)
        .positional(add_positional_options).allow_unregistered().run(), vm);

      if(vm.count("help"))  {   
        print_usage(add_options, add_positional_options);
        return std::shared_ptr<commands::command>(new commands::noop());
      } 


      auto c 
        = std::shared_ptr<commands::add_service>(new commands::add_service());
      
      c->name = vm["name"].as<std::string>();
      c->swagger_input_source = vm["swagger"].as<std::string>();
      c->endpoint = vm["endpoint"].as<std::string>();

      if(vm.count("basic-auth")) {
        std::string userpass = vm["basic-auth"].as<std::string>();
        boost::tokenizer<boost::char_separator<char> >
          tokens(userpass, boost::char_separator<char>(":"));

        if(tokens.begin() == tokens.end() || ++tokens.begin() == tokens.end()) {
          std::cerr << "Invalid basic-auth format" << std::endl;
          throw "";
        }

        c->username = *tokens.begin();
        c->password = *(++tokens.begin());
      }

      if(vm.count("api-key")) {
        c->api_key = vm["api-key"].as<std::string>();
      }

      return std::dynamic_pointer_cast<commands::command>(c);

    }
    else if(cmd == "forget") {
      po::options_description forget_options("Forget service");
      forget_options.add_options()
          ("help,h", "Prints this help message")
          ("name", "Name of the service instance in the local database");

      po::positional_options_description forget_positional_options;
      forget_positional_options.add("name",    1);

      std::vector<std::string> opts 
        = po::collect_unrecognized(parsed.options, po::include_positional);

      /**
       * Remove global arguments
       */
      opts.erase(opts.begin());

      /**
       * Parse 'forget' command specific arguments
       */
      po::store(po::command_line_parser(opts).options(forget_options)
        .positional(forget_positional_options).allow_unregistered().run(), vm);

      if(vm.count("help"))  {   
        print_usage(forget_options, forget_positional_options);
        return std::shared_ptr<commands::command>(new commands::noop());
      } 

      std::string service_to_forget = vm["name"].as<std::string>();

      return std::shared_ptr<commands::command>(
                    new commands::forget_service(service_to_forget));
    }
    /**
     * Handle 'list' command
     */
    else if(cmd == "list") {
      /**
       * 'list' command does not expect any options
       */
      po::options_description list_options("List services");
      list_options.add_options()
          ("help,h", "Prints this help message");

      po::positional_options_description list_positional_options;
      list_positional_options.add("name",    1);

      std::vector<std::string> opts 
        = po::collect_unrecognized(parsed.options, po::include_positional);

      /**
       * Remove global arguments
       */
      opts.erase(opts.begin());

      /**
       * Parse 'list' command specific arguments
       */
      po::store(po::command_line_parser(opts).options(list_options)
        .allow_unregistered().run(), vm);

      if(vm.count("help"))  {   
        print_usage(list_options, list_positional_options);
        return std::shared_ptr<commands::command>(new commands::noop());
      } 

      return std::shared_ptr<commands::command>(new commands::list_services());
    }
    /**
     * Handle 'mount' command
     */
    else if(cmd == "mount") {
      po::options_description mount_options("Mount service");
      mount_options.add_options()
          ("help,h", "Prints this help message")
          ("name", "Name of the service to be mounted")
          ("path", "Path under which the service should be mounted");

      po::positional_options_description mount_positional_options;
      mount_positional_options.add("name",    1);
      mount_positional_options.add("path",    1);

      std::vector<std::string> opts 
        = po::collect_unrecognized(parsed.options, po::include_positional);

      /**
       * Remove global arguments
       */
      opts.erase(opts.begin());

      /**
       * Parse 'forget' command specific arguments
       */
      po::store(po::command_line_parser(opts).options(mount_options)
        .positional(mount_positional_options).allow_unregistered().run(), vm);

      if(vm.count("help"))  {   
        print_usage(mount_options, mount_positional_options);
        return std::shared_ptr<commands::command>(new commands::noop());
      } 

      std::string service_to_mount = vm["name"].as<std::string>();
      std::string mount_point = vm["path"].as<std::string>();

      return std::shared_ptr<commands::command>(
                    new commands::mount_service(service_to_mount, mount_point));
    }
    else if(cmd == "unmount") {

    }
    else {

      if(vm.count("help"))  {   
        print_usage(global, pos);
        return std::shared_ptr<commands::command>(new commands::noop());
      } 
      else {
        std::cerr << "Unknown command: `" << cmd << "` " << std::endl 
                  << std::endl;
      }
    }

    po::notify(vm);

  } 
  catch(boost::program_options::required_option& e) 
  { 
    throw e; 
  } 
  catch(boost::program_options::error& e) 
  { 
    throw e;  
  } 

  return std::shared_ptr<commands::command>(new commands::noop());

}

}
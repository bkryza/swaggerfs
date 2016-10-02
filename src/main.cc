#include <iostream>
#include <memory>
#include <typeindex>

#include "options.h"
#include "commands.h"
#include "service_manager.h"

using namespace swaggerfs;


void handle_command(service_manager &sm, const commands::command& cmd) {

  if(std::type_index(typeid(cmd)) 
            == std::type_index(typeid(commands::add_service))) {
    /**
     * Add new service to the local database
     */
    sm.add_service(dynamic_cast<const commands::add_service&>(cmd));
  }
  else if(std::type_index(typeid(cmd)) 
              == std::type_index(typeid(commands::forget_service))) {
    /**
     * Remove the service from the local database
     */
    sm.forget_service(dynamic_cast<const commands::forget_service&>(cmd).name);
  }
  else if(std::type_index(typeid(cmd)) 
              == std::type_index(typeid(commands::list_services))) {
    /**
     * Print out all services
     */
    for(auto&& service : sm.list_services()) {
      std::cout << service << std::endl;
    }
  }
  else {
    std::cout << "Unknown command" << std::endl;
  }

}


int main(int argc, char* argv[]) {

  cli_options command_line_input;

  service_manager sm;


  /**
   * Parse the command line to get the 'command' the user requested 
   * and its attributes
   */
  std::shared_ptr<commands::command> cmd = command_line_input.parse(argc, argv);
  
  std::cerr << "Parsing command line..." << std::endl;

  /**
   * If the CLI parser returned 'noop' - do nothing, the command line
   * had errors or requested sth the parser handled on its own
   */
  if(dynamic_cast<commands::noop*>(cmd.get())) {
    std::cerr << "Got empty operation" << std::endl;
    return 0;
  }

  /**
   * If the user requested actual command hand it over to 'service_manager'
   */
  sm.initialize();

  handle_command(sm, *cmd);

  return 0;
}

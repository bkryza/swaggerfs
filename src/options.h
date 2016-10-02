#pragma once

#include <boost/program_options.hpp>

#include "commands.h"

namespace po = boost::program_options;

namespace swaggerfs {
  
  class cli_options {
    
    std::string command_group;

  public:

    std::shared_ptr<commands::command> parse(int argc, char** argv);

    void print_usage(const po::options_description &global,
                     const po::positional_options_description &pos);


  };

}
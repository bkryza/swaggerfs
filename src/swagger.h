#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <list>

#include "http_client.h"
namespace pt = boost::property_tree;


namespace swaggerfs {


  /**
   * This namespace provides a simplified object model of Swagger specification.
   * It does not aim to be complete - only it's subset which is required
   * by this SwaggerFS.
   */
  namespace swagger {

    struct tag {
      std::string name;

      tag() {}
      tag(const pt::ptree &o);
    };

    struct operation {
      std::string id;
      std::string path;
      swaggerfs::http::method method;
      std::string summary;

      operation() {}
      operation(const std::string &path, swaggerfs::http::method m, 
                const pt::ptree &o);
    };

  }


  class swagger_parser {

  public:

    static void parse(pt::ptree &model, const std::string& file_name);

    static std::vector<swagger::tag> get_tags(const pt::ptree &model);
    
    static std::vector<swagger::operation> get_operations(const pt::ptree &model,
                                                   const std::string &tag);

  };
  
}

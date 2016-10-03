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

    static swagger::tag get_tag(const pt::ptree &model, const std::string &tag);

    static bool has_tag(const pt::ptree &model, std::string tag);
    


    static std::vector<swagger::operation> get_operations(const pt::ptree &model,
                                                          const std::string &tag);

    static swagger::operation get_operation(const pt::ptree &model,
                                            const std::string &tag,
                                            const std::string &operation_id);

    static bool has_operation(const pt::ptree &model, 
                              const std::string &tag,
                              const std::string &operation_id);



  };
  
}

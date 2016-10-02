#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <list>

namespace pt = boost::property_tree;


namespace swaggerfs {

  enum class http_method {
    GET,
    POST,
    PATCH,
    PUT,
    DELETE
  };

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
      http_method method;
      std::string summary;

      operation() {}
      operation(const std::string &path, http_method m, const pt::ptree &o);

      static http_method method_from_string(const std::string &m);
    };

  }


  class swagger_parser {

    /**
     * Swagger API model
     */ 
    pt::ptree model;

  public:


    void parse(const std::string& file_name);

    std::vector<swagger::tag> get_tags() const;
    
    std::vector<swagger::operation> get_operations(const std::string &tag) const;

  };
  
}

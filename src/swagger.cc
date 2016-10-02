#include "swagger.h"

#include <string>
#include <vector>
#include <iostream>

namespace swaggerfs {

  namespace swagger {
    tag::tag(const pt::ptree &o) {
      name = o.get<std::string>("name");
    }

    operation::operation(const std::string &path, http_method m, 
                         const pt::ptree &o) 
      :path(path), method(m)
    {
      id = o.get<std::string>("operationId");
      summary = o.get<std::string>("summary");
    }

    http_method operation::method_from_string(const std::string &m) {
      if(m == "get" || m == "GET")
        return http_method::GET;
      else if(m == "post" || m == "POST")
        return http_method::POST;
      else if(m == "patch" || m == "PATCH")
        return http_method::PATCH;      
      else if(m == "put" || m == "PUT")
        return http_method::PUT;
      else if(m == "delete" || m == "DELETE")
        return http_method::DELETE;
      else 
        return http_method::GET;
    }
  }

  void swagger_parser::parse(const std::string& file_name) {

    /**
     * Load the json file in this ptree
     */
    pt::read_json(file_name, model);

  }


  std::vector<swagger::tag> swagger_parser::get_tags() const {

    std::vector<swagger::tag> result;

    for(const pt::ptree::value_type &tag_object : model.get_child("tags")) {

      result.push_back(swagger::tag{tag_object.second});

    }

    return result;

  }


  std::vector<swagger::operation> 
  swagger_parser::get_operations(const std::string &tag) const {

    std::vector<swagger::operation> result;

    for(const pt::ptree::value_type &path_object : model.get_child("paths")) {

      std::string path = path_object.first;
      pt::ptree operations = path_object.second;

      for(const pt::ptree::value_type &operation_object : operations) {

        std::string method = operation_object.first;
        pt::ptree operation = operation_object.second;

        /**
         * Check if 'tags' list contains requested tag
         */
        bool operation_has_tag = false;
        for(const pt::ptree::value_type &tag_object
                                              : operation.get_child("tags")) {

          if(tag == tag_object.second.get_value<std::string>())
            operation_has_tag = true;
        }

        if(!operation_has_tag)
          continue;
        
        /**
         * Build operation instance from the property tree
         */
        http_method hm = swagger::operation::method_from_string(method);
        result.push_back(swagger::operation{path, hm, operation});

      }

    }

    return result;

  }


}
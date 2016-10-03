#include "swagger.h"

#include <string>
#include <vector>
#include <iostream>

#include "http_client.h"

namespace swaggerfs {

  namespace swagger {
    tag::tag(const pt::ptree &o) {
      name = o.get<std::string>("name");
    }

    operation::operation(const std::string &path, swaggerfs::http::method m, 
                         const pt::ptree &o) 
      :path(path), method(m)
    {
      id = o.get<std::string>("operationId");
      summary = o.get<std::string>("summary");
    }

  }

  void swagger_parser::parse(pt::ptree &model, const std::string& file_name) {

    /**
     * Load the json file in this ptree
     */
    pt::read_json(file_name, model);

  }


  std::vector<swagger::tag> swagger_parser::get_tags(const pt::ptree &model) {

    std::vector<swagger::tag> result;
    for(const pt::ptree::value_type &tag_object : model.get_child("tags")) {
      result.push_back(swagger::tag{tag_object.second});
    }
    return result;

  }


  swagger::tag get_tag(const pt::ptree &model, const std::string &tag) {

    swagger::tag result;
    for(const pt::ptree::value_type &tag_object : model.get_child("tags")) {
      result = swagger::tag{tag_object.second};
      if(result.name == tag)
        break;
    }
    return result;
  }


  bool has_tag(const pt::ptree &model, std::string tag) {
    std::vector<swagger::tag> tags = swagger_parser::get_tags(model);

    return std::any_of(tags.begin(), tags.end(), 
              [&](swagger::tag t){ return t.name == tag; });

  }
    


  std::vector<swagger::operation> 
  swagger_parser::get_operations(const pt::ptree &model, const std::string &tag) {

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
        swaggerfs::http::method http_method
          = swaggerfs::http::method_from_string(method);
        result.push_back(swagger::operation{path, http_method, operation});

      }

    }

    return result;

  }

  swagger::operation 
  swagger_parser::get_operation(const pt::ptree &model, const std::string &tag,
                                const std::string &operation_id) {

    std::vector<swagger::operation> operations 
      = swagger_parser::get_operations(model, tag);

    for(auto operation : operations) {
      if(operation.id == operation_id)
        return operation;
    }

    return {};
  }

  bool swagger_parser::has_operation(const pt::ptree &model, 
                                     const std::string &tag,
                                     const std::string &operation_id) {

    std::vector<swagger::operation> operations 
      = swagger_parser::get_operations(model, tag);

    return std::any_of(operations.begin(), operations.end(), 
              [&](swagger::operation o){ return o.id == operation_id; });

  }


  /*

   */

}
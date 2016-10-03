#pragma once 

#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#include <boost/asio.hpp>


namespace swaggerfs {

  namespace http {

    enum class method {
      GET,
      POST,
      PATCH,
      PUT,
      DELETE
    };

    method method_from_string(const std::string &m);

    std::string method_to_string(swaggerfs::http::method m);

    struct request {
      method method;
      std::string host;
      std::string path;
      std::string body;
      std::vector<std::pair<std::string, std::string> > headers;
    };

    struct response {
      int status_code;
      std::string body;

      std::vector<std::pair<std::string, std::string> > headers;
    };

    class client {
    public:
      void make_request(const request &req, response &res);
    };

  }

}
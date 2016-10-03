#include "http_client.h"

#include <utility>
#include <string>
#include <vector>
#include <functional>
#include <istream>
#include <algorithm>

#include <boost/algorithm/string.hpp>

using boost::asio::ip::tcp;


namespace swaggerfs {

  namespace http {

    method method_from_string(const std::string &m) {
      if(m == "get" || m == "GET") return method::GET;
      else if(m == "post" || m == "POST") return method::POST;
      else if(m == "patch" || m == "PATCH") return method::PATCH;      
      else if(m == "put" || m == "PUT") return method::PUT;
      else if(m == "delete" || m == "DELETE") return method::DELETE;
      else return method::GET;
    }

    std::string method_to_string(swaggerfs::http::method m) {
      switch(m) {
        case method::GET: return "GET";
        case method::POST: return "POST";
        case method::PUT: return "PUT";
        case method::PATCH: return "PATCH";
        case method::DELETE: return "DELETE";
      }
    }

    void client::make_request(const request &req, response &res) {

      boost::asio::io_service io_service;

      res.status_code = 0;

      /**
       * Get a list of endpoints corresponding to the server name.
       */
      tcp::resolver resolver(io_service);
      tcp::resolver::query query(req.host, "http");
      tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

      /**
       * Try each endpoint until we successfully establish a connection.
       */
      tcp::socket socket(io_service);
      boost::asio::connect(socket, endpoint_iterator);

      /**
       * Form the request. We specify the "Connection: close" header so that the
       * server will close the socket after transmitting the response. This will
       * allow us to treat all data up until the EOF as the content.
       */
      boost::asio::streambuf request;
      std::ostream request_stream(&request);
      request_stream << "GET " << req.path << " HTTP/1.0\r\n";
      request_stream << "Host: " << req.host << "\r\n";
      request_stream << "Accept: */*\r\n";
      request_stream << "Connection: close\r\n\r\n";

      /**
       * Send the request.
       */
      boost::asio::write(socket, request);

      /**
       * Read the response status line. The response streambuf will automatically
       * grow to accommodate the entire line. The growth may be limited by passing
       * a maximum size to the streambuf constructor.
       */
      boost::asio::streambuf response;
      boost::asio::read_until(socket, response, "\r\n");

      /**
       * Check that response is OK.
       */
      std::istream response_stream(&response);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);

      if(!response_stream || http_version.substr(0, 5) != "HTTP/") {
        std::cout << "Invalid response\n";
        return;
      }
      
      if(status_code != 200) {
        std::cout << "Response returned with status code " << status_code << "\n";
        res.status_code = status_code;
        return;
      }

      /**
       * Read the response headers, which are terminated by a blank line.
       */
      boost::asio::read_until(socket, response, "\r\n\r\n");

      /**
       * Process the response headers.
       */
      std::string header;
      std::vector<std::string> header_tokens;

      while (std::getline(response_stream, header) && header != "\r") {
        header_tokens.clear();
        boost::split(header_tokens, header, boost::is_any_of(":"));
        res.headers.push_back(
              std::make_pair(header_tokens[0], boost::trim_copy(header_tokens[1])));
      }

      /**
       * Write whatever content we already have to output.
       */
      if (response.size() > 0) {
         std::istream(&response) >> res.body;
      }

      /**
       * Read until EOF, writing data to output as we go.
       */
      boost::system::error_code error;
      while(boost::asio::read(socket, response,
            boost::asio::transfer_at_least(1), error)) {

        std::istream(&response) >> res.body;
      }

      if (error != boost::asio::error::eof)
        throw boost::system::system_error(error);


    }

  }

}
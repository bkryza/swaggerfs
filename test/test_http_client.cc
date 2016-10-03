#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>

#include "swagger.h"
#include "http_client.h"

using namespace swaggerfs;

const std::string petstore_post_body = 
  "{\n"
  "  \"id\": 37567,\n"
  "  \"category\": {\n"
  "    \"id\": 0,\n"
  "    \"name\": \"string\"\n"
  "  },\n"
  "  \"name\": \"doggie\",\n"
  "  \"photoUrls\": [\n"
  "    \"string\"\n"
  "  ],\n"
  "  \"tags\": [\n"
  "    {\n"
  "      \"id\": 0,\n"
  "      \"name\": \"string\"\n"
  "    }\n"
  "  ],\n"
  "  \"status\": \"available\"\n"
  "}";

constexpr auto PETSTORE_SERVICE_HOST = "petstore.swagger.io";


BOOST_AUTO_TEST_SUITE(http_client)


BOOST_AUTO_TEST_CASE(http_client_post_pet)
{

  http::client client;

  http::request req;
  req.method = http::method::PUT;
  req.host = PETSTORE_SERVICE_HOST;
  req.path = "/v2/pet";
  req.body = petstore_post_body;
  req.headers.push_back(std::make_pair("Content-type", "application/json"));
  req.headers.push_back(
    std::make_pair("Content-length", 
          boost::lexical_cast<std::string>(petstore_post_body.size())));

  http::response res;
  client.make_request(req, res);


  std::cout << "\n============\n"
            << res.body
            << "\n============\n";
  /**
   * Check status code
   */
  BOOST_CHECK_EQUAL(200, res.status_code);

}


BOOST_AUTO_TEST_CASE(http_client_put_pet)
{

  http::client client;

  http::request req;
  req.method = http::method::PUT;
  req.host = PETSTORE_SERVICE_HOST;
  req.path = "/v2/pet";
  req.body = petstore_post_body;
  req.headers.push_back(std::make_pair("Content-type", "application/json"));
  req.headers.push_back(
    std::make_pair("Content-length", 
          boost::lexical_cast<std::string>(petstore_post_body.size())));

  http::response res;
  client.make_request(req, res);


  std::cout << "\n============\n"
            << res.body
            << "\n============\n";
  /**
   * Check status code
   */
  BOOST_CHECK_EQUAL(200, res.status_code);

}


BOOST_AUTO_TEST_CASE(http_client_get_json)
{

  http::client client;

  http::request req;
  req.method = http::method::GET;
  req.host = PETSTORE_SERVICE_HOST;
  req.path = "/v2/pet/37567";
  req.body = "";
  req.headers.push_back(std::make_pair("Accept", "application/json"));

  http::response res;
  client.make_request(req, res);


  std::cout << "\n============\n"
            << res.body
            << "\n============\n";
  /**
   * Check status code
   */
  BOOST_CHECK_EQUAL(200, res.status_code);

  /**
   * Check content type
   */
  std::string content_type;
  for(auto header : res.headers) {
    if(boost::iequals("content-type", header.first)) {
      content_type = header.second;
      break;
    }
  }

  BOOST_CHECK_EQUAL("application/json", content_type);

  /**
   * Check response
   */
  std::istringstream response_body_stream{res.body};
  boost::property_tree::ptree response_root;

  boost::property_tree::json_parser::read_json(response_body_stream, 
                                               response_root);

  BOOST_CHECK_EQUAL(37567, response_root.get("id", 0));

}


BOOST_AUTO_TEST_CASE(http_client_get_xml)
{

  http::client client;

  http::request req;
  req.method = http::method::GET;
  req.host = PETSTORE_SERVICE_HOST;
  req.path = "/v2/pet/37567";
  req.body = "";
  req.headers.push_back(std::make_pair("Accept", "application/xml"));

  http::response res;
  client.make_request(req, res);


  std::cout << "\n============\n"
            << res.body
            << "\n============\n";
  /**
   * Check status code
   */
  BOOST_CHECK_EQUAL(200, res.status_code);

  /**
   * Check content type
   */
  std::string content_type;
  for(auto header : res.headers) {
    if(boost::iequals("content-type", header.first)) {
      content_type = header.second;
      break;
    }
  }

  BOOST_CHECK_EQUAL("application/xml", content_type);

  /**
   * Check response
   */
  std::istringstream response_body_stream{res.body};
  boost::property_tree::ptree response_root;

  boost::property_tree::xml_parser::read_xml(response_body_stream, 
                                             response_root);

  BOOST_CHECK_EQUAL(37567, response_root.get("Pet.id", 0));

}


BOOST_AUTO_TEST_CASE(http_client_delete_pet)
{

  http::client client;

  http::request req;
  req.method = http::method::DELETE;
  req.host = PETSTORE_SERVICE_HOST;
  req.path = "/v2/pet/37567";
  req.body = "";

  http::response res;
  client.make_request(req, res);


  std::cout << "\n============\n"
            << res.body
            << "\n============\n";
  /**
   * Check status code
   */
  BOOST_CHECK_EQUAL(200, res.status_code);

}


BOOST_AUTO_TEST_SUITE_END()
  
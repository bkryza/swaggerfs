#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>


#include "swagger.h"
#include "http_client.h"

using namespace swaggerfs;


BOOST_AUTO_TEST_SUITE(http_client)

BOOST_AUTO_TEST_CASE(http_client_get)
{
  http::client client;

  http::request req;
  req.method = http::method::GET;
  req.host = "petstore.swagger.io";
  req.path = "/v2/pet/123";
  req.body = "";
  req.headers.push_back(std::make_pair("Accept", "application/json"));

  http::response res;
  client.make_request(req, res);

  BOOST_CHECK_EQUAL(200, res.status_code);

}

BOOST_AUTO_TEST_SUITE_END()
  
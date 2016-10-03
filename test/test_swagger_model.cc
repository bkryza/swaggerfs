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

using namespace swaggerfs;

BOOST_AUTO_TEST_SUITE(swagger_model)


BOOST_AUTO_TEST_CASE(swagger_model_parser)
{
  
    pt::ptree model;

    swagger_parser::parse(model, "./resources/petstore.json");

    auto tags = swagger_parser::get_tags(model);

    BOOST_CHECK_EQUAL(3, tags.size());

    auto pet_operations = swagger_parser::get_operations(model, "pet");

    BOOST_CHECK_EQUAL(8, pet_operations.size());
  
}

BOOST_AUTO_TEST_CASE(fuse_paths_parsing)
{
    pt::ptree model;

    swagger_parser::parse(model, "./resources/petstore.json");
    auto tags = swagger_parser::get_tags(model);

    std::string path{"/user"};
    std::vector<std::string> path_tokens;
    boost::split(path_tokens, path, boost::is_any_of("/"));

    std::string tag_name = "user";

    BOOST_CHECK_EQUAL(2, path_tokens.size());

    BOOST_CHECK(std::any_of(tags.begin(), tags.end(), 
                    [&](swagger::tag t){ return t.name == path_tokens[1]; }));

}

BOOST_AUTO_TEST_SUITE_END()
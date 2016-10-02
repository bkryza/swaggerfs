#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "swagger.h"

BOOST_AUTO_TEST_SUITE(swagger_model)


BOOST_AUTO_TEST_CASE(swagger_model_parser)
{
  
    auto petstore_model = swaggerfs::swagger_parser{};

    petstore_model.parse("./resources/petstore.json");

    auto tags = petstore_model.get_tags();

    BOOST_CHECK_EQUAL(3, tags.size());

    auto pet_operations = petstore_model.get_operations("pet");

    BOOST_CHECK_EQUAL(8, pet_operations.size());
  
}


BOOST_AUTO_TEST_SUITE_END()
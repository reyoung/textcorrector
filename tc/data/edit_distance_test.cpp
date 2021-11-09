#include "catch2/catch.hpp"
#include "tc/data/edit_distance.h"

namespace tc::data {
TEST_CASE("edit_distance") {
  REQUIRE(EditDistance(std::string("abc"), std::string("abd")) == 1);
}
}// namespace tc::data
#include "catch2/catch.hpp"
#include "tc/base/except.h"
#include <iostream>
TEST_CASE("tc_base_except") {
  try {
    TC_ENFORCE(false) << "test enforce failed";
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}

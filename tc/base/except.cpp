#include "tc/base/except.h"
namespace tc::base {

const char *Exception::tag() const {
  return "tc";
}
std::string Exception::error() const {
  return error_;
}
}// namespace tc::base

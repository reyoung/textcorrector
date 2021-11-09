#pragma once
#include "exceptxx/BaseException.h"
#include "exceptxx/BaseExceptionImpl.h"
#include "exceptxx/ThrowHelper.h"
namespace tc::base {

class Exception : public exceptxx::BaseExceptionImpl<Exception> {
 public:
  using Error = std::string;

  Exception(Error error, const char *func, size_t line, std::string &&message)
      : BaseExceptionImpl(func, line, std::move(message)), error_(std::move(error)) {}
  const char *tag() const override;
  std::string error() const override;

 private:
  Error error_;
};

}// namespace tc::base

#define TC_LIKELY(x) __builtin_expect(!!(x), 1)
#define TC_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define TC_ENFORCE(cond)    \
  if (TC_UNLIKELY(!(cond))) \
  exceptxx::ThrowHelper<::tc::base::Exception>(#cond, EXCEPTXX_GET_FUNC(), __LINE__) <<= std::ostringstream()

#define TC_THROW()                                                                                   \
  exceptxx::ThrowHelper<::tc::base::Exception>("Throw exception", EXCEPTXX_GET_FUNC(), __LINE__) <<= \
      std::ostringstream()
#pragma once
#include "stdint.h"

namespace tc::data {
enum class TreeAttributes : uint32_t {
  TA_SUPPORT_LOAD_MMAP = 1
};
}
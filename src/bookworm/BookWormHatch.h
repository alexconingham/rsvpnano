#pragma once

#include <stdint.h>

#if defined(ESP_PLATFORM)
#include "esp_random.h"
#endif

#include "bookworm/BookWormState.h"

namespace bookworm {

typedef uint32_t (*RandomU32Fn)();

inline uint32_t defaultRandomU32() {
#if defined(ESP_PLATFORM)
  return esp_random();
#else
  return static_cast<uint32_t>(rand());
#endif
}

void hatchEgg(BookWormState &s, uint32_t nowMsMonotonic, uint32_t hatchedAtUtc = 0,
              RandomU32Fn rnd = defaultRandomU32);

}  // namespace bookworm

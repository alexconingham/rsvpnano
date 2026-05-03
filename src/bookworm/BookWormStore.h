#pragma once

#include <Preferences.h>

#include "bookworm/BookWormState.h"

namespace bookworm {

class BookWormStore {
 public:
  void begin();
  bool loadInto(BookWormState &s);
  void save(const BookWormState &s);
  void ensureHatched(BookWormState &s, uint32_t nowMsMonotonic);

 private:
  Preferences prefs_;
};

}  // namespace bookworm

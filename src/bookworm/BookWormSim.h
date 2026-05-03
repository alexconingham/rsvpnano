#pragma once

#include <stdint.h>

#include "bookworm/BookWormState.h"

namespace bookworm {

class BookWormSim {
 public:
  /// Apply real-time decay / passive drift using wall-clock delta (clamped).
  static void tick(BookWormState &s, uint32_t deltaMs);

  /// Reward for words read while actively playing RSVP (typically state Playing).
  /// When applyNeedsEffect is false (e.g. pet hibernating), words still count for evolution but
  /// hunger/boredom are unchanged.
  static void onReadingWords(BookWormState &s, uint32_t wordsAdvanced, bool applyNeedsEffect = true);

  static void deskFeed(BookWormState &s);
  static void deskPlay(BookWormState &s);
  static void deskPet(BookWormState &s);

  static void syncEvolution(BookWormState &s);
};

}  // namespace bookworm

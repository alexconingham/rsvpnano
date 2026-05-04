#pragma once

#include <stdint.h>

#include "bookworm/BookWormState.h"

namespace bookworm {

class BookWormSim {
 public:
  /// Apply real-time decay / passive drift using wall-clock delta (clamped).
  static void tick(BookWormState &s, uint32_t deltaMs);

  /// Reward for words read while actively playing RSVP (typically state Playing).
  /// When applyNeedsEffect is false (e.g. pet hibernating), hunger/boredom are unchanged.
  /// When syncEvolutionStage is false, totalWordsRead still increases but evolution stage is not
  /// updated (Settings: turn growth off).
  static void onReadingWords(BookWormState &s, uint32_t wordsAdvanced, bool applyNeedsEffect = true,
                             bool syncEvolutionStage = true);

  static void deskFeed(BookWormState &s);
  static void deskPlay(BookWormState &s);
  static void deskPet(BookWormState &s);
  static void deskBoop(BookWormState &s);

  static void syncEvolution(BookWormState &s);
};

}  // namespace bookworm

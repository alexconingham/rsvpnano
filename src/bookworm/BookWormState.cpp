#include "bookworm/BookWormState.h"

#include "bookworm/BookWormConfig.h"

namespace bookworm {

void clampNeeds(BookWormState &s) {
  if (s.hunger > kNeedMax) {
    s.hunger = kNeedMax;
  }
  if (s.boredom > kNeedMax) {
    s.boredom = kNeedMax;
  }
}

uint8_t computeEvolutionStage(uint32_t totalWordsRead) {
  if (totalWordsRead >= kEvolveStage3Words) {
    return 3;
  }
  if (totalWordsRead >= kEvolveStage2Words) {
    return 2;
  }
  if (totalWordsRead >= kEvolveStage1Words) {
    return 1;
  }
  return 0;
}

}  // namespace bookworm

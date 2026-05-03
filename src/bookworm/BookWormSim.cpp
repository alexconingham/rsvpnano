#include "bookworm/BookWormSim.h"

#include <algorithm>

#include "bookworm/BookWormConfig.h"

namespace bookworm {

void BookWormSim::tick(BookWormState &s, uint32_t deltaMs) {
  if (!s.hatched || deltaMs == 0) {
    return;
  }
  constexpr uint32_t kMaxTickMs = 300000;
  if (deltaMs > kMaxTickMs) {
    deltaMs = kMaxTickMs;
  }
  const uint32_t hungerScale = static_cast<uint32_t>(s.hunger);
  const uint32_t boredomScale = static_cast<uint32_t>(s.boredom);
  const uint32_t hungerDelta =
      static_cast<uint32_t>((static_cast<uint64_t>(kHungerRisePerHour) * hungerScale * deltaMs) /
                            (3600000ULL * kNeedMax));
  const uint32_t boredomDelta =
      static_cast<uint32_t>((static_cast<uint64_t>(kBoredomRisePerHour) * boredomScale * deltaMs) /
                            (3600000ULL * kNeedMax));
  s.hunger = static_cast<uint16_t>(std::min<uint32_t>(s.hunger + hungerDelta, kNeedMax));
  s.boredom = static_cast<uint16_t>(std::min<uint32_t>(s.boredom + boredomDelta, kNeedMax));
}

void BookWormSim::onReadingWords(BookWormState &s, uint32_t wordsAdvanced) {
  if (!s.hatched || wordsAdvanced == 0) {
    return;
  }
  const uint32_t hLoss = static_cast<uint32_t>(kReadingHungerHealPermille) * wordsAdvanced;
  const uint32_t bLoss = static_cast<uint32_t>(kReadingBoredomHealPermille) * wordsAdvanced;
  if (s.hunger > hLoss) {
    s.hunger = static_cast<uint16_t>(s.hunger - hLoss);
  } else {
    s.hunger = 0;
  }
  if (s.boredom > bLoss) {
    s.boredom = static_cast<uint16_t>(s.boredom - bLoss);
  } else {
    s.boredom = 0;
  }
  s.totalWordsRead += wordsAdvanced;
  syncEvolution(s);
}

void BookWormSim::deskFeed(BookWormState &s) {
  if (!s.hatched) {
    return;
  }
  if (s.hunger > kDeskFeedHunger) {
    s.hunger = static_cast<uint16_t>(s.hunger - kDeskFeedHunger);
  } else {
    s.hunger = 0;
  }
}

void BookWormSim::deskPlay(BookWormState &s) {
  if (!s.hatched) {
    return;
  }
  if (s.boredom > kDeskPlayBoredom) {
    s.boredom = static_cast<uint16_t>(s.boredom - kDeskPlayBoredom);
  } else {
    s.boredom = 0;
  }
}

void BookWormSim::deskPet(BookWormState &s) {
  if (!s.hatched) {
    return;
  }
  if (s.hunger > kDeskPetBoth) {
    s.hunger = static_cast<uint16_t>(s.hunger - kDeskPetBoth);
  } else {
    s.hunger = 0;
  }
  if (s.boredom > kDeskPetBoth) {
    s.boredom = static_cast<uint16_t>(s.boredom - kDeskPetBoth);
  } else {
    s.boredom = 0;
  }
}

void BookWormSim::syncEvolution(BookWormState &s) {
  s.evolutionStage = computeEvolutionStage(s.totalWordsRead);
}

}  // namespace bookworm

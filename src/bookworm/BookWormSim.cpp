#include "bookworm/BookWormSim.h"

#include <algorithm>

#include "bookworm/BookWormConfig.h"

namespace bookworm {

namespace {

void relieveSickFromCare(BookWormState &s) {
  if (s.sickAccumMs <= kSickCareReliefMs) {
    s.sickAccumMs = 0;
  } else {
    s.sickAccumMs -= kSickCareReliefMs;
  }
}

void bumpCareDesk(BookWormState &s) {
  uint32_t c = s.careScorePermille;
  c = (c * 6u + 980u) / 7u;
  if (c > 1000u) {
    c = 1000u;
  }
  s.careScorePermille = static_cast<uint16_t>(c);
}

}  // namespace

void BookWormSim::tick(BookWormState &s, uint32_t deltaMs) {
  if (!s.hatched || deltaMs == 0) {
    return;
  }
  constexpr uint32_t kMaxTickMs = 300000;
  if (deltaMs > kMaxTickMs) {
    deltaMs = kMaxTickMs;
  }

  if (s.hunger > 700 && s.boredom > 700) {
    s.sickAccumMs = std::min(s.sickAccumMs + deltaMs, kSickAccumCapMs);
  } else if (s.hunger < 550 || s.boredom < 550) {
    const uint64_t dec = static_cast<uint64_t>(deltaMs) * 3ULL;
    if (dec >= s.sickAccumMs) {
      s.sickAccumMs = 0;
    } else {
      s.sickAccumMs -= static_cast<uint32_t>(dec);
    }
  }

  const uint32_t hungerScale = static_cast<uint32_t>(s.hunger);
  const uint32_t boredomScale = static_cast<uint32_t>(s.boredom);

  uint32_t hungerMult = 100u;
  if (s.overfullTicks > 0) {
    hungerMult = 138u;
    const uint32_t drop = std::max(1u, deltaMs / 400u);
    if (drop >= s.overfullTicks) {
      s.overfullTicks = 0;
    } else {
      s.overfullTicks = static_cast<uint16_t>(s.overfullTicks - static_cast<uint16_t>(drop));
    }
  }

  const uint32_t hungerDelta = static_cast<uint32_t>(
      (static_cast<uint64_t>(kHungerRisePerHour) * hungerScale * deltaMs * hungerMult) /
      (3600000ULL * kNeedMax * 100ULL));
  const uint32_t boredomDelta =
      static_cast<uint32_t>((static_cast<uint64_t>(kBoredomRisePerHour) * boredomScale * deltaMs) /
                            (3600000ULL * kNeedMax));
  s.hunger = static_cast<uint16_t>(std::min<uint32_t>(s.hunger + hungerDelta, kNeedMax));
  s.boredom = static_cast<uint16_t>(std::min<uint32_t>(s.boredom + boredomDelta, kNeedMax));
}

void BookWormSim::onReadingWords(BookWormState &s, uint32_t wordsAdvanced, bool applyNeedsEffect,
                                 bool syncEvolutionStage) {
  if (!s.hatched || wordsAdvanced == 0) {
    return;
  }
  if (applyNeedsEffect) {
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
    uint32_t c = s.careScorePermille;
    c = (c * 19u + 760u) / 20u;
    if (c > 1000u) {
      c = 1000u;
    }
    s.careScorePermille = static_cast<uint16_t>(c);
  }
  s.totalWordsRead += wordsAdvanced;
  if (syncEvolutionStage) {
    syncEvolution(s);
  }
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
  relieveSickFromCare(s);
  bumpCareDesk(s);
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
  relieveSickFromCare(s);
  bumpCareDesk(s);
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
  relieveSickFromCare(s);
  bumpCareDesk(s);
}

void BookWormSim::deskBoop(BookWormState &s) {
  if (!s.hatched) {
    return;
  }
  if (s.hunger > kDeskBoopHunger) {
    s.hunger = static_cast<uint16_t>(s.hunger - kDeskBoopHunger);
  } else {
    s.hunger = 0;
  }
  if (s.boredom > kDeskBoopBoredom) {
    s.boredom = static_cast<uint16_t>(s.boredom - kDeskBoopBoredom);
  } else {
    s.boredom = 0;
  }
  relieveSickFromCare(s);
  bumpCareDesk(s);
}

void BookWormSim::syncEvolution(BookWormState &s) {
  s.evolutionStage = computeEvolutionStage(s.totalWordsRead);
}

}  // namespace bookworm

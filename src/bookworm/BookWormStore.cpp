#include "bookworm/BookWormStore.h"

#include <cstring>

#include "bookworm/BookWormConfig.h"
#include "bookworm/BookWormHatch.h"
#include "bookworm/BookWormSim.h"
#include "time/TimeService.h"

namespace bookworm {

namespace {

constexpr const char *kNs = "bworm";

}  // namespace

void BookWormStore::begin() { prefs_.begin(kNs, false); }

bool BookWormStore::loadInto(BookWormState &s) {
  if (!prefs_.isKey("magic")) {
    return false;
  }
  s.magic = prefs_.getUInt("magic", 0);
  if (s.magic != kStateMagic) {
    return false;
  }
  s.version = prefs_.getUChar("ver", 1);
  s.hatched = prefs_.getBool("hatched", false);
  {
    String n = prefs_.getString("name", "");
    std::strncpy(s.name, n.c_str(), sizeof(s.name) - 1);
    s.name[sizeof(s.name) - 1] = '\0';
  }
  s.styleId = prefs_.getUChar("style", 0);
  s.evolutionStage = prefs_.getUChar("evo", 0);
  s.hunger = static_cast<uint16_t>(prefs_.getUShort("hunger", 200));
  s.boredom = static_cast<uint16_t>(prefs_.getUShort("bored", 200));
  s.totalWordsRead = prefs_.getUInt("words", 0);
  s.totalReadMs = prefs_.getUInt("r_ms", 0);
  s.hatchedAtUtc = prefs_.getUInt("h_utc", 0);
  s.lastTickMs = prefs_.getUInt("lst", 0);
  s.sickAccumMs = prefs_.getUInt("sick", 0);
  s.overfullTicks = static_cast<uint16_t>(prefs_.getUShort("ovf", 0));
  s.careScorePermille = static_cast<uint16_t>(prefs_.getUShort("care", 0));
  s.xpBoostTicks = prefs_.getUChar("xpb", 0);
  clampNeeds(s);
  // Do NOT auto-advance evolutionStage here — evolution requires a tap on the companion
  // when the XP bar fills. Stored stage is the user-confirmed stage.
  return true;
}

void BookWormStore::save(const BookWormState &s) {
  prefs_.putUInt("magic", kStateMagic);
  prefs_.putUChar("ver", s.version);
  prefs_.putBool("hatched", s.hatched);
  prefs_.putString("name", s.name);
  prefs_.putUChar("style", s.styleId);
  prefs_.putUChar("evo", s.evolutionStage);
  prefs_.putUShort("hunger", s.hunger);
  prefs_.putUShort("bored", s.boredom);
  prefs_.putUInt("words", s.totalWordsRead);
  prefs_.putUInt("r_ms", s.totalReadMs);
  prefs_.putUInt("h_utc", s.hatchedAtUtc);
  prefs_.putUInt("lst", s.lastTickMs);
  prefs_.putUInt("sick", s.sickAccumMs);
  prefs_.putUShort("ovf", s.overfullTicks);
  prefs_.putUShort("care", s.careScorePermille);
  prefs_.putUChar("xpb", s.xpBoostTicks);
}

void BookWormStore::ensureHatched(BookWormState &s, uint32_t nowMsMonotonic) {
  if (s.hatched) {
    return;
  }
  hatchEgg(s, nowMsMonotonic, TimeService::utcUnixSeconds());
  save(s);
}

}  // namespace bookworm

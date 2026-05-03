#include "bookworm/BookWormStore.h"

#include <cstring>

#include "bookworm/BookWormConfig.h"
#include "bookworm/BookWormHatch.h"
#include "bookworm/BookWormSim.h"

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
  clampNeeds(s);
  BookWormSim::syncEvolution(s);
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
}

void BookWormStore::ensureHatched(BookWormState &s, uint32_t nowMsMonotonic) {
  if (s.hatched) {
    return;
  }
  hatchEgg(s, nowMsMonotonic);
  save(s);
}

}  // namespace bookworm

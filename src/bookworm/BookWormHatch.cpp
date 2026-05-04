#include "bookworm/BookWormHatch.h"

#include <cstring>

#include "bookworm/BookWormConfig.h"
#include "bookworm/BookWormState.h"

namespace bookworm {

namespace {

const char *const kSyl1[] = {"gl", "br", "sn", "w", "scr", "fl", "tr", "cr", "pl", "dr"};
const char *const midSyl[] = {"ibble", "orm", "uff", "ee", "uzz", "onk", "oop", "ax", "ip", "un"};
const char *const kEndSyl[] = {"wick", "bot", "worm", "kin", "let", "mo", "pu", "zo", "bit", "nip"};

}  // namespace

void hatchEgg(BookWormState &s, uint32_t nowMsMonotonic, uint32_t hatchedAtUtc, RandomU32Fn rnd) {
  std::memset(&s, 0, sizeof(s));
  s.magic = kStateMagic;
  s.version = 1;
  s.hatched = true;
  s.hunger = 180;
  s.boredom = 180;
  s.lastTickMs = nowMsMonotonic;
  s.hatchedAtUtc = hatchedAtUtc;

  const uint32_t r1 = rnd();
  const uint32_t r2 = rnd();
  const uint32_t r3 = rnd();
  const char *a = kSyl1[r1 % (sizeof(kSyl1) / sizeof(kSyl1[0]))];
  const char *b = midSyl[r2 % (sizeof(midSyl) / sizeof(midSyl[0]))];
  const char *c = kEndSyl[r3 % (sizeof(kEndSyl) / sizeof(kEndSyl[0]))];
  std::snprintf(s.name, sizeof(s.name), "%s%s%s", a, b, c);
  s.styleId = static_cast<uint8_t>(rnd() % kStyleCount);
  s.evolutionStage = 0;
  s.careScorePermille = 520;
}

}  // namespace bookworm

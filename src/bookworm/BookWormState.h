#pragma once

#include <stdint.h>

#include <Arduino.h>

namespace bookworm {

struct BookWormState {
  uint32_t magic = 0;
  uint8_t version = 1;
  bool hatched = false;
  char name[24] = "";
  uint8_t styleId = 0;
  uint8_t evolutionStage = 0;
  uint16_t hunger = 200;
  uint16_t boredom = 200;
  uint32_t totalWordsRead = 0;
  uint32_t totalReadMs = 0;
  uint32_t hatchedAtUtc = 0;
  uint32_t lastTickMs = 0;
  /// Time spent with both needs very high (sick streak).
  uint32_t sickAccumMs = 0;
  /// Gorged debuff: hunger rises faster until ticks expire.
  uint16_t overfullTicks = 0;
  /// XP bar (0–1000, wraps); filled by reading words, rate boosted by desk actions.
  uint16_t careScorePermille = 0;
  /// Remaining sim ticks of XP boost from desk actions (1 tick = 500 ms).
  uint8_t xpBoostTicks = 0;
};

void clampNeeds(BookWormState &s);

uint8_t computeEvolutionStage(uint32_t totalWordsRead);

}  // namespace bookworm

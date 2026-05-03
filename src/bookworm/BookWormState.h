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
};

void clampNeeds(BookWormState &s);

uint8_t computeEvolutionStage(uint32_t totalWordsRead);

}  // namespace bookworm

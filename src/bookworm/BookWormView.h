#pragma once

#include <Arduino.h>

#include "bookworm/BookWormState.h"

namespace bookworm {

struct BookWormView {
  String name;
  uint8_t styleId = 0;
  uint8_t evolutionStage = 0;
  uint16_t hungerPermille = 0;
  uint16_t boredomPermille = 0;
  String moodLine;
  bool flashDeskAction = false;
};

}  // namespace bookworm

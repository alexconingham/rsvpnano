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
  bool hibernate = false;
  /// Wall-clock ms for eye blink / pupil wander (pass 0 for static preview).
  uint32_t companionAnimMs = 0;
  /// Short transient line after boop / desk actions (empty if none).
  String toastLine;
  /// True when hunger/boredom are very high — companion asks for attention (visual ping).
  bool attentionPing = false;
  /// Age label e.g. "12d", empty if unknown (no hatch UTC or no valid time).
  String ageLine;
  /// Long sick streak — droopy proc face + mood copy.
  bool isSick = false;
  /// High care score — small gold accent on companion header.
  bool careAccent = false;
};

}  // namespace bookworm

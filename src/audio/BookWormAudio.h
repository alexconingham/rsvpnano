#pragma once

#include <Arduino.h>
#include <vector>

namespace bookworm {

class BookWormAudio {
 public:
  enum class DeskSfx : uint8_t {
    Play = 0,
    Feed = 1,
    Pet = 2,
    Sick = 3,
    Boop = 4,
    Evolution = 5,
  };

  struct ToneGenerator {
    uint32_t frequency = 0;
    uint32_t durationMs = 0;
    uint32_t startMs = 0;
    float amplitude = 0.5f;
    bool active = false;
  };

  static void begin();
  static void playDeskSfx(DeskSfx sfx);
  static void update();
  static void stop();

 private:
  static void playSequence(const std::vector<ToneGenerator> &tones);
};

}  // namespace bookworm

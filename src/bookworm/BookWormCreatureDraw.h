#pragma once

#include <stdint.h>

namespace bookworm {

struct CreatureStyleColors {
  uint16_t body = 0;
  uint16_t shadow = 0;
  uint16_t highlight = 0;
  uint16_t accent = 0;
  uint16_t eye = 0;
  uint16_t eyePupil = 0;
};

/// Filled rectangle in virtual display coordinates (RGB565).
using CreaturePlotFn = void (*)(void *ctx, int x, int y, int w, int h, uint16_t rgb565);

/// Symmetry-axis pixel creature: ZzSprite-style fuzzy mirrored silhouette (Frank Force, MIT) with
/// deterministic per-cell noise, four hash-chosen body templates, optional fused head mass, stub
/// limbs, evolution ornaments. Colors from `fillCreatureStyleColorsCc29` (Lospec CC-29 only).
void drawProcCreature(CreaturePlotFn plot, void *ctx, int anchorCx, int anchorCy,
                      const CreatureStyleColors &palette, const char *petName, uint8_t styleId,
                      uint8_t evolutionStage, bool flashPulse, uint32_t animTimeMs,
                      uint16_t hungerPermille, uint16_t boredomPermille, bool isSick);

}  // namespace bookworm

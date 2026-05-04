#pragma once

// Lospec CC-29 palette only — https://lospec.com/palette-list/cc-29 (RGB565 for ST7789).

#include "bookworm/BookWormCreatureDraw.h"

#include <stdint.h>

namespace bookworm {

enum { kCc29Count = 29 };

inline uint16_t cc29Color(unsigned index) {
  static const uint16_t kTable[29] = {
      0xF79C, 0xBDB7, 0x8411, 0x630C, 0x4229, 0x39CB, 0x2104, 0x3148, 0x420D, 0x4C19,
      0x6E1A, 0xA6F8, 0xEF13, 0xD50D, 0xB28A, 0x6A8D, 0x4A0B, 0x8247, 0xA3CB, 0xE676,
      0xC68D, 0x8D8C, 0x53CF, 0x4AC9, 0x7B88, 0xB5AF, 0xEE58, 0xCC59, 0x5AAD,
  };
  return kTable[index < static_cast<unsigned>(kCc29Count) ? index : 0u];
}

/// Near-black from CC-29 for creature outline / pupils.
inline uint16_t cc29Outline() { return cc29Color(6); }

/// Eight CC-29 “skins”; all creature pixels are chosen from this palette.
inline void fillCreatureStyleColorsCc29(CreatureStyleColors *out, uint8_t styleId) {
  const uint8_t s = static_cast<uint8_t>(styleId % 8);
  uint16_t body = cc29Color(9);
  uint16_t shadow = cc29Color(8);
  uint16_t hi = cc29Color(10);
  uint16_t accent = cc29Color(12);
  switch (s) {
    case 0:
      body = cc29Color(9);
      shadow = cc29Color(8);
      hi = cc29Color(10);
      accent = cc29Color(12);
      break;
    case 1:
      body = cc29Color(21);
      shadow = cc29Color(23);
      hi = cc29Color(20);
      accent = cc29Color(12);
      break;
    case 2:
      body = cc29Color(22);
      shadow = cc29Color(23);
      hi = cc29Color(11);
      accent = cc29Color(10);
      break;
    case 3:
      body = cc29Color(15);
      shadow = cc29Color(16);
      hi = cc29Color(27);
      accent = cc29Color(27);
      break;
    case 4:
      body = cc29Color(14);
      shadow = cc29Color(17);
      hi = cc29Color(26);
      accent = cc29Color(12);
      break;
    case 5:
      body = cc29Color(18);
      shadow = cc29Color(17);
      hi = cc29Color(19);
      accent = cc29Color(20);
      break;
    case 6:
      body = cc29Color(25);
      shadow = cc29Color(24);
      hi = cc29Color(0);
      accent = cc29Color(21);
      break;
    default:
      body = cc29Color(2);
      shadow = cc29Color(4);
      hi = cc29Color(1);
      accent = cc29Color(9);
      break;
  }
  out->body = body;
  out->shadow = shadow;
  out->highlight = hi;
  out->accent = accent;
  out->eye = cc29Color(0);
  out->eyePupil = cc29Outline();
}

}  // namespace bookworm

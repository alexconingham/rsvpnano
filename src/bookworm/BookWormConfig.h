#pragma once

#include <stdint.h>

namespace bookworm {

inline constexpr uint32_t kStateMagic = 0x42574D31;  // 'BWM1'

inline constexpr uint16_t kNeedMax = 1000;

// Need drift per real-time hour at 100% (scaled by current need level / 1000).
inline constexpr uint32_t kHungerRisePerHour = 120;
inline constexpr uint32_t kBoredomRisePerHour = 100;

inline constexpr uint16_t kReadingHungerHealPermille = 8;   // per word at Playing
inline constexpr uint16_t kReadingBoredomHealPermille = 10;

inline constexpr uint16_t kDeskFeedHunger = 180;
inline constexpr uint16_t kDeskPlayBoredom = 200;
inline constexpr uint16_t kDeskPetBoth = 80;

inline constexpr uint8_t kStyleCount = 8;

inline constexpr uint32_t kEvolveStage1Words = 3000;
inline constexpr uint32_t kEvolveStage2Words = 12000;
inline constexpr uint32_t kEvolveStage3Words = 50000;

inline constexpr uint32_t kSimTickIntervalMs = 500;
inline constexpr uint32_t kPersistenceDebounceMs = 45000;

}  // namespace bookworm

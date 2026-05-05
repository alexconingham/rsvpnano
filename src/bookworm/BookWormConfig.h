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
inline constexpr uint16_t kDeskBoopHunger = 25;
inline constexpr uint16_t kDeskBoopBoredom = 35;

inline constexpr uint8_t kStyleCount = 8;

inline constexpr uint32_t kEvolveStage1Words = 5000;
inline constexpr uint32_t kEvolveStage2Words = 25000;
inline constexpr uint32_t kEvolveStage3Words = 100000;

inline constexpr uint32_t kSimTickIntervalMs = 500;
inline constexpr uint32_t kPersistenceDebounceMs = 45000;

/// Both needs high for this long → visibly "sick"; relieved by care + lower needs.
inline constexpr uint32_t kSickAccumThresholdMs = 300000;
inline constexpr uint32_t kSickAccumCapMs = 7200000;
inline constexpr uint32_t kSickCareReliefMs = 200000;

/// Feed-spam: desk feeds within a few seconds stack `overfullTicks` (hunger rises faster).
inline constexpr uint16_t kOverfullTicksFromSpam = 100;

/// Desk-action boost: healing per word is multiplied while xpBoostTicks > 0.
inline constexpr uint8_t  kXpBoostHealMult  = 2;    // 2× hunger/boredom heal rate
/// How long the desk-action XP boost lasts in sim ticks (1 tick = 500 ms).
inline constexpr uint8_t  kXpBoostTicks     = 120;  // ~60 s

}  // namespace bookworm

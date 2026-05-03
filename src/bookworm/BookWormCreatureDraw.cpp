#include "bookworm/BookWormCreatureDraw.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace bookworm {

namespace {

constexpr int kCellPx = 3;
constexpr int kGridH = 22;
constexpr int kGridW = 32;
constexpr int kGridCx = kGridW / 2;

inline uint32_t fnv1aMix(const char *name, uint8_t styleId, uint8_t evo) {
  uint32_t h = 2166136261u;
  h = (h ^ styleId) * 16777619u;
  h = (h ^ evo) * 16777619u;
  if (name) {
    for (const char *p = name; *p; ++p) {
      h = (h ^ static_cast<uint32_t>(static_cast<uint8_t>(*p))) * 16777619u;
    }
  }
  return h ? h : 0xA53C5EEDu;
}

inline uint32_t xorshift32(uint32_t *state) {
  uint32_t x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *state = x;
  return x;
}

inline int ellipseHalfWidth(int y, int halfMaxBase, uint8_t evo) {
  const int mid = kGridH / 2;
  double dy = (y - mid) / double(kGridH / 2 + 1);
  double t = dy * dy;
  if (t > 1.0) {
    t = 1.0;
  }
  int base = static_cast<int>(halfMaxBase * std::sqrt(1.0 - t) + 0.5);
  base = (base * (100 + static_cast<int>(evo) * 12)) / 100;
  return std::max(1, std::min(halfMaxBase + 2, base));
}

inline uint16_t pickBodyColor(const CreatureStyleColors &pal, uint32_t noise, bool flashPulse,
                              uint32_t flashRng) {
  if (flashPulse && (flashRng & 3u) == 0u) {
    return pal.accent;
  }
  switch (noise % 13u) {
    case 0:
    case 1:
      return pal.shadow;
    case 2:
    case 3:
      return pal.highlight;
    default:
      return pal.body;
  }
}

void plotCell(CreaturePlotFn plot, void *ctx, int gx, int gy, int anchorCx, int anchorCy,
              uint16_t color) {
  const int px = anchorCx + (gx - kGridCx) * kCellPx - kCellPx / 2;
  const int py = anchorCy + (gy - kGridH / 2) * kCellPx - kCellPx / 2;
  plot(ctx, px, py, kCellPx, kCellPx, color);
}

}  // namespace

void drawProcCreature(CreaturePlotFn plot, void *ctx, int anchorCx, int anchorCy,
                      const CreatureStyleColors &palette, const char *petName, uint8_t styleId,
                      uint8_t evolutionStage, bool flashPulse) {
  uint8_t grid[kGridH][kGridW];
  std::memset(grid, 0, sizeof(grid));

  const uint8_t evo = std::min<uint8_t>(evolutionStage, 3);
  const int halfMax = 11 + static_cast<int>(evo);
  uint32_t rng = fnv1aMix(petName, styleId, evo);

  for (int y = 0; y < kGridH; ++y) {
    const int hw = ellipseHalfWidth(y, halfMax, evo);
    for (int xi = 0; xi < hw; ++xi) {
      const uint32_t r = xorshift32(&rng);
      const float xiNorm = hw <= 1 ? 0.f : static_cast<float>(xi) / static_cast<float>(hw - 1);
      const float yNorm =
          std::fabs(static_cast<double>(y - kGridH / 2)) / static_cast<double>(kGridH / 2 + 1);
      const double density =
          0.30 + 0.50 * (1.0 - yNorm) * (1.0 - static_cast<double>(xiNorm));
      const uint32_t threshold =
          static_cast<uint32_t>(density * 4294967295.0 * 0.92);
      if (r < threshold) {
        const int xl = kGridCx - 1 - xi;
        const int xr = kGridCx + xi;
        if (xl >= 0 && xl < kGridW) {
          grid[y][xl] = 1;
        }
        if (xr >= 0 && xr < kGridW && xr != xl) {
          grid[y][xr] = 1;
        }
      }
    }
  }

  // Evolution ornaments (symmetric) — bump a few cells so higher stages read clearly.
  if (evo >= 1) {
    const int top = std::max(0, 2 - static_cast<int>(evo) / 2);
    uint32_t rr = xorshift32(&rng);
    const int off = 1 + static_cast<int>(rr % 3u);
    if (top < kGridH && kGridCx - off >= 0) {
      grid[top][kGridCx - off] = 1;
      grid[top][kGridCx + off] = 1;
    }
  }
  if (evo >= 2) {
    const int sy = kGridH / 3;
    for (int d = 3; d <= 4 + static_cast<int>(evo); ++d) {
      if (sy >= 0 && sy < kGridH && kGridCx - d >= 0 && kGridCx + d < kGridW) {
        grid[sy][kGridCx - d] = 1;
        grid[sy][kGridCx + d] = 1;
      }
    }
  }
  if (evo >= 3) {
    const int ty = 1;
    for (int dx = 2; dx <= 6; ++dx) {
      if (ty >= 0 && ty < kGridH && kGridCx - dx >= 0 && kGridCx + dx < kGridW) {
        grid[ty][kGridCx - dx] = 1;
        grid[ty][kGridCx + dx] = 1;
      }
    }
  }

  // Blit body
  for (int y = 0; y < kGridH; ++y) {
    for (int x = 0; x < kGridW; ++x) {
      if (!grid[y][x]) {
        continue;
      }
      const uint32_t n = xorshift32(&rng);
      const uint32_t fr = xorshift32(&rng);
      const uint16_t c = pickBodyColor(palette, n, flashPulse, fr);
      plotCell(plot, ctx, x, y, anchorCx, anchorCy, c);
    }
  }

  // Eyes: first solid row, place symmetric pair just above the blob.
  int minY = kGridH;
  int maxY = 0;
  for (int y = 0; y < kGridH; ++y) {
    for (int x = 0; x < kGridW; ++x) {
      if (grid[y][x]) {
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
      }
    }
  }
  if (minY < kGridH && maxY >= minY) {
    const int eyeY = std::max(0, minY - 1);
    const uint32_t re = xorshift32(&rng);
    const int sep = 2 + static_cast<int>(re % 3u);
    if (eyeY < kGridH && kGridCx - sep >= 1 && kGridCx + sep < kGridW - 1) {
      const auto eyeChunk = [&](int gx, int gy, uint16_t c) {
        plotCell(plot, ctx, gx, gy, anchorCx, anchorCy, c);
      };
      eyeChunk(kGridCx - sep, eyeY, palette.eye);
      eyeChunk(kGridCx + sep, eyeY, palette.eye);
      const int pxL = anchorCx + (kGridCx - sep - kGridCx) * kCellPx - kCellPx / 2;
      const int py = anchorCy + (eyeY - kGridH / 2) * kCellPx - kCellPx / 2;
      const int pxR = anchorCx + (kGridCx + sep - kGridCx) * kCellPx - kCellPx / 2;
      plot(ctx, pxL + kCellPx / 2 - 1, py + kCellPx / 2 - 1, 2, 2, palette.eyePupil);
      plot(ctx, pxR + kCellPx / 2 - 1, py + kCellPx / 2 - 1, 2, 2, palette.eyePupil);
    }
  }
}

}  // namespace bookworm

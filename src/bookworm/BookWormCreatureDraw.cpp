#include "bookworm/BookWormCreatureDraw.h"

#include "bookworm/BookWormCc29.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace bookworm {

namespace {

constexpr int kCellPx = 3;
constexpr int kGridH = 28;
constexpr int kGridW = 36;
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

inline uint32_t cellNoise(uint32_t salt, int i, int j) {
  uint32_t x =
      salt + static_cast<uint32_t>(i * 73856093) + static_cast<uint32_t>(j * 19349663);
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return x;
}

inline float u01(uint32_t x) { return static_cast<float>(x & 0xFFFFFFu) * (1.0f / 16777216.0f); }

inline uint16_t shadeForCell(const CreatureStyleColors &pal, int gx, int gy, int minY, int maxY,
                             int bellyY, bool flashPulse, uint32_t flashHash) {
  if (flashPulse && (flashHash ^ static_cast<uint32_t>(gx * 17 + gy * 131)) % 37u < 2u) {
    return pal.accent;
  }
  if (gy >= bellyY) {
    return pal.shadow;
  }
  const int h = std::max(1, maxY - minY);
  const float yn = static_cast<float>(gy - minY) / static_cast<float>(h);
  const int distMid = std::abs(gx - kGridCx);
  if (yn < 0.4f && distMid <= 5) {
    return pal.highlight;
  }
  return pal.body;
}

void plotCell(CreaturePlotFn plot, void *ctx, int gx, int gy, int anchorCx, int anchorCy,
              uint16_t color) {
  const int px = anchorCx + (gx - kGridCx) * kCellPx - kCellPx / 2;
  const int py = anchorCy + (gy - kGridH / 2) * kCellPx - kCellPx / 2;
  plot(ctx, px, py, kCellPx, kCellPx, color);
}

/// Paint mirrored columns: i = distance from axis, xl = kGridCx - 1 - i - dc, xr = kGridCx + i + dc
void setMirror(uint8_t grid[kGridH][kGridW], int i, int gy, int dc) {
  if (gy < 0 || gy >= kGridH || i < 0) {
    return;
  }
  const int xl = kGridCx - 1 - i - dc;
  const int xr = kGridCx + i + dc;
  if (xl >= 0 && xl < kGridW) {
    grid[gy][xl] = 1;
  }
  if (xr >= 0 && xr < kGridW && xr != xl) {
    grid[gy][xr] = 1;
  }
}

/// ZzSprite-style edge + solid interior: fuzzy boundary from per-cell radius, plus an ellipse core
/// that is always filled so shapes read as mass (not PNG/rendering — avoids hole noise + speckle).
void zzSilhouettePass(uint8_t grid[kGridH][kGridW], uint32_t stream, int row0, int w, int h,
                      float maxRadiusPx, float yBias, int doubleCenter, uint32_t noiseSalt,
                      float innerCoreScale) {
  if (w < 1 || h < 1) {
    return;
  }
  const float centerJ = (1.0f - 2.0f * yBias) * static_cast<float>(std::max(1, h - 1)) * 0.5f;
  const float capR = std::max(2.0f, maxRadiusPx);
  const float innerR = capR * innerCoreScale;
  const float innerR2 = innerR * innerR;
  const int dc = doubleCenter;

  for (int j = 0; j < h; ++j) {
    for (int i = 0; i < w; ++i) {
      const uint32_t n1 = cellNoise(noiseSalt ^ stream, i, j);
      const float cellR = u01(n1) * capR;
      const float dj = static_cast<float>(j) - centerJ;
      const float dist2 = static_cast<float>(i * i) + dj * dj;
      const bool coreSolid = (dist2 <= innerR2);
      const bool fuzzyEdge = (cellR * cellR > dist2);
      if (coreSolid || fuzzyEdge) {
        const int gy = row0 + j;
        setMirror(grid, i, gy, dc);
      }
    }
  }
}

/// Grow mask by 4-neighbor dilation (fills 1px gaps / thickens limbs). Run twice for diagonals.
void dilateCardinal(uint8_t grid[kGridH][kGridW]) {
  uint8_t tmp[kGridH][kGridW];
  std::memcpy(tmp, grid, sizeof(tmp));
  static const int d4y[] = {-1, 1, 0, 0};
  static const int d4x[] = {0, 0, -1, 1};
  for (int y = 0; y < kGridH; ++y) {
    for (int x = 0; x < kGridW; ++x) {
      if (tmp[y][x]) {
        grid[y][x] = 1;
        continue;
      }
      for (int k = 0; k < 4; ++k) {
        const int ny = y + d4y[k];
        const int nx = x + d4x[k];
        if (ny >= 0 && ny < kGridH && nx >= 0 && nx < kGridW && tmp[ny][nx]) {
          grid[y][x] = 1;
          break;
        }
      }
    }
  }
}

void addEvolutionFeatures(uint8_t grid[kGridH][kGridW], uint32_t *rng, uint8_t evo) {
  if (evo < 1) {
    return;
  }
  const int topRow = std::max(0, 1);
  const int off = 2 + static_cast<int>(xorshift32(rng) % 3u);  // antenna separation
  if (topRow < kGridH && kGridCx - off >= 0) {
    grid[topRow][kGridCx - off] = 1;
    grid[topRow][kGridCx + off] = 1;
  }
  if (evo >= 2) {
    const int sy = kGridH * 2 / 5;
    for (int d = 3; d <= 5 + static_cast<int>(evo); ++d) {
      if (sy >= 0 && sy < kGridH && kGridCx - d >= 0 && kGridCx + d < kGridW) {
        grid[sy][kGridCx - d] = 1;
        grid[sy][kGridCx + d] = 1;
      }
    }
  }
  if (evo >= 3) {
    const int ty = 0;
    for (int dx = 3; dx <= 7; ++dx) {
      if (ty < kGridH && kGridCx - dx >= 0 && kGridCx + dx < kGridW) {
        grid[ty][kGridCx - dx] = 1;
        grid[ty][kGridCx + dx] = 1;
      }
    }
  }
}

/// Short symmetric stubs — reads as tiny arms / paws, not long SVG lines.
void addStubLimbs(uint8_t grid[kGridH][kGridW], uint32_t salt, int minY, int maxY, uint8_t evo) {
  if (maxY < minY) {
    return;
  }
  const int midY = (minY + maxY) / 2;
  const uint32_t r0 = cellNoise(salt, 41, 90);
  const int reach = 2 + static_cast<int>(r0 % 3u) + static_cast<int>(evo > 1);
  const int thick = 2;
  for (int t = 0; t < thick; ++t) {
    const int y = midY + t - thick / 2;
    if (y < 0 || y >= kGridH) {
      continue;
    }
    for (int s = 1; s <= reach; ++s) {
      if (kGridCx + s < kGridW) {
        grid[y][kGridCx + s] = 1;
      }
      if (kGridCx - 1 - s >= 0) {
        grid[y][kGridCx - 1 - s] = 1;
      }
    }
  }
}

void bboxFilled(const uint8_t grid[kGridH][kGridW], int *outMinY, int *outMaxY) {
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
  *outMinY = minY;
  *outMaxY = maxY;
}

}  // namespace

void drawProcCreature(CreaturePlotFn plot, void *ctx, int anchorCx, int anchorCy,
                      const CreatureStyleColors &palette, const char *petName, uint8_t styleId,
                      uint8_t evolutionStage, bool flashPulse, uint32_t animTimeMs,
                      uint16_t hungerPermille, uint16_t boredomPermille, bool isSick) {
  uint8_t grid[kGridH][kGridW];
  std::memset(grid, 0, sizeof(grid));

  const uint8_t evo = std::min<uint8_t>(evolutionStage, 3);
  const uint32_t seed = fnv1aMix(petName, styleId, evo);
  uint32_t sp = seed ^ 0x9E3779B9u;
  auto nextU = [&sp]() -> uint32_t {
    sp ^= sp << 13;
    sp ^= sp >> 17;
    sp ^= sp << 5;
    return sp;
  };

  // Four silhouette families (hash); each biases aspect + ZzSprite params.
  const int templateId = static_cast<int>((seed >> 8) % 4u);

  bool flipWide = (nextU() & 1u) != 0;
  if (templateId == 1 || templateId == 3) {
    flipWide = true;
  } else if (templateId == 2) {
    flipWide = false;
  }

  int baseSize = 24 + static_cast<int>(evo) * 2;
  int w = flipWide ? (baseSize - 3) : (baseSize / 2 - 1);
  int h = flipWide ? (baseSize / 2 - 1) : (baseSize - 3);

  if (templateId == 2) {
    h += 5;
    w = std::max(4, w - 2);
  } else if (templateId == 3) {
    w += 4;
    h = std::max(6, h - 1);
  }

  w = std::max(4, std::min(kGridCx - 2, w));
  h = std::max(6, std::min(kGridH - 2, h));

  const int doubleCenter = (nextU() & 1u) != 0 ? 1 : 0;
  float yBias = -0.11f + static_cast<float>(nextU() % 2400) * (1.0f / 10000.0f);
  if (templateId == 1) {
    yBias += 0.085f;
  }

  const float extent = std::sqrt(static_cast<float>(w * w + h * h)) * 0.5f;
  float maxR =
      extent * (0.72f + static_cast<float>(nextU() % 4096) * (0.38f / 4096.0f));

  int row0 = std::max(0, (kGridH - h) / 2 + static_cast<int>(templateId % 2) - 1);

  const float bodyCore = (templateId == 1) ? 0.52f : 0.55f;
  zzSilhouettePass(grid, seed, row0, w, h, maxR, yBias, doubleCenter, 0xC001D00Du, bodyCore);

  // Second fused mass: "big head" / top-heavy (templates 0,3) — breaks single-ellipse redundancy.
  if (templateId == 0 || templateId == 3) {
    const int hh = std::max(4, std::min(9, 4 + static_cast<int>(nextU() % 5u)));
    const int hw = std::max(3, std::min(w - 1, 3 + static_cast<int>(nextU() % 4u)));
    const float headExtent = std::sqrt(static_cast<float>(hw * hw + hh * hh)) * 0.5f;
    float headMaxR = headExtent * (0.78f + static_cast<float>(nextU() % 3000) * (0.25f / 3000.0f));
    int headRow0 = std::max(0, row0 - hh + 2);
    zzSilhouettePass(grid, seed ^ 0xFACEB00Cu, headRow0, hw, hh, headMaxR, -0.06f, 0,
                     0x51ED9001u, 0.58f);
  }

  uint32_t rng = nextU();
  addEvolutionFeatures(grid, &rng, evo);

  int minY = kGridH;
  int maxY = 0;
  bboxFilled(grid, &minY, &maxY);

  if (templateId != 2 && maxY >= minY && (evo >= 1 || (seed & 3u) != 0u)) {
    addStubLimbs(grid, seed ^ 0x600DCAFEu, minY, maxY, evo);
  }

  bboxFilled(grid, &minY, &maxY);

  dilateCardinal(grid);
  dilateCardinal(grid);

  uint8_t outline[kGridH][kGridW];
  std::memset(outline, 0, sizeof(outline));
  for (int y = 0; y < kGridH; ++y) {
    for (int x = 0; x < kGridW; ++x) {
      if (grid[y][x]) {
        continue;
      }
      static const int d4y[] = {-1, 1, 0, 0};
      static const int d4x[] = {0, 0, -1, 1};
      for (int i = 0; i < 4; ++i) {
        const int ny = y + d4y[i];
        const int nx = x + d4x[i];
        if (ny >= 0 && ny < kGridH && nx >= 0 && nx < kGridW && grid[ny][nx]) {
          outline[y][x] = 1;
          break;
        }
      }
    }
  }

  const uint16_t outlineCol = cc29Outline();

  for (int y = 0; y < kGridH; ++y) {
    for (int x = 0; x < kGridW; ++x) {
      if (!outline[y][x]) {
        continue;
      }
      plotCell(plot, ctx, x, y, anchorCx, anchorCy, outlineCol);
    }
  }

  bboxFilled(grid, &minY, &maxY);
  const int bellyY =
      minY < kGridH ? minY + std::max(3, (maxY - minY) * 5 / 8) : kGridH / 2;
  const uint32_t flashHash = xorshift32(&rng);

  for (int y = 0; y < kGridH; ++y) {
    for (int x = 0; x < kGridW; ++x) {
      if (!grid[y][x]) {
        continue;
      }
      uint16_t c = shadeForCell(palette, x, y, minY, maxY, bellyY, flashPulse, flashHash);
      plotCell(plot, ctx, x, y, anchorCx, anchorCy, c);
    }
  }

  if (minY < kGridH && maxY >= minY) {
    const int span = std::max(2, maxY - minY);
    int faceY = minY + span / 4;
    if (faceY < 0) {
      faceY = 0;
    }
    if (faceY > kGridH - 1) {
      faceY = kGridH - 1;
    }
    const uint32_t re = xorshift32(&rng);
    const int sep = 6 + static_cast<int>(re % 4u);
    constexpr int kEyeRadX = 2;
    constexpr int kEyeRadY = 2;

    int faceYClamped = faceY;
    if (faceYClamped - kEyeRadY < 0) {
      faceYClamped = kEyeRadY;
    }
    if (faceYClamped + kEyeRadY >= kGridH) {
      faceYClamped = kGridH - 1 - kEyeRadY;
    }

    const int leftCx = kGridCx - sep;
    const int rightCx = kGridCx + sep;
    if (faceYClamped >= 0 && faceYClamped < kGridH && leftCx - kEyeRadX >= 0 &&
        rightCx + kEyeRadX < kGridW) {
      const uint32_t eyeSeed = fnv1aMix(petName, styleId, evo);
      const uint32_t anim = animTimeMs;
      const bool needyHungry = hungerPermille > 700;
      const bool needyBored = boredomPermille > 700;
      const bool bothStressed = hungerPermille > 600 && boredomPermille > 600;
      const bool content =
          hungerPermille < 350 && boredomPermille < 350 && !needyHungry && !needyBored;

      uint32_t blinkPeriod = 3600u + (eyeSeed % 1800u);
      if (bothStressed) {
        blinkPeriod = blinkPeriod * 3u / 4u;
      } else if (needyBored && !needyHungry) {
        blinkPeriod = blinkPeriod * 11u / 10u;
      }
      if (isSick) {
        blinkPeriod = blinkPeriod * 5u / 8u;
      }
      bool eyeShut = false;
      if (anim > 0u) {
        const uint32_t t = (anim + (eyeSeed >> 15)) % blinkPeriod;
        uint32_t kBlinkMs = 140u;
        if (bothStressed) {
          kBlinkMs = 190u;
        } else if (needyBored && !needyHungry) {
          kBlinkMs = 200u;
        }
        if (isSick) {
          kBlinkMs = 280u;
        }
        eyeShut = (t >= blinkPeriod - kBlinkMs);
      }

      static const int8_t kLookQx[] = {0, 1, 1, 1, 0, -1, -1, -1};
      static const int8_t kLookQy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
      unsigned lookK = 0;
      if (anim > 0u && !eyeShut) {
        lookK = ((anim + (eyeSeed >> 7)) / 420u) % 8u;
        if (needyHungry && !needyBored) {
          lookK = 5u + (lookK % 3u);
        } else if (needyBored && !needyHungry) {
          lookK = (lookK + 3u) % 8u;
        }
      }

      const auto drawClosedEye = [&](int centerGx, int centerGy) {
        for (int dy = -kEyeRadY; dy <= kEyeRadY; ++dy) {
          for (int dx = -kEyeRadX; dx <= kEyeRadX; ++dx) {
            plotCell(plot, ctx, centerGx + dx, centerGy + dy, anchorCx, anchorCy, palette.eye);
          }
        }
        const int gx0 = centerGx - kEyeRadX;
        const int gy0 = centerGy - kEyeRadY;
        const int px0 = anchorCx + (gx0 - kGridCx) * kCellPx - kCellPx / 2;
        const int py0 = anchorCy + (gy0 - kGridH / 2) * kCellPx - kCellPx / 2;
        const int eyeW = (2 * kEyeRadX + 1) * kCellPx;
        const int eyeH = (2 * kEyeRadY + 1) * kCellPx;
        const int lidH = std::max(2, eyeH / 3);
        plot(ctx, px0, py0 + (eyeH - lidH) / 2, eyeW, lidH, palette.shadow);
      };

      const auto eyeOpenBlock = [&](int centerGx, int centerGy) {
        for (int dy = -kEyeRadY; dy <= kEyeRadY; ++dy) {
          for (int dx = -kEyeRadX; dx <= kEyeRadX; ++dx) {
            plotCell(plot, ctx, centerGx + dx, centerGy + dy, anchorCx, anchorCy, palette.eye);
          }
        }
        const int gx0 = centerGx - kEyeRadX;
        const int gy0 = centerGy - kEyeRadY;
        const int px0 = anchorCx + (gx0 - kGridCx) * kCellPx - kCellPx / 2;
        const int py0 = anchorCy + (gy0 - kGridH / 2) * kCellPx - kCellPx / 2;
        const int eyeW = (2 * kEyeRadX + 1) * kCellPx;
        const int eyeH = (2 * kEyeRadY + 1) * kCellPx;
        int pupilFracNum = 12;
        int pupilFracDen = 25;
        if (content) {
          pupilFracNum = 14;
        } else if (needyBored && !needyHungry) {
          pupilFracNum = 9;
        } else if (bothStressed) {
          pupilFracNum = 11;
        }
        if (isSick) {
          pupilFracNum = std::min(pupilFracNum, 8);
        }
        const int pupilW = std::max(kCellPx + 1, eyeW * pupilFracNum / pupilFracDen);
        const int pupilH = std::max(kCellPx + 1, eyeH * pupilFracNum / pupilFracDen);
        const int maxDx = std::max(0, (eyeW - pupilW) / 2);
        const int maxDy = std::max(0, (eyeH - pupilH) / 2);
        int pdx = 0;
        int pdy = 0;
        if (maxDx > 0 || maxDy > 0) {
          const int mx = std::min(2, maxDx);
          const int my = std::min(2, maxDy);
          pdx = static_cast<int>(kLookQx[lookK]) * mx;
          pdy = static_cast<int>(kLookQy[lookK]) * my;
          if (pdx > maxDx) {
            pdx = maxDx;
          }
          if (pdx < -maxDx) {
            pdx = -maxDx;
          }
          if (pdy > maxDy) {
            pdy = maxDy;
          }
          if (pdy < -maxDy) {
            pdy = -maxDy;
          }
        }
        if (needyHungry) {
          pdy = std::min(maxDy, pdy + std::max(1, maxDy / 2 + 1));
        }
        plot(ctx, px0 + (eyeW - pupilW) / 2 + pdx, py0 + (eyeH - pupilH) / 2 + pdy, pupilW, pupilH,
             palette.eyePupil);
      };

      if (eyeShut) {
        drawClosedEye(leftCx, faceYClamped);
        drawClosedEye(rightCx, faceYClamped);
      } else {
        eyeOpenBlock(leftCx, faceYClamped);
        eyeOpenBlock(rightCx, faceYClamped);
      }
    }
  }
}

}  // namespace bookworm

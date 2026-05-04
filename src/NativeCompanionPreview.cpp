// Desktop-only: procedural companion → PNG (640×172). Excluded from firmware via
// platformio [env] src_filter. Uses stb_image_write (test/support/stb_image_write.h).
//
// Build:  pio run -e native_companion_preview
// Single: .pio/build/native_companion_preview/program.exe [name] [styleId] [evo0-3] [flash0|1] [out.png]
// Batch:  .pio/build/native_companion_preview/program.exe --batch
//         → writes examples/evo0 … examples/evo3, five PNGs each (companion_01.png … companion_05.png).
//
// Requires a host g++ on PATH (e.g. MSYS2 mingw-w64). Run from repo root so examples/ is relative.

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <bookworm/BookWormCreatureDraw.h>
#include <bookworm/BookWormCc29.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <vector>

namespace {

constexpr int kW = 640;
constexpr int kH = 172;
constexpr int kNamePool = 20;
constexpr int kStages = 4;
constexpr int kPerStage = 5;

struct Rgb888 {
  uint8_t r, g, b;
};

// Distinct seeds for proc-gen (name affects FNV mix; syllable-style like hatch names).
static const char *const kVariantNames[kNamePool] = {
    "glormwick", "bruffkin",  "scrabnip",  "flooplet", "trunkzo",    "pliddlemo", "druzzlebit",
    "wormcrax",  "snorpkin",  "gleebot",   "braxnibble", "scroopuff", "flobbit", "tribblet",
    "plankzo",   "werpuggle", "snuffmo",   "craxnip", "glibblet", "droozwick",
};

Rgb888 from565(uint16_t c) {
  Rgb888 out;
  out.r = static_cast<uint8_t>(((c >> 11) & 0x1Fu) << 3);
  out.g = static_cast<uint8_t>(((c >> 5) & 0x3Fu) << 2);
  out.b = static_cast<uint8_t>((c & 0x1Fu) << 3);
  return out;
}

struct PreviewCtx {
  Rgb888 *pix;
};

void plot(void *ctx, int x, int y, int w, int h, uint16_t rgb565) {
  auto *c = static_cast<PreviewCtx *>(ctx);
  const Rgb888 col = from565(rgb565);
  const int x1 = std::max(0, x);
  const int y1 = std::max(0, y);
  const int x2 = std::min(kW, x + w);
  const int y2 = std::min(kH, y + h);
  for (int yy = y1; yy < y2; ++yy) {
    for (int xx = x1; xx < x2; ++xx) {
      c->pix[yy * kW + xx] = col;
    }
  }
}

bool writePng(const char *path, const Rgb888 *pix) {
  std::vector<uint8_t> rgb(static_cast<size_t>(kW * kH * 3));
  for (int i = 0; i < kW * kH; ++i) {
    const size_t o = static_cast<size_t>(i) * 3U;
    rgb[o] = pix[i].r;
    rgb[o + 1] = pix[i].g;
    rgb[o + 2] = pix[i].b;
  }
  return stbi_write_png(path, kW, kH, 3, rgb.data(), kW * 3) != 0;
}

bool renderOnePng(const char *outPath, const char *name, uint8_t style, uint8_t evo, bool flash) {
  if (evo > 3) {
    evo = 3;
  }
  std::vector<Rgb888> pixels(static_cast<size_t>(kW * kH));
  constexpr Rgb888 bg = {32, 32, 40};
  for (size_t i = 0; i < pixels.size(); ++i) {
    pixels[i] = bg;
  }

  bookworm::CreatureStyleColors pal;
  bookworm::fillCreatureStyleColorsCc29(&pal, style);

  PreviewCtx ctx{pixels.data()};
  const int cx = kW / 2;
  const int cy = 72;
  bookworm::drawProcCreature(plot, &ctx, cx, cy, pal, name, style, evo, flash, 0, 280, 280, false);

  if (!writePng(outPath, pixels.data())) {
    return false;
  }
  return true;
}

bool isBatchArg(const char *s) {
  return s != nullptr &&
         (std::strcmp(s, "--batch") == 0 || std::strcmp(s, "-batch") == 0 ||
          std::strcmp(s, "batch") == 0);
}

int runBatchExamples() {
  namespace fs = std::filesystem;
  std::error_code ec;
  fs::create_directories("examples", ec);
  if (ec) {
    std::fprintf(stderr, "failed to create examples/: %s\n", ec.message().c_str());
    return 1;
  }

  int total = 0;
  for (int evo = 0; evo < kStages; ++evo) {
    char folder[64];
    std::snprintf(folder, sizeof(folder), "examples/evo%d", evo);
    fs::create_directories(folder, ec);
    if (ec) {
      std::fprintf(stderr, "failed to create %s: %s\n", folder, ec.message().c_str());
      return 1;
    }

    for (int i = 0; i < kPerStage; ++i) {
      char pathBuf[192];
      std::snprintf(pathBuf, sizeof(pathBuf), "examples/evo%d/companion_%02d.png", evo, i + 1);
      const int idx = evo * kPerStage + i;
      const uint8_t style = static_cast<uint8_t>(idx % 8);
      const uint8_t evoU = static_cast<uint8_t>(evo);
      const bool flash = ((idx % 6) == 0);
      const char *petName = kVariantNames[static_cast<size_t>(idx % kNamePool)];
      if (!renderOnePng(pathBuf, petName, style, evoU, flash)) {
        std::fprintf(stderr, "failed to write %s\n", pathBuf);
        return 1;
      }
      std::printf("%s  name=%s style=%u evo=%u flash=%d\n", pathBuf, petName,
                  static_cast<unsigned>(style), static_cast<unsigned>(evoU), flash ? 1 : 0);
      ++total;
    }
  }
  std::printf("wrote %d files under examples/evo0 … evo%d/\n", total, kStages - 1);
  return 0;
}

}  // namespace

int main(int argc, char **argv) {
  if (argc >= 2 && isBatchArg(argv[1])) {
    return runBatchExamples();
  }

  const char *name = "bruffkin";
  uint8_t style = 0;
  uint8_t evo = 0;
  bool flash = false;
  const char *outPath = "companion_preview.png";

  if (argc >= 2 && argv[1][0] != '-') {
    name = argv[1];
  }
  if (argc >= 3) {
    style = static_cast<uint8_t>(std::atoi(argv[2]) & 255);
  }
  if (argc >= 4) {
    evo = static_cast<uint8_t>(std::atoi(argv[3]));
    if (evo > 3) {
      evo = 3;
    }
  }
  if (argc >= 5) {
    flash = (std::atoi(argv[4]) != 0);
  }
  if (argc >= 6) {
    outPath = argv[5];
  }

  if (!renderOnePng(outPath, name, style, evo, flash)) {
    std::fprintf(stderr, "failed to write %s\n", outPath);
    return 1;
  }
  std::printf("wrote %s (%dx%d PNG) name=%s style=%u evo=%u flash=%d\n", outPath, kW, kH, name,
              static_cast<unsigned>(style), static_cast<unsigned>(evo), flash ? 1 : 0);
  return 0;
}

// Desktop-only: procedural companion → PPM image (640×172). Excluded from firmware via
// platformio [env] src_filter.
//
// Build:  pio run -e native_companion_preview
// Run:    .pio/build/native_companion_preview/program.exe [name] [styleId] [evo0-3] [flash0|1] [out.ppm]
//
// Requires a host g++ on PATH (e.g. MSYS2 mingw-w64). Open the PPM in an image viewer or browser.

#include <bookworm/BookWormCreatureDraw.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

constexpr int kW = 640;
constexpr int kH = 172;

struct Rgb888 {
  uint8_t r, g, b;
};

static const uint16_t kStyleColors[] = {
    0xF813, 0x07E8, 0x281F, 0xFFE5, 0xF81F, 0x05FF, 0xFDA0, 0xDEFB,
};

Rgb888 from565(uint16_t c) {
  Rgb888 out;
  out.r = static_cast<uint8_t>(((c >> 11) & 0x1Fu) << 3);
  out.g = static_cast<uint8_t>(((c >> 5) & 0x3Fu) << 2);
  out.b = static_cast<uint8_t>((c & 0x1Fu) << 3);
  return out;
}

uint16_t shade565(uint16_t c, int num, int den) {
  uint32_t r = ((c >> 11) & 0x1Fu) * static_cast<uint32_t>(num) / static_cast<uint32_t>(den);
  uint32_t g = ((c >> 5) & 0x3Fu) * static_cast<uint32_t>(num) / static_cast<uint32_t>(den);
  uint32_t b = (c & 0x1Fu) * static_cast<uint32_t>(num) / static_cast<uint32_t>(den);
  return static_cast<uint16_t>((r << 11) | (g << 5) | b);
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

bool writePpm(const char *path, const Rgb888 *pix) {
  FILE *f = std::fopen(path, "wb");
  if (!f) {
    return false;
  }
  std::fprintf(f, "P6 %d %d 255\n", kW, kH);
  for (int i = 0; i < kW * kH; ++i) {
    std::fputc(static_cast<int>(pix[i].r), f);
    std::fputc(static_cast<int>(pix[i].g), f);
    std::fputc(static_cast<int>(pix[i].b), f);
  }
  std::fclose(f);
  return true;
}

}  // namespace

int main(int argc, char **argv) {
  const char *name = "bruffkin";
  uint8_t style = 0;
  uint8_t evo = 0;
  bool flash = false;
  const char *outPath = "companion_preview.ppm";

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

  std::vector<Rgb888> pixels(static_cast<size_t>(kW * kH));
  Rgb888 bg = {32, 32, 40};
  for (size_t i = 0; i < pixels.size(); ++i) {
    pixels[i] = bg;
  }

  const uint16_t pet565 = kStyleColors[style % 8];
  bookworm::CreatureStyleColors pal;
  pal.body = pet565;
  pal.shadow = shade565(pet565, 2, 3);
  pal.highlight = shade565(pet565, 5, 4);
  pal.accent = 0x07E0;
  pal.eye = 0xFFDF;
  pal.eyePupil = 0x2965;

  PreviewCtx ctx{pixels.data()};
  const int cx = kW / 2;
  const int cy = 72;
  bookworm::drawProcCreature(plot, &ctx, cx, cy, pal, name, style, evo, flash);

  if (!writePpm(outPath, pixels.data())) {
    std::fprintf(stderr, "failed to write %s\n", outPath);
    return 1;
  }
  std::printf("wrote %s (%dx%d PPM) name=%s style=%u evo=%u flash=%d\n", outPath, kW, kH, name,
              static_cast<unsigned>(style), static_cast<unsigned>(evo), flash ? 1 : 0);
  return 0;
}

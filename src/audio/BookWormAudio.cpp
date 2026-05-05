#include "audio/BookWormAudio.h"

#include <Wire.h>
#include <driver/i2s.h>
#include <math.h>

namespace bookworm {

constexpr i2s_port_t I2S_PORT = I2S_NUM_0;
constexpr uint32_t SAMPLE_RATE = 24000;
constexpr size_t SAMPLES_PER_BUFFER = 512;

static bool audioInitialized = false;
static std::vector<BookWormAudio::ToneGenerator> activeTones;
static uint32_t sampleCount = 0;

std::vector<BookWormAudio::ToneGenerator> makePlaySequence() {
  std::vector<BookWormAudio::ToneGenerator> seq;
  BookWormAudio::ToneGenerator t1;
  t1.frequency = 523; t1.durationMs = 150; t1.amplitude = 0.25f;
  seq.push_back(t1);
  BookWormAudio::ToneGenerator t2;
  t2.frequency = 659; t2.durationMs = 150; t2.amplitude = 0.25f;
  seq.push_back(t2);
  BookWormAudio::ToneGenerator t3;
  t3.frequency = 784; t3.durationMs = 200; t3.amplitude = 0.30f;
  seq.push_back(t3);
  return seq;
}

std::vector<BookWormAudio::ToneGenerator> makeFeedSequence() {
  std::vector<BookWormAudio::ToneGenerator> seq;
  BookWormAudio::ToneGenerator t1;
  t1.frequency = 440; t1.durationMs = 100; t1.amplitude = 0.25f;
  seq.push_back(t1);
  BookWormAudio::ToneGenerator t2;
  t2.frequency = 523; t2.durationMs = 100; t2.amplitude = 0.25f;
  seq.push_back(t2);
  BookWormAudio::ToneGenerator t3;
  t3.frequency = 587; t3.durationMs = 150; t3.amplitude = 0.30f;
  seq.push_back(t3);
  return seq;
}

std::vector<BookWormAudio::ToneGenerator> makePetSequence() {
  std::vector<BookWormAudio::ToneGenerator> seq;
  BookWormAudio::ToneGenerator t1;
  t1.frequency = 659; t1.durationMs = 100; t1.amplitude = 0.20f;
  seq.push_back(t1);
  BookWormAudio::ToneGenerator t2;
  t2.frequency = 784; t2.durationMs = 120; t2.amplitude = 0.20f;
  seq.push_back(t2);
  BookWormAudio::ToneGenerator t3;
  t3.frequency = 659; t3.durationMs = 100; t3.amplitude = 0.20f;
  seq.push_back(t3);
  return seq;
}

std::vector<BookWormAudio::ToneGenerator> makeSickSequence() {
  std::vector<BookWormAudio::ToneGenerator> seq;
  BookWormAudio::ToneGenerator t1;
  t1.frequency = 440; t1.durationMs = 200; t1.amplitude = 0.20f;
  seq.push_back(t1);
  BookWormAudio::ToneGenerator t2;
  t2.frequency = 349; t2.durationMs = 200; t2.amplitude = 0.20f;
  seq.push_back(t2);
  BookWormAudio::ToneGenerator t3;
  t3.frequency = 293; t3.durationMs = 300; t3.amplitude = 0.25f;
  seq.push_back(t3);
  return seq;
}

std::vector<BookWormAudio::ToneGenerator> makeBoopSequence() {
  std::vector<BookWormAudio::ToneGenerator> seq;
  BookWormAudio::ToneGenerator t1;
  t1.frequency = 587; t1.durationMs = 80; t1.amplitude = 0.15f;
  seq.push_back(t1);
  BookWormAudio::ToneGenerator t2;
  t2.frequency = 659; t2.durationMs = 100; t2.amplitude = 0.20f;
  seq.push_back(t2);
  BookWormAudio::ToneGenerator t3;
  t3.frequency = 523; t3.durationMs = 120; t3.amplitude = 0.15f;
  seq.push_back(t3);
  return seq;
}

std::vector<BookWormAudio::ToneGenerator> makeEvolutionSequence() {
  std::vector<BookWormAudio::ToneGenerator> seq;
  BookWormAudio::ToneGenerator t1;
  t1.frequency = 784; t1.durationMs = 150; t1.amplitude = 0.35f;  // G5 - triumphant start
  seq.push_back(t1);
  BookWormAudio::ToneGenerator t2;
  t2.frequency = 988; t2.durationMs = 150; t2.amplitude = 0.35f;  // B5 - even higher
  seq.push_back(t2);
  BookWormAudio::ToneGenerator t3;
  t3.frequency = 1047; t3.durationMs = 250; t3.amplitude = 0.40f;  // C6 - ultimate peak
  seq.push_back(t3);
  return seq;
}

static void writeReg(uint8_t reg, uint8_t val) {
  Wire1.beginTransmission(0x18);
  Wire1.write(reg);
  Wire1.write(val);
  Wire1.endTransmission(true);
}

// Try multiple TCA9554 addresses to find which one works
static void enableAmplifierAllAddresses() {
  // Try both 0x20 (TCA9554) and 0x38 (TCA9554A)
  for (uint8_t addr : {0x20, 0x38}) {
    // Set output port (reg 0x01) - all bits high to enable everything
    Wire1.beginTransmission(addr);
    Wire1.write(0x01);
    Wire1.write(0xFF);
    int r1 = Wire1.endTransmission(true);

    // Set config (reg 0x03) - all bits low = output mode
    Wire1.beginTransmission(addr);
    Wire1.write(0x03);
    Wire1.write(0x00);
    int r2 = Wire1.endTransmission(true);

    Serial.printf("[AUDIO] TCA9554 addr 0x%02x: out=%d cfg=%d\n", addr, r1, r2);
  }
}

static uint8_t readReg(uint8_t reg) {
  Wire1.beginTransmission(0x18);
  Wire1.write(reg);
  if (Wire1.endTransmission(false) != 0) {
    Serial.printf("[AUDIO] ES8311 read addr fail reg 0x%02x\n", reg);
    return 0xFF;
  }
  if (Wire1.requestFrom((uint8_t)0x18, (uint8_t)1) != 1) {
    Serial.printf("[AUDIO] ES8311 read req fail reg 0x%02x\n", reg);
    return 0xFF;
  }
  return Wire1.read();
}

static bool initializeEs8311() {
  // Verify codec presence
  Wire1.beginTransmission(0x18);
  int probeResult = Wire1.endTransmission(true);
  Serial.printf("[AUDIO] ES8311 probe at 0x18: result=%d (0=ok)\n", probeResult);

  if (probeResult != 0) {
    Serial.println("[AUDIO] ES8311 NOT FOUND on I2C bus!");
    return false;
  }

  // Read chip ID
  uint8_t id1 = readReg(0xFD);
  uint8_t id2 = readReg(0xFE);
  uint8_t version = readReg(0xFF);
  Serial.printf("[AUDIO] ES8311 ID: 0xFD=0x%02x 0xFE=0x%02x 0xFF=0x%02x\n", id1, id2, version);

  // ES8311 init based on Espressif reference for slave mode, MCLK from master
  // Step 1: Reset and basic config
  writeReg(0x45, 0x00);  // Reset all
  delay(20);

  // Step 2: Clock manager - slave mode, MCLK from external
  writeReg(0x01, 0x30);  // Clock manager: enable BCLK and MCLK
  writeReg(0x02, 0x00);  // CLK_MANAGER2: divider = 1 (no division)
  writeReg(0x03, 0x10);  // CLK_MANAGER3: ADC fs setting
  writeReg(0x04, 0x10);  // CLK_MANAGER4: DAC fs setting
  writeReg(0x05, 0x00);  // CLK_MANAGER5: ADC/DAC clock div
  writeReg(0x06, 0x03);  // CLK_MANAGER6: BCLK div
  writeReg(0x07, 0x00);  // CLK_MANAGER7
  writeReg(0x08, 0xFF);  // CLK_MANAGER8
  writeReg(0x16, 0x24);  // ADC analog config
  writeReg(0x44, 0x08);  // DAC source select

  // Step 3: Serial port config
  writeReg(0x09, 0x0C);  // SDP_IN: 16-bit I2S in
  writeReg(0x0A, 0x0C);  // SDP_OUT: 16-bit I2S out

  // Step 4: System power up
  writeReg(0x0B, 0x00);  // SYSTEM
  writeReg(0x0C, 0x00);  // SYSTEM
  writeReg(0x10, 0x1F);  // SYSTEM10: bias enable
  writeReg(0x11, 0x7F);  // SYSTEM11: bias enable
  writeReg(0x00, 0x80);  // RESET: Set CSM (slave mode)
  writeReg(0x0D, 0x01);  // SYSTEM: power up
  writeReg(0x01, 0x3F);  // CLK_MANAGER1: enable all clocks
  writeReg(0x14, 0x1A);  // SYSTEM14: enable analog PGA
  writeReg(0x12, 0x28);  // SYSTEM12: enable DAC

  // Step 5: DAC settings - unmute, max volume
  writeReg(0x37, 0x48);  // DAC: ramp rate / DSM enable
  writeReg(0x32, 0xA0);  // DAC volume (lower than max to reduce distortion)
  writeReg(0x31, 0x00);  // DAC settings (no mute)

  delay(100);

  // Verify some writes
  uint8_t r32 = readReg(0x32);
  uint8_t r37 = readReg(0x37);
  Serial.printf("[AUDIO] ES8311 verify: 0x32=0x%02x 0x37=0x%02x\n", r32, r37);

  Serial.println("[AUDIO] ES8311 codec configured");
  return true;
}

void BookWormAudio::begin() {
  Serial.println("[AUDIO_BEGIN_CALLED]");
  if (audioInitialized) {
    Serial.println("[AUDIO_ALREADY_INIT]");
    return;
  }

  Serial.println("[AUDIO_INIT_CODEC]");
  initializeEs8311();

  Serial.println("[AUDIO_CONFIGURING_I2S]");
  // I2S configuration
  i2s_config_t i2s_config = {
      .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
      .dma_buf_count = 4,
      .dma_buf_len = SAMPLES_PER_BUFFER,
      .use_apll = true,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0,
  };

  // I2S pin configuration (from BoardConfig)
  i2s_pin_config_t pin_config;
  pin_config.bck_io_num = 15;      // BCLK
  pin_config.ws_io_num = 46;       // WS/LRCK
  pin_config.data_out_num = 45;    // DOUT
  pin_config.data_in_num = 6;      // DIN (not used for output)
  pin_config.mck_io_num = 7;       // MCLK

  esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, nullptr);
  if (err != ESP_OK) {
    Serial.printf("[AUDIO_I2S_INSTALL_FAILED] %d\n", err);
    return;
  }
  Serial.println("[AUDIO_I2S_INSTALLED]");

  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("[AUDIO_I2S_PIN_FAILED] %d\n", err);
    i2s_driver_uninstall(I2S_PORT);
    return;
  }
  Serial.println("[AUDIO_I2S_PINS_SET]");

  audioInitialized = true;
  Serial.println("[AUDIO_INIT_COMPLETE]");
}

void BookWormAudio::playSequence(const std::vector<ToneGenerator> &tones) {
  constexpr uint32_t kGapMs = 40;  // small gap between tones
  uint32_t nowMs = millis();
  uint32_t cumulativeOffset = 0;

  for (const auto &tone : tones) {
    ToneGenerator gen = tone;
    gen.startMs = nowMs + cumulativeOffset;
    gen.active = true;
    activeTones.push_back(gen);
    cumulativeOffset += gen.durationMs + kGapMs;
  }
}

void BookWormAudio::playDeskSfx(DeskSfx sfx) {
  Serial.println("[AUDIO_PLAYDESK_CALLED]");
  // Force enable amplifier on every play
  enableAmplifierAllAddresses();
  if (!audioInitialized) {
    Serial.println("[AUDIO_NOT_INIT_CALLING_BEGIN]");
    begin();
  } else {
    Serial.println("[AUDIO_ALREADY_INIT_SKIPPING_BEGIN]");
    initializeEs8311();
  }

  switch (sfx) {
    case DeskSfx::Play:
      playSequence(makePlaySequence());
      break;
    case DeskSfx::Feed:
      playSequence(makeFeedSequence());
      break;
    case DeskSfx::Pet:
      playSequence(makePetSequence());
      break;
    case DeskSfx::Sick:
      playSequence(makeSickSequence());
      break;
    case DeskSfx::Boop:
      playSequence(makeBoopSequence());
      break;
    case DeskSfx::Evolution:
      playSequence(makeEvolutionSequence());
      break;
  }
}

void BookWormAudio::update() {
  if (!audioInitialized) {
    return;
  }

  uint32_t nowMs = millis();

  // Remove finished tones (only if started AND duration elapsed)
  for (auto it = activeTones.begin(); it != activeTones.end();) {
    // If tone hasn't started yet, keep it
    if (nowMs < it->startMs) {
      ++it;
      continue;
    }
    // If tone has finished, remove it
    if ((nowMs - it->startMs) >= it->durationMs) {
      it = activeTones.erase(it);
    } else {
      ++it;
    }
  }

  if (activeTones.empty()) {
    return;
  }

  // Generate audio samples
  int16_t samples[SAMPLES_PER_BUFFER * 2];  // Stereo
  memset(samples, 0, sizeof(samples));

  constexpr uint32_t kFadeMs = 10;  // 10ms fade in/out for smooth transitions

  for (size_t i = 0; i < SAMPLES_PER_BUFFER; ++i) {
    float sampleL = 0.0f;
    float sampleR = 0.0f;

    for (const auto &tone : activeTones) {
      // Skip tones that haven't started yet
      if (nowMs < tone.startMs) {
        continue;
      }
      uint32_t elapsedMs = nowMs - tone.startMs;
      if (elapsedMs >= tone.durationMs) {
        continue;
      }

      // Envelope: fade in/out to prevent clicks
      float envelope = 1.0f;
      if (elapsedMs < kFadeMs) {
        envelope = (float)elapsedMs / (float)kFadeMs;
      } else if (elapsedMs > tone.durationMs - kFadeMs) {
        envelope = (float)(tone.durationMs - elapsedMs) / (float)kFadeMs;
      }

      // Generate sine wave with envelope
      float phase = 2.0f * M_PI * tone.frequency * (sampleCount + i) / SAMPLE_RATE;
      float sample = tone.amplitude * envelope * sinf(phase);

      sampleL += sample;
      sampleR += sample;
    }

    // Soft clip to prevent harsh distortion
    sampleL = tanhf(sampleL);
    sampleR = tanhf(sampleR);

    samples[i * 2] = static_cast<int16_t>(sampleL * 32767);
    samples[i * 2 + 1] = static_cast<int16_t>(sampleR * 32767);
  }

  size_t bytes_written;
  i2s_write(I2S_PORT, samples, sizeof(samples), &bytes_written, portMAX_DELAY);
  sampleCount += SAMPLES_PER_BUFFER;
}

void BookWormAudio::stop() {
  activeTones.clear();
  if (audioInitialized) {
    i2s_stop(I2S_PORT);
  }
}

}  // namespace bookworm

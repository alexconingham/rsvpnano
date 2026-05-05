#pragma once

#include <Arduino.h>
#include <cstring>
#include <vector>

#ifdef ARDUINO
#include <esp_heap_caps.h>
#endif

// Stores all words as null-terminated strings in a single flat PSRAM buffer.
// Eliminates the thousands of small heap allocations that std::vector<String>
// would produce, which exhaust internal DRAM on large books.
struct WordStore {
  char* buffer_ = nullptr;
  uint32_t capacity_ = 0;
  uint32_t used_ = 0;
  std::vector<uint32_t> offsets_;

  WordStore() = default;
  ~WordStore() { clear(); }

  WordStore(const WordStore&) = delete;
  WordStore& operator=(const WordStore&) = delete;

  WordStore(WordStore&& o) noexcept
      : buffer_(o.buffer_), capacity_(o.capacity_), used_(o.used_),
        offsets_(std::move(o.offsets_)) {
    o.buffer_ = nullptr;
    o.capacity_ = 0;
    o.used_ = 0;
  }

  WordStore& operator=(WordStore&& o) noexcept {
    if (this != &o) {
      clear();
      buffer_ = o.buffer_;
      capacity_ = o.capacity_;
      used_ = o.used_;
      offsets_ = std::move(o.offsets_);
      o.buffer_ = nullptr;
      o.capacity_ = 0;
      o.used_ = 0;
    }
    return *this;
  }

  bool reserveBuffer(size_t charCapacity) {
    if (charCapacity <= capacity_) return true;
    char* nb = static_cast<char*>(psramAlloc(charCapacity));
    if (!nb) return false;
    if (buffer_ && used_ > 0) memcpy(nb, buffer_, used_);
    psramFree(buffer_);
    buffer_ = nb;
    capacity_ = static_cast<uint32_t>(charCapacity);
    return true;
  }

  void reserveWords(size_t wordCount) { offsets_.reserve(wordCount); }

  bool push_back(const char* word, size_t len) {
    const size_t needed = static_cast<size_t>(used_) + len + 1;
    if (needed > capacity_) {
      const size_t newCap = needed + (needed >> 1);
      if (!reserveBuffer(newCap)) return false;
    }
    offsets_.push_back(static_cast<uint32_t>(used_));
    memcpy(buffer_ + used_, word, len);
    buffer_[used_ + len] = '\0';
    used_ += static_cast<uint32_t>(len + 1);
    return true;
  }

  bool push_back(const String& word) {
    return push_back(word.c_str(), word.length());
  }

  size_t size() const { return offsets_.size(); }
  bool empty() const { return offsets_.empty(); }

  const char* rawAt(size_t i) const { return buffer_ + offsets_[i]; }

  String wordAt(size_t i) const { return String(rawAt(i)); }

  void clear() {
    psramFree(buffer_);
    buffer_ = nullptr;
    capacity_ = 0;
    used_ = 0;
    offsets_.clear();
  }

 private:
  static void* psramAlloc(size_t n) {
#ifdef ARDUINO
    return heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    return malloc(n);
#endif
  }
  static void psramFree(void* p) {
    if (!p) return;
#ifdef ARDUINO
    heap_caps_free(p);
#else
    free(p);
#endif
  }
};

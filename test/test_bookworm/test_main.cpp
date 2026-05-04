#include <unity.h>

#include <cstdint>

#include "bookworm/BookWormHatch.h"
#include "bookworm/BookWormSim.h"
#include "bookworm/BookWormState.h"

namespace {

uint32_t gSeq = 0;
uint32_t testRng() { return ++gSeq; }

}  // namespace

void setUp(void) {}

void tearDown(void) {}

void test_hatch_is_deterministic_with_fixed_rng() {
  bookworm::BookWormState s;
  gSeq = 100;
  bookworm::hatchEgg(s, 0, 0, testRng);
  TEST_ASSERT_TRUE(s.hatched);
  TEST_ASSERT_EQUAL_STRING("bruffkin", s.name);
  TEST_ASSERT_EQUAL_UINT8(0, s.styleId);
}

void test_tick_increases_needs_over_time() {
  bookworm::BookWormState s{};
  s.hatched = true;
  s.hunger = 200;
  s.boredom = 200;
  bookworm::BookWormSim::tick(s, 60UL * 60UL * 1000UL);
  TEST_ASSERT_GREATER_THAN(200, static_cast<int>(s.hunger));
  TEST_ASSERT_GREATER_THAN(200, static_cast<int>(s.boredom));
}

void test_reading_reduces_needs_and_counts_words() {
  bookworm::BookWormState s{};
  s.hatched = true;
  s.hunger = 500;
  s.boredom = 500;
  bookworm::BookWormSim::onReadingWords(s, 20);
  TEST_ASSERT_LESS_THAN(500, static_cast<int>(s.hunger));
  TEST_ASSERT_LESS_THAN(500, static_cast<int>(s.boredom));
  TEST_ASSERT_EQUAL_UINT32(20, s.totalWordsRead);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_hatch_is_deterministic_with_fixed_rng);
  RUN_TEST(test_tick_increases_needs_over_time);
  RUN_TEST(test_reading_reduces_needs_and_counts_words);
  return UNITY_END();
}

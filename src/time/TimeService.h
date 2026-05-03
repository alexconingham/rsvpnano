#pragma once

#include <Arduino.h>

class TimeService {
 public:
  /// Start SNTP client once per boot (safe to call multiple times).
  static void requestSntpOnce();

  static bool hasValidLocalTime();

  /// Local wall time HH:MM, or "--:--" if unknown.
  static String formatHHMM();

 private:
  static bool sntpStarted_;
};

#include "time/TimeService.h"

#include <sys/time.h>
#include <time.h>

bool TimeService::sntpStarted_ = false;

void TimeService::requestSntpOnce() {
  if (sntpStarted_) {
    return;
  }
  sntpStarted_ = true;
  setenv("TZ", "NZST-12NZDT,M9.5.0,M4.1.0/3", 1);
  tzset();
  // Arduino-ESP32 2.0.x exposes configTime() (LWIP SNTP); avoids esp_sntp_* API differences.
  configTime(0, 0, "pool.ntp.org", "time.google.com");
}

bool TimeService::hasValidLocalTime() {
  time_t now = time(nullptr);
  return now > 1700000000;
}

uint32_t TimeService::utcUnixSeconds() {
  time_t now = time(nullptr);
  if (now <= 1700000000) {
    return 0;
  }
  return static_cast<uint32_t>(now);
}

String TimeService::formatHHMM() {
  struct tm ti;
  if (!getLocalTime(&ti)) {
    return String("--:--");
  }
  char buf[8];
  strftime(buf, sizeof(buf), "%H:%M", &ti);
  return String(buf);
}

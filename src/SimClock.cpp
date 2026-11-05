#include "SimClock.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

void SimClock::advance(float dtSeconds) {
    if (!paused_ && dtSeconds > 0.0f) days_ += double(dtSeconds) * double(scale_);
}

void SimClock::setScale(float daysPerSecond) {
    if (!std::isfinite(daysPerSecond)) return;
    scale_ = std::clamp(daysPerSecond, kMinScale, kMaxScale);
}

// Day count -> Gregorian date (H. Hinnant's days_from_civil inverse). J2000 noon is 10957.5 days
// after 1970-01-01, so floor(days + 10957.5) is the day number counted from 1970-01-01.
std::string SimClock::date() const {
    long long z = (long long)std::floor(days_ + 10957.5) + 719468;
    long long era = (z >= 0 ? z : z - 146096) / 146097;
    long long doe = z - era * 146097;                                    // day of era [0, 146096]
    long long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;  // year of era [0, 399]
    long long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);             // day of year from 1 March
    long long mp = (5 * doy + 2) / 153;                                  // month from March [0, 11]
    long long day = doy - (153 * mp + 2) / 5 + 1;
    long long month = mp < 10 ? mp + 3 : mp - 9;
    long long year = yoe + era * 400 + (month <= 2 ? 1 : 0);
    char buf[64];
    std::snprintf(buf, sizeof buf, "%04lld-%02lld-%02lld", year, month, day);
    return buf;
}

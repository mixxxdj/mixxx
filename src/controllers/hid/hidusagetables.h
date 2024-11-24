#pragma once

#include <QString>

namespace mixxx {

namespace hid {

class HidUsageTables {
  public:
    HidUsageTables() = default;
    static QString getUsagePageDescription(uint16_t usagePage);
    static QString getUsageDescription(uint16_t usagePage, uint16_t usage);
};

} // namespace hid

} // namespace mixxx

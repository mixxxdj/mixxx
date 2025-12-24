#pragma once

#include <QString>

namespace mixxx {

namespace hid {

namespace HidUsageTables {

QString getUsagePageDescription(uint16_t usagePage);
QString getUsageDescription(uint16_t usagePage, uint16_t usage);

} // namespace HidUsageTables

} // namespace hid

} // namespace mixxx

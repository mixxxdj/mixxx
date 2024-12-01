#include "controllers/hid/hidusagetables.h"

#include "controllers/hid/hidusagetablesdata.h"

namespace {
// The HID Usage Tables 1.5 PDF specifies that the vendor-defined Usage-Page
// range is 0xFF00 to 0xFFFF.
constexpr uint16_t kStartOfVendorDefinedUsagePageRange = 0xFF00;

} // namespace

namespace mixxx {

namespace hid {

namespace HidUsageTables {

QString getUsagePageDescription(uint16_t usagePage) {
    if (usagePage >= kStartOfVendorDefinedUsagePageRange) {
        return QStringLiteral("Vendor-defined");
    }

    for (const auto& page : kHidUsagePages) {
        if (page.id == usagePage) {
            return page.name;
        }
    }
    return QStringLiteral("Reserved");
}

QString getUsageDescription(uint16_t usagePage, uint16_t usage) {
    if (usagePage >= kStartOfVendorDefinedUsagePageRange) {
        return QStringLiteral("Vendor-defined");
    }

    for (const auto& page : kHidUsagePages) {
        if (page.id == usagePage) {
            for (const auto& usageItem : page.usages) {
                if (usageItem.id == usage) {
                    return usageItem.name;
                }
            }
            break; // No need to continue if the usage page is found
        }
    }
    return QStringLiteral("Reserved");
}

} // namespace HidUsageTables

} // namespace hid

} // namespace mixxx

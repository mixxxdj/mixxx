#include "controllers/hid/hidreportdescriptor.h"

#include <algorithm>
#include <cstdint>
#include <optional>

#include "moc_hidreportdescriptor.cpp"
#include "util/assert.h"

namespace hid::reportDescriptor {

/// Extracts the value of the specified control in logical scale,
/// from the given report data
int32_t extractLogicalValue(const QByteArray& reportData, const Control& control) {
    VERIFY_OR_DEBUG_ASSERT(control.m_bitSize > 0 && control.m_bitSize <= 32) {
        return control.m_logicalMinimum; // Safe value in allowed range
    }

    uint8_t numberOfBytesToCopy = ((control.m_bitPosition + control.m_bitSize - 1) / 8) + 1;

    int64_t value = 0;
    std::memcpy(&value, reportData.data() + control.m_bytePosition, numberOfBytesToCopy);

    value >>= control.m_bitPosition;

    bool isSigned = control.m_logicalMinimum < 0;
    // Mask out the bits that are not part of the control
    if (isSigned) {
        bool isNegative = value & (1ULL << (control.m_bitSize - 1));
        value &= (1ULL << (control.m_bitSize - 1)) - 1;
        if (isNegative) {
            value = ((1ULL << (control.m_bitSize - 1)) - value) * -1;
        }
    } else {
        value &= (1ULL << control.m_bitSize) - 1;
    }

    return value;
}

/// Sets the bits in the report data of the specified control,
/// to the given value in logical scale
bool applyLogicalValue(QByteArray& reportData, const Control& control, int32_t controlValue) {
    VERIFY_OR_DEBUG_ASSERT(control.m_bitSize > 0 && control.m_bitSize <= 32) {
        return false;
    }

    if (control.m_flags.no_null_null) {
        // Nullable controls allow any possible value in the bitrange
        if (control.m_logicalMinimum < 0) {
            if (controlValue < -std::pow(2, control.m_bitSize - 1) ||
                    controlValue > std::pow(2, control.m_bitSize - 1) - 1) {
                return false;
            }
        } else {
            if (controlValue < 0 || controlValue > std::pow(2, control.m_bitSize)) {
                return false;
            }
        }
    } else {
        // Non-Nullable controls only allow values in the logical range
        if (controlValue < control.m_logicalMinimum || controlValue > control.m_logicalMaximum) {
            return false;
        }
    }

    uint64_t mask = ((1ULL << control.m_bitSize) - 1) << control.m_bitPosition;
    uint64_t value = (static_cast<uint64_t>(controlValue) << control.m_bitPosition) & mask;

    // Check if the value fits into the data (position + bitSize)
    uint8_t lastByteToCopy = control.m_bytePosition +
            (control.m_bitPosition + control.m_bitSize - 1) / 8;

    if (lastByteToCopy >= reportData.size()) {
        return false;
    }

    for (uint8_t byteIdx = control.m_bytePosition; byteIdx <= lastByteToCopy; ++byteIdx) {
        // Clear the bits that are part of the control
        reportData[byteIdx] &= ~static_cast<uint8_t>(mask & 0xFF);
        // Set the new value
        reportData[byteIdx] |= static_cast<uint8_t>(value & 0xFF);
        mask >>= 8;
        value >>= 8;
    }

    return true;
}

QString getScaledUnitString(uint32_t unit) {
    struct UnitInfo {
        const char* pPhysicalQuantity[5];
    };

    // This table of physical units is derived from the unit item table
    // in chapter 6.2.2.7 of HID class definition 1.11
    // These official unit symbols are not translateable
    const UnitInfo unitInfos[] = {
            {"", "cm", "rad", "″", "°"},    // length/angle
            {"", "g", "g", "slug", "slug"}, // mass
            {"", "s", "s", "s", "s"},       // time
            {"", "K", "K", "°F", "°F"},     // temperature
            {"", "A", "A", "A", "A"},       // current
            {"", "cd", "cd", "cd", "cd"}    // luminous intensity
    };

    int8_t exponents[] = {0, 1, 2, 3, 4, 5, 6, 7, -8, -7, -6, -5, -4, -3, -2, -1};

    QString unitString;

    auto appendQuantity = [&](int shift, const UnitInfo& unitInfo) {
        int8_t value = (unit >> shift) & 0xF;
        if (value != 0) {
            if (!unitString.isEmpty()) {
                unitString += "*";
            }
            unitString += unitInfo.pPhysicalQuantity[(unit & 0xF)];
            if (exponents[value] != 1) {
                unitString += "^" + QString::number(exponents[value]);
            }
        }
    };

    for (int quantityIdx = 0; quantityIdx < 6; ++quantityIdx) {
        appendQuantity(4 + quantityIdx * 4, unitInfos[quantityIdx]);
    }

    return unitString;
}

// Class for Controls

Control::Control(const ControlFlags flags,
        const uint32_t usage,
        const int32_t logicalMinimum,
        const int32_t logicalMaximum,
        const int32_t physicalMinimum,
        const int32_t physicalMaximum,
        const int8_t unitExponent,
        const uint32_t unit,
        const uint16_t bytePosition,
        const uint8_t bitPosition,
        const uint8_t bitSize)
        : m_flags(flags),
          m_usage(usage),
          m_logicalMinimum(logicalMinimum),
          m_logicalMaximum(logicalMaximum),
          m_physicalMinimum(physicalMinimum),
          m_physicalMaximum(physicalMaximum),
          m_unitExponent(unitExponent),
          m_unit(unit),
          m_bytePosition(bytePosition),
          m_bitPosition(bitPosition),
          m_bitSize(bitSize) {
}

Report::Report(const HidReportType& reportType, const uint8_t& reportId)
        : m_reportType(reportType),
          m_reportId(reportId),
          m_lastBytePosition(0),
          m_lastBitPosition(0) {
}

void Report::addControl(const Control& item) {
    m_controls.push_back(item);
}

void Report::increasePosition(unsigned int bitSize) {
    // Calculate the new bit position
    m_lastBitPosition += bitSize;

    // If the bit position exceeds 8 bits, adjust the byte position
    m_lastBytePosition += m_lastBitPosition / 8;
    m_lastBitPosition %= 8;
}

// Class for Collections
void Collection::addReport(const Report& report) {
    m_reports.push_back(report);
}

std::optional<std::reference_wrapper<const hid::reportDescriptor::Report>>
Collection::getReport(
        const HidReportType& reportType, const uint8_t& reportId) const {
    for (const auto& report : m_reports) {
        if (report.m_reportType == reportType && report.m_reportId == reportId) {
            return report;
        }
    }
    return std::nullopt;
}

// HID Report Descriptor Parser
HidReportDescriptor::HidReportDescriptor(const std::vector<uint8_t>& data)
        : m_data(data),
          m_pos(0),
          m_deviceUsesReportIds(kNotSet),
          m_collectionLevel(0) {
}

std::pair<HidItemTag, HidItemSize> HidReportDescriptor::readTag() {
    uint8_t byte = m_data[m_pos++];

    VERIFY_OR_DEBUG_ASSERT(byte !=
            static_cast<uint8_t>(HidItemSize::LongItemKeyword)){
            // Long items are only reserved for future use, they can't be used
            // according to HID class definition 1.11
    };

    HidItemTag tag = static_cast<HidItemTag>(
            byte & static_cast<uint8_t>(HidItemTag::AllTagBitsMask));
    HidItemSize size = static_cast<HidItemSize>(
            byte & static_cast<uint8_t>(HidItemSize::AllSizeBitsMask));

    return {tag, size};
}

uint32_t HidReportDescriptor::readPayload(HidItemSize payloadSize) {
    uint32_t payload;

    switch (payloadSize) {
    case HidItemSize::ZeroBytePayload:
        return 0;
    case HidItemSize::OneBytePayload:
        VERIFY_OR_DEBUG_ASSERT(m_pos + 1 <= m_data.size()) {
            return 0;
        }
        return m_data[m_pos++];
    case HidItemSize::TwoBytePayload:
        VERIFY_OR_DEBUG_ASSERT(m_pos + 2 <= m_data.size()) {
            return 0;
        }
        payload = m_data[m_pos++];
        payload |= m_data[m_pos++] << 8;
        return payload;
    case HidItemSize::FourBytePayload:
        VERIFY_OR_DEBUG_ASSERT(m_pos + 4 <= m_data.size()) {
            return 0;
        }
        payload = m_data[m_pos++];
        payload |= m_data[m_pos++] << 8;
        payload |= m_data[m_pos++] << 16;
        payload |= m_data[m_pos++] << 24;
        return payload;
    default:
        DEBUG_ASSERT(true);
        return 0;
    }
}

int32_t HidReportDescriptor::getSignedValue(uint32_t payload, HidItemSize payloadSize) {
    switch (payloadSize) {
    case HidItemSize::ZeroBytePayload:
        return 0;
    case HidItemSize::OneBytePayload:
        if (payload & 0x80) {                                  // Check if the sign bit is set
            return static_cast<int32_t>(payload | 0xFFFFFF00); // Sign extend to 32 bits
        }
        return static_cast<int32_t>(payload);
    case HidItemSize::TwoBytePayload:
        if (payload & 0x8000) {                                // Check if the sign bit is set
            return static_cast<int32_t>(payload | 0xFFFF0000); // Sign extend to 32 bits
        }
        return static_cast<int32_t>(payload);
    case HidItemSize::FourBytePayload:
        return static_cast<int32_t>(payload); // Already 32 bits, no need to sign extend
    default:
        DEBUG_ASSERT(true);
        return 0;
    }
}

uint32_t HidReportDescriptor::getDecodedUsage(
        uint16_t usagePage, uint32_t usage, HidItemSize usageSize) {
    switch (usageSize) {
    case HidItemSize::ZeroBytePayload:
        return usagePage << 16;
    case HidItemSize::OneBytePayload:
    case HidItemSize::TwoBytePayload:
        return (usagePage << 16) + usage;
    case HidItemSize::FourBytePayload:
        return usage; // Full 32bit usage superseds Usage Page
    default:
        DEBUG_ASSERT(true);
        return usagePage << 16;
    }
}

HidReportType HidReportDescriptor::getReportType(HidItemTag tag) {
    switch (tag) {
    case HidItemTag::Input:
        return HidReportType::Input;
    case HidItemTag::Output:
        return HidReportType::Output;
        break;
    case HidItemTag::Feature:
        return HidReportType::Feature;
    default:
        DEBUG_ASSERT(true);
        return HidReportType::Input; // Dummy value for error case
    }
}

Collection HidReportDescriptor::parse() {
    Collection collection;                            // Top level collection
    std::unique_ptr<Report> pCurrentReport = nullptr; // Use a unique_ptr for pCurrentReport

    // Global item values
    GlobalItems globalItems;

    // Local item values
    LocalItems localItems;

    while (m_pos < m_data.size()) {
        auto [tag, size] = readTag();
        auto payload = readPayload(size);

        switch (tag) {
        // Global Items
        case HidItemTag::UsagePage:
            globalItems.usagePage = payload;
            break;
        case HidItemTag::LogicalMinimum:
            globalItems.logicalMinimum = getSignedValue(payload, size);
            break;
        case HidItemTag::LogicalMaximum:
            globalItems.logicalMaximum = getSignedValue(payload, size);
            break;
        case HidItemTag::PhysicalMinimum:
            globalItems.physicalMinimum = getSignedValue(payload, size);
            break;
        case HidItemTag::PhysicalMaximum:
            globalItems.physicalMaximum = getSignedValue(payload, size);
            break;
        case HidItemTag::UnitExponent:
            // HID class definition restricts the unit exponent range to -8 to +7
            globalItems.unitExponent = static_cast<int8_t>(payload & 0x0F);
            if (globalItems.unitExponent >= 8) {
                globalItems.unitExponent -= 16;
            }
            break;
        case HidItemTag::Unit:
            globalItems.unit = payload;
            break;
        case HidItemTag::ReportSize:
            globalItems.reportSize = payload;
            break;
        case HidItemTag::ReportId:
            globalItems.reportId = static_cast<uint8_t>(payload);
            break;
        case HidItemTag::ReportCount:
            globalItems.reportCount = payload;
            break;
        case HidItemTag::Push:
            // Places a copy of the global item state table on the stack
            globalItemsStack.push_back(globalItems);
            break;
        case HidItemTag::Pop:
            // Replaces the item state table with the top structure from the stack
            VERIFY_OR_DEBUG_ASSERT(!globalItemsStack.empty()) {
                globalItems = globalItemsStack.back();
                globalItemsStack.pop_back();
            }
            break;

        // Local Items
        case HidItemTag::Usage:
            localItems.Usage.push_back(getDecodedUsage(globalItems.usagePage, payload, size));
            break;
        case HidItemTag::UsageMinimum:
            localItems.UsageMinimum = getDecodedUsage(globalItems.usagePage, payload, size);
            break;
        case HidItemTag::UsageMaximum:
            localItems.UsageMaximum = getDecodedUsage(globalItems.usagePage, payload, size);
            break;
        case HidItemTag::DesignatorIndex:
            localItems.DesignatorIndex = payload;
            break;
        case HidItemTag::DesignatorMinimum:
            localItems.DesignatorMinimum = payload;
            break;
        case HidItemTag::DesignatorMaximum:
            localItems.DesignatorMaximum = payload;
            break;
        case HidItemTag::StringIndex:
            localItems.StringIndex = payload;
            break;
        case HidItemTag::StringMinimum:
            localItems.StringMinimum = payload;
            break;
        case HidItemTag::StringMaximum:
            localItems.StringMaximum = payload;
            break;
        case HidItemTag::Delimiter:
            localItems.Delimiter = payload;
            break;

        // Main Items
        case HidItemTag::Input:
        case HidItemTag::Output:
        case HidItemTag::Feature: {
            if (pCurrentReport == nullptr) {
                // First control of this device
                if (globalItems.reportId == kNoReportId) {
                    m_deviceUsesReportIds = false;
                } else {
                    m_deviceUsesReportIds = true;
                }
                pCurrentReport = std::make_unique<Report>(getReportType(tag), globalItems.reportId);
            } else if (pCurrentReport->m_reportType != getReportType(tag) ||
                    globalItems.reportId != pCurrentReport->m_reportId) {
                // First control of a new report
                collection.addReport(*pCurrentReport);
                pCurrentReport = std::make_unique<Report>(getReportType(tag), globalItems.reportId);
            }

            int32_t physicalMinimum;
            int32_t physicalMaximum;
            if (globalItems.physicalMinimum == 0 && globalItems.physicalMaximum == 0) {
                // According remark in chapter 6.2.2.7 of HID class definition 1.11
                physicalMinimum = globalItems.logicalMinimum;
                physicalMaximum = globalItems.logicalMaximum;
            } else {
                physicalMinimum = globalItems.physicalMinimum;
                physicalMaximum = globalItems.physicalMaximum;
            }

            auto flags = std::bit_cast<ControlFlags>(payload);

            if (flags.data_constant == 1) {
                // Constant value padding - Usually for byte alignment
                pCurrentReport->increasePosition(globalItems.reportSize * globalItems.reportCount);
            } else if (flags.array_variable == 0) {
                // Array (e.g. list of pressed keys of a computer keyboard)
                // NOT IMPLEMENTED as not relevant for mapping wizard,
                // but could be implemented by overloaded Control class
                pCurrentReport->increasePosition(globalItems.reportSize * globalItems.reportCount);
            } else {
                // Normal variable control
                uint32_t usage = 0;
                unsigned int numOfControls =
                        (localItems.UsageMinimum != kNotSet &&
                                localItems.UsageMaximum != kNotSet)
                        ? localItems.UsageMaximum - localItems.UsageMinimum + 1
                        : globalItems.reportCount;
                for (unsigned int controlIdx = 0;
                        controlIdx < numOfControls;
                        controlIdx++) {
                    if (localItems.UsageMinimum != kNotSet && localItems.UsageMaximum != kNotSet) {
                        if (controlIdx == 0) {
                            usage = localItems.UsageMinimum;
                        } else if (usage < localItems.UsageMaximum) {
                            usage++;
                        }
                    } else if (!localItems.Usage.empty()) {
                        // If there are less usages than reportCount,
                        // the last usage value is valid for the remaining
                        usage = localItems.Usage.front();
                        localItems.Usage.erase(localItems.Usage.begin());
                    }

                    Control control(flags,
                            usage,
                            globalItems.logicalMinimum,
                            globalItems.logicalMaximum,
                            physicalMinimum,
                            physicalMaximum,
                            globalItems.unitExponent,
                            globalItems.unit,
                            pCurrentReport->getLastBytePosition(),
                            pCurrentReport->getLastBitPosition(),
                            globalItems.reportSize);
                    pCurrentReport->addControl(control);
                    pCurrentReport->increasePosition(globalItems.reportSize);
                }
                pCurrentReport->increasePosition(
                        (globalItems.reportCount - numOfControls) *
                        globalItems.reportSize);
            }

            localItems = LocalItems();
            break;
        }

        case HidItemTag::Collection:
            m_collectionLevel++;

            // We only handle top-level-collections
            // according to chapter 8.4 "Report Constraints" HID class definition 1.11
            if (m_collectionLevel == 1) {
                DEBUG_ASSERT(payload == static_cast<uint32_t>(CollectionType::Application));
            }
            // Local items are only valid for the actual control definition, reset them
            localItems = LocalItems();
            break;
        case HidItemTag::EndCollection:
            if (m_collectionLevel == 1) {
                if (pCurrentReport) {
                    collection.addReport(*pCurrentReport);
                    pCurrentReport.reset();
                }
                m_topLevelCollections.push_back(collection);
                collection = Collection();
            }
            if (m_collectionLevel > 0) {
                m_collectionLevel--;
            }
            break;

        default:
            DEBUG_ASSERT(true);
            break;
        }
    }

    if (pCurrentReport) {
        collection.addReport(*pCurrentReport);
    }

    return collection;
}

std::optional<std::reference_wrapper<const hid::reportDescriptor::Report>>
HidReportDescriptor::getReport(
        const HidReportType& reportType, const uint8_t& reportId) const {
    for (const auto& collection : m_topLevelCollections) {
        auto report = collection.getReport(reportType, reportId);
        if (report) {
            return report;
        }
    }
    return std::nullopt;
}

std::vector<std::tuple<size_t, HidReportType, uint8_t>>
HidReportDescriptor::getListOfReports() const {
    std::vector<std::tuple<size_t, HidReportType, uint8_t>> orderedList;
    for (size_t i = 0; i < m_topLevelCollections.size(); ++i) {
        for (const auto& report : m_topLevelCollections[i].getReports()) {
            orderedList.emplace_back(i, report.m_reportType, report.m_reportId);
        }
    }
    return orderedList;
}

} // namespace hid::reportDescriptor

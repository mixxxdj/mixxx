#pragma once

#include <qobjectdefs.h>

#include <QMetaType>
#include <QString>
#include <cstdint>
#include <vector>

namespace hid::reportDescriptor {

Q_NAMESPACE

QString getScaledUnitString(uint32_t unit);

constexpr int kNotSet = -1;
// Value used instead of the ReportID, if device don't have ReportIDs
constexpr int kNoReportId = 0x00;

enum class HidReportType {
    Input,
    Output,
    Feature
};
Q_ENUM_NS(HidReportType)

// clang-format off
// Enum class for HID Item Tags (incl. the two type bits)
enum class HidItemTag : uint8_t {
    // "Main Items" according to chapter 6.2.2.4 of HID class definition 1.11
    Input             = 0b1000'00'00,
    Output            = 0b1001'00'00,
    Feature           = 0b1011'00'00,

    Collection        = 0b1010'00'00,
    EndCollection     = 0b1100'00'00,

    // "Global Items" according to chapter 6.2.2.7 of HID class definition 1.11
    UsagePage         = 0b0000'01'00,

    LogicalMinimum    = 0b0001'01'00,
    LogicalMaximum    = 0b0010'01'00,
    PhysicalMinimum   = 0b0011'01'00,
    PhysicalMaximum   = 0b0100'01'00,

    UnitExponent      = 0b0101'01'00,
    Unit              = 0b0110'01'00,

    ReportSize        = 0b0111'01'00,
    ReportId          = 0b1000'01'00,
    ReportCount       = 0b1001'01'00,

    Push              = 0b1010'01'00,
    Pop               = 0b1011'01'00,

    // "Local Items" according to chapter 6.2.2.8 of HID class definition 1.11
    Usage             = 0b0000'10'00,
    UsageMinimum      = 0b0001'10'00,
    UsageMaximum      = 0b0010'10'00,
        
    DesignatorIndex   = 0b0011'10'00,
    DesignatorMinimum = 0b0100'10'00,
    DesignatorMaximum = 0b0101'10'00,
        
    StringIndex       = 0b0111'10'00,
    StringMinimum     = 0b1000'10'00,
    StringMaximum     = 0b1001'10'00,
        
    Delimiter         = 0b1010'10'00,
        

    AllTagBitsMask    = 0b1111'11'00
};

// Enum class for HID Item Sizes
enum class HidItemSize : uint8_t {
    // "Short Items" sizes according to chapter 6.2.2.2 of HID class definition 1.11
    ZeroBytePayload   = 0b0000'00'00,
    OneBytePayload    = 0b0000'00'01,
    TwoBytePayload    = 0b0000'00'10,
    FourBytePayload   = 0b0000'00'11,

    
    // Special value for "Long Items" according to chapter 6.2.2.3 of HID class definition 1.11
    LongItemKeyword   = 0b1111'11'10,

    AllSizeBitsMask   = 0b0000'00'11
};

// Collection types according to chapter 6.2.2.6 of HID class definition 1.11
enum class CollectionType : uint8_t {
    Physical      = 0x00, // e.g. group of axes
    Application   = 0x01, // e.g. mouse or keyboard
    Logical       = 0x02, // interrelated data
    Report        = 0x03,
    NamedArray    = 0x04,
    UsageSwitch   = 0x05,
    UsageModifier = 0x06,
    Reserved      = 0x07, // range of 0x07-0x7F
    VendorDefined = 0x80  // range of 0x80-0xFF
};
// clang-format on

struct ControlFlags {
    uint32_t data_constant : 1;          // Data (0) | Constant (1)
    uint32_t array_variable : 1;         // Array (0) | Variable (1)
    uint32_t absolute_relative : 1;      // Absolute (0) | Relative (1)
    uint32_t no_wrap_wrap : 1;           // No Wrap (0) | Wrap (1)
    uint32_t linear_non_linear : 1;      // Linear (0) | Non Linear (1)
    uint32_t preferred_no_preferred : 1; // Preferred State (0) | No Preferred (1)
    uint32_t no_null_null : 1;           // No Null position (0) | Null state(1)
    uint32_t non_volatile_volatile : 1;  // Non Volatile (0) | Volatile (1)
    uint32_t bit_field_buffered : 1;     // Bit Field (0) | Buffered Bytes (1)
    uint32_t reserved : 23;
};

// Class representing a control described in the HID report descriptor
class Control {
  public:
    Control(const ControlFlags flags,
            const uint32_t usage,
            const int32_t logicalMinimum,
            const int32_t logicalMaximum,
            const int32_t physicalMinimum,
            const int32_t physicalMaximum,
            const int8_t unitExponent,
            const uint32_t unit,
            const uint16_t bytePosition, // Position of the first byte in the report
            const uint8_t bitPosition,   // Position of first bit in first byte
            const uint8_t bitSize);

    const ControlFlags m_flags;

    const uint32_t m_usage;
    const int32_t m_logicalMinimum;
    const int32_t m_logicalMaximum;
    const int32_t m_physicalMinimum;
    const int32_t m_physicalMaximum;
    const int8_t m_unitExponent;
    const uint32_t m_unit;
    const uint16_t m_bytePosition; // Position of the first byte in the report
    const uint8_t m_bitPosition;   // Position of first bit in first byte
    const uint8_t m_bitSize;

  private:
};

int32_t extractLogicalValue(const QByteArray& data, const Control& control);
bool applyLogicalValue(QByteArray& data, const Control& control, int32_t controlValue);

// Class representing a report in the HID report descriptor
class Report {
  public:
    Report(const HidReportType& reportType, const uint8_t& reportId);

    void addControl(const Control& item);
    void increasePosition(unsigned int bitSize);
    uint16_t getLastBytePosition() const {
        return m_lastBytePosition;
    }
    uint8_t getLastBitPosition() const {
        return m_lastBitPosition;
    }

    const std::vector<Control>& getControls() const {
        return m_controls;
    }

    const HidReportType m_reportType;
    const uint8_t m_reportId;
    uint16_t getReportSize() const {
        return m_lastBytePosition;
    }

  private:
    std::vector<Control> m_controls;
    uint16_t m_lastBytePosition;
    uint8_t m_lastBitPosition; // Last bit position inside last byte
};

// Class representing a collection of HID items
class Collection {
  public:
    Collection() = default;
    void addReport(const Report& report);
    const Report* getReport(const HidReportType& reportType, const uint8_t& reportId) const;
    const std::vector<Report>& getReports() const {
        return m_reports;
    }

  private:
    std::vector<Report> m_reports;
};

// Class for parsing HID report descriptors
class HidReportDescriptor {
  public:
    HidReportDescriptor(const uint8_t* pData, size_t length);

    bool isDeviceWithReportIds() const {
        return m_deviceHasReportIds;
    }

    Collection parse();
    const Report* getReport(const HidReportType& reportType, const uint8_t& reportId) const;
    const std::vector<std::tuple<size_t, HidReportType, uint8_t>> getListOfReports() const;

  private:
    // Define the struct for global items
    struct GlobalItems {
        uint16_t usagePage = 0;
        int32_t logicalMinimum = 0;
        int32_t logicalMaximum = 0;
        int32_t physicalMinimum = 0;
        int32_t physicalMaximum = 0;
        int8_t unitExponent = 0;
        uint32_t unit = 0;
        uint32_t reportSize = 0;
        uint8_t reportId = 0;
        uint32_t reportCount = 0;
    };

    struct LocalItems {
        std::vector<int64_t> Usage;
        int64_t UsageMinimum = kNotSet;
        int64_t UsageMaximum = kNotSet;
        int64_t DesignatorIndex = kNotSet;
        int64_t DesignatorMinimum = kNotSet;
        int64_t DesignatorMaximum = kNotSet;
        int64_t StringIndex = kNotSet;
        int64_t StringMinimum = kNotSet;
        int64_t StringMaximum = kNotSet;
        int64_t Delimiter = kNotSet;
    };

    std::pair<HidItemTag, HidItemSize> readTag();
    uint32_t readPayload(HidItemSize payloadSize);

    int32_t getSignedValue(uint32_t payload, HidItemSize payloadSize);
    uint32_t getDecodedUsage(uint16_t usagePage, uint32_t usage, HidItemSize usageSize);

    HidReportType getReportType(HidItemTag tag);

    const uint8_t* m_pData;
    size_t m_length;
    size_t m_pos;

    bool m_deviceHasReportIds;

    std::vector<GlobalItems> globalItemsStack;

    unsigned int m_collectionLevel;
    std::vector<Collection> m_topLevelCollections;
};

} // namespace hid::reportDescriptor

Q_DECLARE_METATYPE(hid::reportDescriptor::Control);

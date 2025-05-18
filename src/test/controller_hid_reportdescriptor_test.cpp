#include <gtest/gtest.h>

#include <limits>
#include <vector>

#include "controllers/hid/hidreportdescriptor.h"

using namespace hid::reportDescriptor;

// Example HID report descriptor data

// clang-format off
uint8_t reportDescriptor[] = {
        0x05, 0x01, // Usage Page (Generic Desktop)
        0x09, 0x02, // Usage (Mouse)
        0xA1, 0x01, // Collection (Application)
        0x09, 0x01,  // Usage (Pointer)
        0xA1, 0x00,  // Collection (Physical)
        0x05, 0x09,   // Usage Page (Button)
        0x19, 0x01,   // Usage Minimum (1)
        0x29, 0x03,   // Usage Maximum (3)
        0x15, 0x00,   // Logical Minimum (0)
        0x25, 0x01,   // Logical Maximum (1)
        0x95, 0x03,   // Report Count (3)
        0x75, 0x01,   // Report Size (1)
        0x81, 0x02,   // Input (Data, Variable, Absolute)
        0x95, 0x01,   // Report Count (1)
        0x75, 0x05,   // Report Size (5)
        0x81, 0x01,   // Input (Constant)
        0x05, 0x01,   // Usage Page (Generic Desktop)
        0x09, 0x30,   // Usage (X)
        0x09, 0x31,   // Usage (Y)
        0x15, 0x81,   // Logical Minimum (-127)
        0x25, 0x7F,   // Logical Maximum (127)
        0x75, 0x08,   // Report Size (8)
        0x95, 0x02,   // Report Count (2)
        0x81, 0x06,   // Input (Data, Variable, Relative)
        0xC0,        // End Collection
        0xC0        // End Collection
};
// clang-format on

TEST(HidReportDescriptorParserTest, ParseReportDescriptor) {
    const std::vector<uint8_t> reportDescriptorVector(
            reportDescriptor, reportDescriptor + sizeof(reportDescriptor));
    HidReportDescriptor parser(reportDescriptorVector);
    Collection collection = parser.parse();

    // Use getListOfReports to get the list of reports
    auto reportsList = parser.getListOfReports();
    ASSERT_EQ(reportsList.size(), 1);

    auto& [collectionIdx, reportType, reportId] = reportsList[0];
    ASSERT_EQ(collectionIdx, 0);

    // Use getReport to get the report
    auto reportOpt = parser.getReport(reportType, reportId);
    ASSERT_TRUE(reportOpt.has_value());

    const auto& report = reportOpt->get();

    // Validate Report fields
    ASSERT_EQ(report.m_reportType, reportType);
    ASSERT_EQ(report.m_reportId, reportId);

    // Validate all Control fields
    const std::vector<Control>& controls = report.getControls();
    ASSERT_EQ(controls.size(), 5);

    // Mouse Button 1
    EXPECT_EQ(controls[0].m_usage, 0x0009'0001);
    EXPECT_EQ(controls[0].m_logicalMinimum, 0);
    EXPECT_EQ(controls[0].m_logicalMaximum, 1);
    EXPECT_EQ(controls[0].m_physicalMinimum, 0);
    EXPECT_EQ(controls[0].m_physicalMaximum, 1);
    EXPECT_EQ(controls[0].m_unitExponent, 0);
    EXPECT_EQ(controls[0].m_unit, 0);
    EXPECT_EQ(controls[0].m_bytePosition, 0);
    EXPECT_EQ(controls[0].m_bitPosition, 0);
    EXPECT_EQ(controls[0].m_bitSize, 1);

    // Mouse Button 2
    EXPECT_EQ(controls[1].m_usage, 0x0009'0002);
    EXPECT_EQ(controls[1].m_logicalMinimum, 0);
    EXPECT_EQ(controls[1].m_logicalMaximum, 1);
    EXPECT_EQ(controls[1].m_physicalMinimum, 0);
    EXPECT_EQ(controls[1].m_physicalMaximum, 1);
    EXPECT_EQ(controls[1].m_unitExponent, 0);
    EXPECT_EQ(controls[1].m_unit, 0);
    EXPECT_EQ(controls[1].m_bytePosition, 0);
    EXPECT_EQ(controls[1].m_bitPosition, 1);
    EXPECT_EQ(controls[1].m_bitSize, 1);

    // Mouse Button 3
    EXPECT_EQ(controls[2].m_usage, 0x0009'0003);
    EXPECT_EQ(controls[2].m_logicalMinimum, 0);
    EXPECT_EQ(controls[2].m_logicalMaximum, 1);
    EXPECT_EQ(controls[2].m_physicalMinimum, 0);
    EXPECT_EQ(controls[2].m_physicalMaximum, 1);
    EXPECT_EQ(controls[2].m_unitExponent, 0);
    EXPECT_EQ(controls[2].m_unit, 0);
    EXPECT_EQ(controls[2].m_bytePosition, 0);
    EXPECT_EQ(controls[2].m_bitPosition, 2);
    EXPECT_EQ(controls[2].m_bitSize, 1);

    // Mouse Movement X
    EXPECT_EQ(controls[3].m_usage, 0x0001'0030);
    EXPECT_EQ(controls[3].m_logicalMinimum, -127);
    EXPECT_EQ(controls[3].m_logicalMaximum, 127);
    EXPECT_EQ(controls[3].m_physicalMinimum, -127);
    EXPECT_EQ(controls[3].m_physicalMaximum, 127);
    EXPECT_EQ(controls[3].m_unitExponent, 0);
    EXPECT_EQ(controls[3].m_unit, 0);
    EXPECT_EQ(controls[3].m_bitSize, 8);
    EXPECT_EQ(controls[3].m_bytePosition, 1);
    EXPECT_EQ(controls[3].m_bitPosition, 0);

    // Mouse Movement Y
    EXPECT_EQ(controls[4].m_usage, 0x0001'0031);
    EXPECT_EQ(controls[4].m_logicalMinimum, -127);
    EXPECT_EQ(controls[4].m_logicalMaximum, 127);
    EXPECT_EQ(controls[4].m_physicalMinimum, -127);
    EXPECT_EQ(controls[4].m_physicalMaximum, 127);
    EXPECT_EQ(controls[4].m_unitExponent, 0);
    EXPECT_EQ(controls[4].m_unit, 0);
    EXPECT_EQ(controls[4].m_bitSize, 8);
    EXPECT_EQ(controls[4].m_bytePosition, 2);
    EXPECT_EQ(controls[4].m_bitPosition, 0);
}

TEST(HidReportDescriptorTest, ControlValue_1Bit) {
    auto reportData = QByteArray::fromHex("81'00'00'FF'01");
    Control control({0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Flags
            0x0009'0001,                            // UsagePage/Usage
            0,                                      // LogicalMinimum
            1,                                      // LogicalMaximum
            0,                                      // PhysicalMinimum
            1,                                      // PhysicalMaximum
            0,                                      // UnitExponent
            0,                                      // Unit
            3,                                      // BytePosition
            0,                                      // BitPosition
            1);                                     // BitSize

    int32_t value = extractLogicalValue(reportData, control);
    EXPECT_EQ(value, 0x1);

    bool result = applyLogicalValue(reportData, control, 0);
    EXPECT_TRUE(result);
    EXPECT_EQ(reportData, QByteArray::fromHex("81'00'00'FE'01"));

    int32_t value2 = extractLogicalValue(reportData, control);
    EXPECT_EQ(value2, 0x0);
}

TEST(HidReportDescriptorTest, ControlValue_unsigned11Bits) {
    auto reportData = QByteArray::fromHex("81'30'46'00'01");
    Control control({0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Flags
            0x0009'0001,                            // UsagePage/Usage
            0,                                      // LogicalMinimum
            2047,                                   // LogicalMaximum
            0,                                      // PhysicalMinimum
            2047,                                   // PhysicalMaximum
            0,                                      // UnitExponent
            0,                                      // Unit
            1,                                      // BytePosition
            2,                                      // BitPosition
            11);                                    // BitSize

    int32_t value = extractLogicalValue(reportData, control);
    EXPECT_EQ(value, 0b001'1000'1100);

    bool result = applyLogicalValue(reportData, control, 0b010'1010'1010);
    EXPECT_TRUE(result);
    EXPECT_EQ(reportData, QByteArray::fromHex("81'A8'4A'00'01"));

    int32_t value2 = extractLogicalValue(reportData, control);
    EXPECT_EQ(value2, 0b010'1010'1010);
}

TEST(HidReportDescriptorTest, ControlValue_signed11Bits) {
    auto reportData = QByteArray::fromHex("AA'BB'CC'DD'EE");
    Control control({0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Flags
            0x0009'0001,                            // UsagePage/Usage
            -1000,                                  // LogicalMinimum
            1000,                                   // LogicalMaximum
            -10,                                    // PhysicalMinimum
            10,                                     // PhysicalMaximum
            0,                                      // UnitExponent
            0,                                      // Unit
            2,                                      // BytePosition
            0,                                      // BitPosition
            11);                                    // BitSize

    int32_t value = extractLogicalValue(reportData, control);
    EXPECT_EQ(value, -564);

    bool result = applyLogicalValue(reportData, control, +200);
    EXPECT_TRUE(result);
    EXPECT_EQ(reportData, QByteArray::fromHex("AA'BB'C8'D8'EE"));

    int32_t value2 = extractLogicalValue(reportData, control);
    EXPECT_EQ(value2, +200);

    bool result2 = applyLogicalValue(reportData, control, -200);
    EXPECT_TRUE(result2);
    EXPECT_EQ(reportData, QByteArray::fromHex("AA'BB'38'DF'EE"));

    int32_t value3 = extractLogicalValue(reportData, control);
    EXPECT_EQ(value3, -200);
}

TEST(HidReportDescriptorTest, ControlValue_unsigned32Bits) {
    auto reportData = QByteArray::fromHex("0A'21'43'65'B7");
    Control control({0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Flags
            0x0009'0001,                            // UsagePage/Usage
            0,                                      // LogicalMinimum
            0x7FFFFFFF,                             // LogicalMaximum
            0,                                      // PhysicalMinimum
            0x7FFFFFFF,                             // PhysicalMaximum
            0,                                      // UnitExponent
            0,                                      // Unit
            0,                                      // BytePosition
            4,                                      // BitPosition
            32);                                    // BitSize

    int32_t value = extractLogicalValue(reportData, control);
    EXPECT_EQ(value, 0x76'54'32'10);

    bool result = applyLogicalValue(reportData, control, 0x01'23'45'67);
    EXPECT_TRUE(result);
    EXPECT_EQ(reportData, QByteArray::fromHex("7A'56'34'12'B0"));

    int32_t value2 = extractLogicalValue(reportData, control);
    EXPECT_EQ(value2, 0x01'23'45'67);
}

TEST(HidReportDescriptorTest, ControlValue_signed32Bits) {
    auto reportData = QByteArray::fromHex("0A'21'43'65'B7");
    Control control({0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Flags
            0x0009'0001,                            // UsagePage/Usage
            std::numeric_limits<int32_t>::min(),    // LogicalMinimum
            std::numeric_limits<int32_t>::max(),    // LogicalMaximum
            10,                                     // PhysicalMinimum
            10,                                     // PhysicalMaximum
            0,                                      // UnitExponent
            0,                                      // Unit
            0,                                      // BytePosition
            4,                                      // BitPosition
            32);                                    // BitSize

    int32_t value = extractLogicalValue(reportData, control);
    EXPECT_EQ(value, 0x76'54'32'10);

    bool result = applyLogicalValue(reportData, control, std::numeric_limits<int32_t>::min());
    EXPECT_TRUE(result);
    EXPECT_EQ(reportData, QByteArray::fromHex("0A'00'00'00'B8"));

    int32_t value2 = extractLogicalValue(reportData, control);
    EXPECT_EQ(value2, std::numeric_limits<int32_t>::min());

    bool result2 = applyLogicalValue(reportData, control, std::numeric_limits<int32_t>::max());
    EXPECT_TRUE(result2);
    EXPECT_EQ(reportData, QByteArray::fromHex("FA'FF'FF'FF'B7"));

    int32_t value3 = extractLogicalValue(reportData, control);
    EXPECT_EQ(value3, std::numeric_limits<int32_t>::max());
}

TEST(HidReportDescriptorTest, SetControlValue_OutOfRange) {
    auto reportData = QByteArray::fromHex("81'00'00'00'01");
    Control control({0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Flags
            0x0009'0001,                            // UsagePage/Usage
            0,                                      // LogicalMinimum
            2047,                                   // LogicalMaximum
            0,                                      // PhysicalMinimum
            2047,                                   // PhysicalMaximum
            0,                                      // UnitExponent
            0,                                      // Unit
            0,                                      // BytePosition
            0,                                      // BitPosition
            11);                                    // BitSize

    bool result = applyLogicalValue(reportData, control, 3000);
    EXPECT_FALSE(result);
}

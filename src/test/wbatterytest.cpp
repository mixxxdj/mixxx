#include "test/mixxxtest.h"

#include "widget/wbattery.h"

namespace {

class WBatteryTest : public MixxxTest {};

TEST_F(WBatteryTest, formatTooltip) {
    EXPECT_EQ("0%", WBattery::formatTooltip(0));
    EXPECT_EQ("9%", WBattery::formatTooltip(9.4));
    EXPECT_EQ("11%", WBattery::formatTooltip(10.5));
    EXPECT_EQ("99%", WBattery::formatTooltip(99.1));
    EXPECT_EQ("100%", WBattery::formatTooltip(99.9));
    EXPECT_EQ("100%", WBattery::formatTooltip(100));
    EXPECT_EQ("100%", WBattery::formatTooltip(100.1));
}

}  // namespace

#include "util/cmdlineargs.h"

#include <gtest/gtest.h>

class CmdlineArgsTest : public ::testing::Test {
};

#ifdef MIXXX_USE_QML
TEST_F(CmdlineArgsTest, DefaultQmlOff) {
    CmdlineArgs args;
    EXPECT_FALSE(args.isQml());
}
#endif

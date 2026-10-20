#include "util/cmdlineargs.h"

#include <gtest/gtest.h>

class CmdlineArgsTest : public ::testing::Test {
  protected:
    // parse() takes argc/argv; build them from a list.
    static bool parseArgs(CmdlineArgs& args, const QStringList& arguments) {
        std::vector<QByteArray> storage;
        std::vector<char*> argv;
        storage.reserve(arguments.size());
        argv.reserve(arguments.size());
        for (const QString& arg : arguments) {
            storage.push_back(arg.toLocal8Bit());
            argv.push_back(storage.back().data());
        }
        return args.parse(static_cast<int>(argv.size()), argv.data());
    }
};

#ifdef MIXXX_USE_QML
TEST_F(CmdlineArgsTest, DefaultQmlOff) {
    CmdlineArgs args;
    EXPECT_FALSE(args.isQml());
}

TEST_F(CmdlineArgsTest, OverrideNoNewUi) {
    CmdlineArgs args;
    EXPECT_TRUE(parseArgs(args, {"mixxx", "--no-new-ui"}));
    EXPECT_FALSE(args.isQml());
}

TEST_F(CmdlineArgsTest, OverrideNewUi) {
    CmdlineArgs args;
    EXPECT_TRUE(parseArgs(args, {"mixxx", "--new-ui"}));
    EXPECT_TRUE(args.isQml());
}
#endif

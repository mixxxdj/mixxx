#include "util/fileutils.h"

#include <QtDebug>

#include "test/mixxxtest.h"

class FileUtilsTest : public testing::Test {
};

TEST_F(FileUtilsTest, TestSafeFilename) {
    // Generate a file name for the temporary file
    const auto fileName = QStringLiteral("broken/ <> :\"\\|ok?*!.mp3");
    const auto expected = QStringLiteral("broken/ ## ##/#ok##!.mp3");
    auto output = FileUtils::safeFilename(fileName);
    ASSERT_EQ(expected, output);

    // test 0 byte characters
    const auto fileName0 = QStringLiteral("t2\0\10Z");
    auto output2 = FileUtils::safeFilename(fileName0);
    ASSERT_EQ(QStringLiteral("t2##Z"), output2);
}

TEST_F(FileUtilsTest, TestDirReplace) {
    // Generate a file name for the temporary file
    const auto fileName = QStringLiteral("filename/with\\chara-cters.mp3");
    const auto expected = QStringLiteral("filename-with-chara-cters.mp3");
    auto output = FileUtils::replaceDirChars(fileName);
    ASSERT_EQ(expected, output);
}

TEST_F(FileUtilsTest, TestFileEscape) {
    // Generate a file name for the temporary file
    const auto fileName = QStringLiteral("A<>ame/with\\chars.mp3");
    const auto expected = QStringLiteral("A##ame-with-chars.mp3");
    auto output = FileUtils::escapeFileName(fileName);
    ASSERT_EQ(expected, output);
}

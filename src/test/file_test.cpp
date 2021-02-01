#include "util/file.h"

#include <QtDebug>

#include "test/mixxxtest.h"

class FileTest : public testing::Test {
};

TEST_F(FileTest, TestSafeFilename) {
    // Generate a file name for the temporary file
    const auto fileName = QStringLiteral("broken/ <> :\"\\|ok?*!.mp3");
    const auto expected = QStringLiteral("broken# ## ####ok##!.mp3");
    auto output = FileUtils::safeFilename(fileName);
    //qDebug() << "replaced" << output;
    ASSERT_EQ(expected, output);

    // test 0 byte characters
    const auto fileName0 = QStringLiteral("t2\0\10Z");
    auto output2 = FileUtils::safeFilename(fileName0);
    //qDebug() << "replaced" << output2;
    ASSERT_EQ(QStringLiteral("t2##Z"), output2);
}

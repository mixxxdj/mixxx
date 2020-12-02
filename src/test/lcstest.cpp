#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>

#include <QByteArray>
#include <QString>
#include <QtGlobal>
#include <memory>

#include "gtest/gtest_pred_impl.h"
#include "util/lcs.h"

TEST(LCS, BasicLCS) {
    ASSERT_STREQ(qPrintable(QString("FOO")),
                 qPrintable(LCS("FOO", "FOO")));

    ASSERT_STREQ(qPrintable(QString("")),
                 qPrintable(LCS("FOO", "BAR")));

    // Prefix
    ASSERT_STREQ(qPrintable(QString("FOO")),
                 qPrintable(LCS("FOO", "FOO BAR")));

    // Suffix
    ASSERT_STREQ(qPrintable(QString("FOO")),
                 qPrintable(LCS("FOO", "BAR FOO")));

    // Infix
    ASSERT_STREQ(qPrintable(QString("FOO")),
                 qPrintable(LCS("FOO", "BAR FOO BAZ")));

    ASSERT_STREQ(qPrintable(QString(" FOO ")),
                 qPrintable(LCS("QUUX FOO FROB", "BAR FOO BAZ")));
}

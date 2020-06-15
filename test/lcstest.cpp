#include <gtest/gtest.h>
#include <QtDebug>

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

#include <QChar>
#include <QDebug>
#include <QLocale>
#include <QString>

#include <gtest/gtest.h>

#include "util/stringhelper.h"

namespace {

TEST(StringHelperTest, Ascii) {
    QChar c = StringHelper::getFirstCharForGrouping("a");
    ASSERT_EQ(c, QChar('A'));
}

TEST(StringHelperTest, Null) {
    QChar c = StringHelper::getFirstCharForGrouping("");
    ASSERT_TRUE(c.isNull());
}

TEST(StringHelperTest, Change) {
    QLocale prev;
    QLocale::setDefault(QLocale::Catalan);
    QString s1 = QString::fromUtf8("ç");
    QString s2 = QString::fromUtf8("C");
    QChar c1 = StringHelper::getFirstCharForGrouping(s1);
    QChar c2 = s2.at(0);
    ASSERT_EQ(QString::localeAwareCompare(c1, c2), 0) 
            << qPrintable(c1) << " " << qPrintable(c2);
    QLocale::setDefault(prev);
}

TEST(StringHelperTest, RemoveAccent) {
    QLocale prev;
    QLocale::setDefault(QLocale::Catalan);
    QString s1 = QString::fromUtf8("à");
    QString s2 = QString::fromUtf8("A");
    QChar c1 = StringHelper::getFirstCharForGrouping(s1);
    QChar c2 = s2.at(0);
    ASSERT_EQ(QString::localeAwareCompare(c1, c2), 0) 
            << qPrintable(c1) << " " << qPrintable(c2);
    QLocale::setDefault(prev);
}

TEST(StringHelperTest, Finnish) {
    QLocale prev;
    QLocale::setDefault(QLocale::Finnish);
    
    QString s1 = QString::fromUtf8("å");
    QString s2 = QString::fromUtf8("Å");
    QChar c1 = StringHelper::getFirstCharForGrouping(s1);
    QChar c2 = s2.at(0);
#if __LINUX__
    ASSERT_EQ(QString::localeAwareCompare(c1, c2), 0) 
            << qPrintable(c1) << " " << qPrintable(c2);
#endif
    QLocale::setDefault(prev);
}

TEST(StringHelperTest, Chinese) {
    QLocale prev;
    QLocale::setDefault(QLocale::Chinese);
    
    QString s1 = QString::fromUtf8("诶");
    QString s2 = QString::fromUtf8("诶");
    QChar c1 = StringHelper::getFirstCharForGrouping(s1);
    QChar c2 = s2.at(0);
    ASSERT_EQ(QString::localeAwareCompare(c1, c2), 0) 
            << qPrintable(c1) << " " << qPrintable(c2);
    
    QLocale::setDefault(prev);
}

}

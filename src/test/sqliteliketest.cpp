#include <gtest/gtest.h>
#include "library/trackcollection.h"

class TrackCollectionTest : public TrackCollection {
   public:
#ifdef __SQLITE3__
     static int likeCompareLatinLowTest(
                    QString* pattern,
                    QString* string,
                    const QChar esc) {
         return TrackCollection::likeCompareLatinLow(
                        pattern, string, esc);

     }
#endif // __SQLITE3__
};


class SqliteLikeTest : public testing::Test {
  public:
    SqliteLikeTest() {
    }

  protected:
    virtual void SetUp() {
    }
};

#ifdef __SQLITE3__
TEST_F(SqliteLikeTest, PatternTest) {
    QString pattern;
    QString string;
    QChar esc;

    pattern = QString::fromUtf8("%väth%");
    string = QString::fromUtf8("Sven Väth");
    esc = '\0';
    EXPECT_TRUE(TrackCollectionTest::likeCompareLatinLowTest(&pattern, &string, esc));

    pattern = QString::fromUtf8("%vath%");
    string = QString::fromUtf8("Sven Väth");
    esc = '\0';
    EXPECT_TRUE(TrackCollectionTest::likeCompareLatinLowTest(&pattern, &string, esc));

    pattern = QString::fromUtf8("%väth%");
    string = QString::fromUtf8("Sven Vath");
    esc = '\0';
    EXPECT_TRUE(TrackCollectionTest::likeCompareLatinLowTest(&pattern, &string, esc));

    pattern = QString::fromUtf8("%v_th%");
    string = QString::fromUtf8("Sven Väth");
    esc = '\0';
    EXPECT_TRUE(TrackCollectionTest::likeCompareLatinLowTest(&pattern, &string, esc));

    pattern = QString::fromUtf8("%v_th%%");
    string = QString::fromUtf8("Sven Väth");
    esc = '\0';
    EXPECT_TRUE(TrackCollectionTest::likeCompareLatinLowTest(&pattern, &string, esc));

    pattern = QString::fromUtf8("%v%_%th%%");
    string = QString::fromUtf8("Sven Väth");
    esc = '\0';
    EXPECT_TRUE(TrackCollectionTest::likeCompareLatinLowTest(&pattern, &string, esc));

    pattern = QString::fromUtf8("%v!%th%");
    string = QString::fromUtf8("Sven V%th");
    esc = '!';
    EXPECT_TRUE(TrackCollectionTest::likeCompareLatinLowTest(&pattern, &string, esc));

    pattern = QString::fromUtf8("%ä%");
    string = QString::fromUtf8("Tiësto");
    esc = '\0';
    EXPECT_FALSE(TrackCollectionTest::likeCompareLatinLowTest(&pattern, &string, esc));
}
#endif // __SQLITE3__



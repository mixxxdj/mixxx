#include <gtest/gtest.h>

#include <QtDebug>

#include "track/steminfo.h"

namespace {

TEST(StemInfoTest, equality) {
    mixxx::Stem stem1("Foo", "black"), stem2("Bar", "white");

    ASSERT_NE(stem1, stem2);
    stem1.setColor(stem2.getColor());
    ASSERT_NE(stem1, stem2);
    stem1.setLabel(stem2.getLabel());
    ASSERT_EQ(stem1, stem2);
    stem1.setColor(QColor(0xff, 0xff, 0xff)); // #fd4a4a
    ASSERT_EQ(stem1, stem2);
}

TEST(StemInfoTest, validity) {
    mixxx::Stem stem("black", "foo");

    ASSERT_FALSE(stem.isValid());
    stem.setColor("red");
    ASSERT_TRUE(stem.isValid());
    stem.setLabel("");
    ASSERT_FALSE(stem.isValid());
    stem.setLabel("Foo");
    ASSERT_TRUE(stem.isValid());

    stem = mixxx::Stem("Bar", "green");
    ASSERT_TRUE(stem.isValid());
}

} // namespace

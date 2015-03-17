#include <gtest/gtest.h>
#include <QtDebug>

#include "engine/channelhandle.h"
#include "test/mixxxtest.h"

namespace {

TEST(ChannelHandleTest, BasicUsage) {
    ChannelHandleFactory factory;
    const QString group = "[Test]";
    const QString group2 = "[Test2]";

    ChannelHandle nullHandle;
    EXPECT_EQ(nullHandle, nullHandle);
    EXPECT_FALSE(nullHandle.valid());

    EXPECT_FALSE(factory.handleForGroup(group).valid());
    EXPECT_EQ(0, factory.getOrCreateHandle(group).handle());
    EXPECT_EQ(0, factory.getOrCreateHandle(group).handle());
    ChannelHandle testHandle = factory.handleForGroup(group);
    EXPECT_TRUE(testHandle.valid());
    EXPECT_EQ(0, testHandle.handle());
    EXPECT_QSTRING_EQ(group, factory.groupForHandle(testHandle));
    EXPECT_NE(nullHandle, testHandle);
    EXPECT_EQ(testHandle, testHandle);

    EXPECT_FALSE(factory.handleForGroup(group2).valid());
    EXPECT_EQ(1, factory.getOrCreateHandle(group2).handle());
    EXPECT_EQ(1, factory.getOrCreateHandle(group2).handle());
    ChannelHandle testHandle2 = factory.handleForGroup(group2);
    EXPECT_TRUE(testHandle2.valid());
    EXPECT_EQ(1, testHandle2.handle());
    EXPECT_QSTRING_EQ(group2, factory.groupForHandle(testHandle2));
    EXPECT_NE(nullHandle, testHandle2);
    EXPECT_EQ(testHandle2, testHandle2);

    EXPECT_NE(testHandle, testHandle2);
}

TEST(ChannelHandleTest, ChannelHandleMap) {
    ChannelHandleFactory factory;

    ChannelHandle test = factory.getOrCreateHandle("[Test]");
    EXPECT_TRUE(test.valid());
    ChannelHandle test2 = factory.getOrCreateHandle("[Test2]");
    EXPECT_TRUE(test2.valid());

    ChannelHandleMap<QString> map;

    EXPECT_QSTRING_EQ(QString(), map.at(ChannelHandle()));

    map.insert(test2, "bar");
    EXPECT_QSTRING_EQ("bar", map.at(test2));

    map.insert(test, "foo");
    EXPECT_QSTRING_EQ("foo", map.at(test));

    QString& reference = map[test];
    reference.chop(1);
    EXPECT_QSTRING_EQ("fo", map.at(test));

    // Replaces existing value.
    map.insert(test, "foo");
    EXPECT_QSTRING_EQ("foo", map.at(test));
}

}  // namespace

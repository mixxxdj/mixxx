#include <gtest/gtest.h>
#include <QtDebug>

#include "engine/channelhandle.h"
#include "test/mixxxtest.h"

TEST(ChannelHandleTest, BasicUsage) {
    ChannelHandleFactory factory;
    const QString group = "[Test]";
    const QString group2 = "[Test2]";

    EXPECT_EQ(nullptr, factory.handleForGroup(group));
    // The ChannelHandleFactory constructor creates handles for [Master] and [Headphone]
    EXPECT_EQ(0, factory.getOrCreateHandle(group)->handle());
    EXPECT_EQ(0, factory.getOrCreateHandle(group)->handle());
    const ChannelHandle* pTestHandle = factory.handleForGroup(group);
    EXPECT_NE(nullptr, pTestHandle);
    EXPECT_TRUE(pTestHandle->valid());
    EXPECT_EQ(0, pTestHandle->handle());
    EXPECT_QSTRING_EQ(group, factory.groupForHandle(pTestHandle));
    EXPECT_EQ(*pTestHandle, *pTestHandle);

    EXPECT_EQ(nullptr, factory.handleForGroup(group2));
    EXPECT_EQ(1, factory.getOrCreateHandle(group2)->handle());
    EXPECT_EQ(1, factory.getOrCreateHandle(group2)->handle());
    const ChannelHandle* pTestHandle2 = factory.handleForGroup(group2);
    EXPECT_NE(nullptr, pTestHandle2);
    EXPECT_TRUE(pTestHandle2->valid());
    EXPECT_EQ(1, pTestHandle2->handle());
    EXPECT_QSTRING_EQ(group2, factory.groupForHandle(pTestHandle2));
    EXPECT_EQ(*pTestHandle2, *pTestHandle2);

    EXPECT_NE(*pTestHandle, *pTestHandle2);
}

TEST(ChannelHandleTest, ChannelHandleMap) {
    ChannelHandleFactory factory;

    const ChannelHandle* test = factory.getOrCreateHandle("[Test]");
    EXPECT_TRUE(test->valid());
    const ChannelHandle* test2 = factory.getOrCreateHandle("[Test2]");
    EXPECT_TRUE(test2->valid());

    ChannelHandleMap<QString> map;

    EXPECT_QSTRING_EQ(QString(), map.at(nullptr));

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

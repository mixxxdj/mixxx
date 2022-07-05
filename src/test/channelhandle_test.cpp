#include <gtest/gtest.h>
#include <QtDebug>

#include "engine/channelhandle.h"
#include "test/mixxxtest.h"

TEST(ChannelHandleTest, GroupHandle) {
    resetAllGroupHandles();

    const QString group = "[Test]";
    const QString group2 = "[Test2]";

    EXPECT_EQ(nullptr, getGroupHandleByName(group));
    EXPECT_EQ(0, indexOfGroupHandle(getOrCreateGroupHandleByName(group)));
    EXPECT_EQ(0, indexOfGroupHandle(getOrCreateGroupHandleByName(group)));
    GroupHandle testHandle = getGroupHandleByName(group);
    EXPECT_NE(nullptr, testHandle);
    EXPECT_EQ(0, indexOfGroupHandle(testHandle));
    EXPECT_QSTRING_EQ(group, nameOfGroupHandle(testHandle));
    EXPECT_EQ(*testHandle, *testHandle);

    EXPECT_EQ(nullptr, getGroupHandleByName(group2));
    EXPECT_EQ(1, indexOfGroupHandle(getOrCreateGroupHandleByName(group2)));
    EXPECT_EQ(1, indexOfGroupHandle(getOrCreateGroupHandleByName(group2)));
    GroupHandle testHandle2 = getGroupHandleByName(group2);
    EXPECT_NE(nullptr, testHandle2);
    EXPECT_EQ(1, indexOfGroupHandle(testHandle2));
    EXPECT_QSTRING_EQ(group2, nameOfGroupHandle(testHandle2));
    EXPECT_EQ(*testHandle2, *testHandle2);

    EXPECT_NE(*testHandle, *testHandle2);
}

TEST(ChannelHandleTest, ChannelHandleMap) {
    resetAllGroupHandles();

    GroupHandle test = getOrCreateGroupHandleByName("[Test]");
    GroupHandle test2 = getOrCreateGroupHandleByName("[Test2]");

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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDebug>

#include "track/keyfactory.h"

using ::testing::UnorderedElementsAre;

class KeyFactoryTest : public testing::Test {
};

TEST_F(KeyFactoryTest, MakePreferredKeys) {
    KeyChangeList key_changes = {
            {mixxx::track::io::key::B_MAJOR, 1.0},
            {mixxx::track::io::key::B_MINOR, 2.0},
            {mixxx::track::io::key::B_MAJOR, 3.0},
    };
    QHash<QString, QString> extraVersionInfo = {{QStringLiteral("a"), QStringLiteral("b")}};

    Keys track_keys = KeyFactory::makePreferredKeys(
            key_changes, extraVersionInfo, mixxx::audio::SampleRate(44100), 4.0);

    EXPECT_EQ(track_keys.getGlobalKey(), mixxx::track::io::key::B_MAJOR);
    EXPECT_EQ(track_keys.getGlobalKeyText().toStdString(), "B");
    EXPECT_EQ(track_keys.getSubVersion().toStdString(), "a=b");
}

#include "soundio/jackautoconnector.h"

#include <gtest/gtest.h>

namespace {

const QStringList kInputs{
        QStringLiteral("ardour:Mixxx Main/audio_in 1"),
        QStringLiteral("ardour:Mixxx Main/audio_in 2"),
        QStringLiteral("ardour:Mixxx Phones/audio_in 1"),
        QStringLiteral("ardour:Mixxx Phones/audio_in 2")};
const QString kPrefix = QStringLiteral("Mixxx");

TEST(JackAutoConnectorTest, RepairsDuplicateConnectedAndRememberedRoutes) {
    QSet<QString> claimed;
    for (qsizetype channel = 0; channel < 4; ++channel) {
        const QString duplicate = kInputs.at(channel % 2);
        const QString destination = jack::selectAutoConnectDestination(kInputs,
                {duplicate},
                duplicate,
                kPrefix,
                claimed,
                {},
                QString::number(channel),
                channel,
                4);
        EXPECT_EQ(kInputs.at(channel), destination);
        claimed.insert(destination);
    }
}

TEST(JackAutoConnectorTest, PreservesAvailableRememberedRouteAndReservesIt) {
    const QHash<QString, QString> reserved{{kInputs.at(0), QStringLiteral("later")}};
    EXPECT_EQ(kInputs.at(1),
            jack::selectAutoConnectDestination(kInputs,
                    {},
                    {},
                    kPrefix,
                    {},
                    reserved,
                    QStringLiteral("first"),
                    0,
                    4));
    EXPECT_EQ(kInputs.at(0),
            jack::selectAutoConnectDestination(kInputs,
                    {},
                    kInputs.at(0),
                    kPrefix,
                    {kInputs.at(1)},
                    reserved,
                    QStringLiteral("later"),
                    1,
                    4));
}

TEST(JackAutoConnectorTest, DoesNotReplaceMissingRouteWithPartialInputs) {
    EXPECT_TRUE(
            jack::selectAutoConnectDestination({kInputs.at(0), kInputs.at(1)},
                    {},
                    kInputs.at(2),
                    kPrefix,
                    {},
                    {},
                    QStringLiteral("headphone"),
                    0,
                    4)
                    .isEmpty());
    EXPECT_TRUE(
            jack::selectAutoConnectDestination({kInputs.at(0), kInputs.at(1)},
                    {kInputs.at(0)},
                    kInputs.at(0),
                    kPrefix,
                    {kInputs.at(0), kInputs.at(1)},
                    {},
                    QStringLiteral("headphone"),
                    2,
                    4)
                    .isEmpty());
}

TEST(JackAutoConnectorTest, ReplacesMissingRouteWithCompleteInputs) {
    EXPECT_EQ(kInputs.at(2),
            jack::selectAutoConnectDestination(kInputs,
                    {},
                    QStringLiteral("ardour:Mixxx Head/audio_in 1"),
                    kPrefix,
                    {kInputs.at(0), kInputs.at(1)},
                    {},
                    QStringLiteral("headphone"),
                    2,
                    4));
}

} // namespace

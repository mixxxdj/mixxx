#include <gtest/gtest.h>

#include <QMetaObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QUrl>
#include <QVariant>
#include <array>
#include <memory>

#include "preferences/interface.h"
#include "util/duration.h"

namespace {

QString formatDuration(double seconds, TrackTime::DisplayFormat format) {
    const auto precision = format == TrackTime::DisplayFormat::TRADITIONAL_COARSE
            ? mixxx::Duration::Precision::SECONDS
            : mixxx::Duration::Precision::CENTISECONDS;

    switch (format) {
    case TrackTime::DisplayFormat::SECONDS:
        return mixxx::Duration::formatSeconds(seconds, precision);
    case TrackTime::DisplayFormat::SECONDS_LONG:
        return mixxx::Duration::formatSecondsLong(seconds, precision);
    case TrackTime::DisplayFormat::KILO_SECONDS:
        return mixxx::Duration::formatKiloSeconds(seconds, precision);
    case TrackTime::DisplayFormat::TRADITIONAL:
    case TrackTime::DisplayFormat::TRADITIONAL_COARSE:
    case TrackTime::DisplayFormat::HECTO_SECONDS:
        return mixxx::Duration::formatTime(seconds, precision);
    }
    return mixxx::Duration::kInvalidDurationString;
}

class QmlDurationFormatterTest : public testing::Test {
  protected:
    void SetUp() override {
        m_engine.addImportPath(QStringLiteral(RESOURCE_FOLDER "/qml"));

        QQmlComponent component(&m_engine);
        component.setData(R"(
import QtQml 2.12
import Mixxx 1.0 as Mixxx
import "Deck" as DeckComponents

QtObject {
    readonly property int traditional: Mixxx.DurationFormatter.Mode.Traditional
    readonly property int traditionalCoarse: Mixxx.DurationFormatter.Mode.TraditionalCoarse
    readonly property int seconds: Mixxx.DurationFormatter.Mode.Seconds
    readonly property int secondsLong: Mixxx.DurationFormatter.Mode.SecondsLong
    readonly property int kiloSeconds: Mixxx.DurationFormatter.Mode.KiloSeconds
    readonly property int hectoSeconds: Mixxx.DurationFormatter.Mode.HectoSeconds
    readonly property int trackTimeTraditional: DeckComponents.TrackTime.Mode.Traditional
    readonly property int trackTimeTraditionalCoarse: DeckComponents.TrackTime.Mode.TraditionalCoarse
    readonly property int trackTimeSeconds: DeckComponents.TrackTime.Mode.Seconds
    readonly property int trackTimeSecondsLong: DeckComponents.TrackTime.Mode.SecondsLong
    readonly property int trackTimeKiloSeconds: DeckComponents.TrackTime.Mode.KiloSeconds
    readonly property int trackTimeHectoSeconds: DeckComponents.TrackTime.Mode.HectoSeconds
    readonly property int trackTimeElapsed: DeckComponents.TrackTime.Display.Elapsed
    readonly property int trackTimeRemaining: DeckComponents.TrackTime.Display.Remaining
    readonly property int trackTimeBoth: DeckComponents.TrackTime.Display.Both

    function formatDuration(value, mode) {
        return Mixxx.DurationFormatter.format(value, mode);
    }
}
)",
                QUrl::fromLocalFile(QStringLiteral(
                        RESOURCE_FOLDER "/qml/durationformattertest.qml")));

        m_pRoot.reset(component.create());
        ASSERT_FALSE(component.isError()) << qPrintable(component.errorString());
        ASSERT_TRUE(m_pRoot) << qPrintable(component.errorString());
    }

    QString qmlFormat(double seconds, TrackTime::DisplayFormat format) const {
        QVariant result;
        const bool invoked = QMetaObject::invokeMethod(m_pRoot.get(),
                "formatDuration",
                Q_RETURN_ARG(QVariant, result),
                Q_ARG(QVariant, seconds),
                Q_ARG(QVariant, static_cast<int>(format)));
        EXPECT_TRUE(invoked);
        return result.toString();
    }

    QQmlEngine m_engine;
    std::unique_ptr<QObject> m_pRoot;
};

TEST_F(QmlDurationFormatterTest, DisplayFormatValuesMatchCppEnum) {
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::TRADITIONAL),
            m_pRoot->property("traditional").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::TRADITIONAL_COARSE),
            m_pRoot->property("traditionalCoarse").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::SECONDS),
            m_pRoot->property("seconds").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::SECONDS_LONG),
            m_pRoot->property("secondsLong").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::KILO_SECONDS),
            m_pRoot->property("kiloSeconds").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::HECTO_SECONDS),
            m_pRoot->property("hectoSeconds").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::TRADITIONAL),
            m_pRoot->property("trackTimeTraditional").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::TRADITIONAL_COARSE),
            m_pRoot->property("trackTimeTraditionalCoarse").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::SECONDS),
            m_pRoot->property("trackTimeSeconds").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::SECONDS_LONG),
            m_pRoot->property("trackTimeSecondsLong").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::KILO_SECONDS),
            m_pRoot->property("trackTimeKiloSeconds").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayFormat::HECTO_SECONDS),
            m_pRoot->property("trackTimeHectoSeconds").toInt());
}

TEST_F(QmlDurationFormatterTest, DisplayModeValuesMatchCppEnum) {
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayMode::ELAPSED),
            m_pRoot->property("trackTimeElapsed").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayMode::REMAINING),
            m_pRoot->property("trackTimeRemaining").toInt());
    EXPECT_EQ(static_cast<int>(TrackTime::DisplayMode::ELAPSED_AND_REMAINING),
            m_pRoot->property("trackTimeBoth").toInt());
}

TEST_F(QmlDurationFormatterTest, OutputMatchesCppFormatter) {
    constexpr std::array formats{
            TrackTime::DisplayFormat::TRADITIONAL,
            TrackTime::DisplayFormat::TRADITIONAL_COARSE,
            TrackTime::DisplayFormat::SECONDS,
            TrackTime::DisplayFormat::SECONDS_LONG,
            TrackTime::DisplayFormat::KILO_SECONDS,
            TrackTime::DisplayFormat::HECTO_SECONDS,
    };
    constexpr std::array values{
            -1.0,
            0.0,
            1.0,
            1.49,
            59.0,
            61.1234,
            321.1236,
            999.99,
            1000.0,
            3599.999,
            3600.0,
            24.0 * 3600.0,
            25.0 * 3600.0 + 1.0,
    };

    for (const auto format : formats) {
        for (const double value : values) {
            SCOPED_TRACE(testing::Message()
                    << "format=" << static_cast<int>(format) << ", value=" << value);
            EXPECT_EQ(formatDuration(value, format), qmlFormat(value, format));
        }
    }
}

} // namespace

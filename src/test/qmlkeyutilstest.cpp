#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QQmlEngine>
#include <memory>

#include "qml/qmlkeyutils.h"
#include "test/mixxxtest.h"
#include "track/keyutils.h"

using namespace mixxx::qml;

namespace {

class QmlKeyUtilsTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_qmlKeyUtils = std::make_unique<QmlKeyUtils>();
    }

    std::unique_ptr<QmlKeyUtils> m_qmlKeyUtils;
};

TEST_F(QmlKeyUtilsTest, KeyIsValid) {
    // 1 is C_MAJOR (valid)
    EXPECT_TRUE(m_qmlKeyUtils->keyIsValid(1.0));
    // 22 is A_MINOR (valid)
    EXPECT_TRUE(m_qmlKeyUtils->keyIsValid(22.0));
    // 0 is INVALID
    EXPECT_FALSE(m_qmlKeyUtils->keyIsValid(0.0));
    // Out of bounds
    EXPECT_FALSE(m_qmlKeyUtils->keyIsValid(-1.0));
    EXPECT_FALSE(m_qmlKeyUtils->keyIsValid(99.0));
}

TEST_F(QmlKeyUtilsTest, KeyToOpenKeyNumber) {
    // C_MAJOR (1) -> 1
    EXPECT_EQ(m_qmlKeyUtils->keyToOpenKeyNumber(1.0), 1);
    // A_MINOR (22) -> 1
    EXPECT_EQ(m_qmlKeyUtils->keyToOpenKeyNumber(22.0), 1);
    // INVALID (0) -> 0
    EXPECT_EQ(m_qmlKeyUtils->keyToOpenKeyNumber(0.0), 0);
}

TEST_F(QmlKeyUtilsTest, ScaleKeySteps) {
    // C_MAJOR (1) + 2 steps -> D_MAJOR (3)
    EXPECT_DOUBLE_EQ(m_qmlKeyUtils->scaleKeySteps(1.0, 2), 3.0);
    // C_MAJOR (1) - 1 step -> B_MAJOR (12)
    EXPECT_DOUBLE_EQ(m_qmlKeyUtils->scaleKeySteps(1.0, -1), 12.0);
    // INVALID (0) scaled stays INVALID (0)
    EXPECT_DOUBLE_EQ(m_qmlKeyUtils->scaleKeySteps(0.0, 5), 0.0);
}

TEST_F(QmlKeyUtilsTest, KeyToStringWithExplicitNotation) {
    // OpenKey (2)
    double openKeyNotation = static_cast<double>(KeyUtils::KeyNotation::OpenKey);
    // C_MAJOR (1) -> "1d"
    EXPECT_EQ(m_qmlKeyUtils->keyToString(1.0, openKeyNotation), QStringLiteral("1d"));
    // A_MINOR (22) -> "1m"
    EXPECT_EQ(m_qmlKeyUtils->keyToString(22.0, openKeyNotation), QStringLiteral("1m"));

    // Lancelot (3)
    double lancelotNotation = static_cast<double>(KeyUtils::KeyNotation::Lancelot);
    // C_MAJOR (1) -> "8B"
    EXPECT_EQ(m_qmlKeyUtils->keyToString(1.0, lancelotNotation), QStringLiteral("8B"));
    // A_MINOR (22) -> "8A"
    EXPECT_EQ(m_qmlKeyUtils->keyToString(22.0, lancelotNotation), QStringLiteral("8A"));

    // Traditional (4)
    double traditionalNotation = static_cast<double>(KeyUtils::KeyNotation::Traditional);
    // C_MAJOR (1) -> "C"
    EXPECT_EQ(m_qmlKeyUtils->keyToString(1.0, traditionalNotation), QStringLiteral("C"));
    // A_MINOR (22) -> "Am"
    EXPECT_EQ(m_qmlKeyUtils->keyToString(22.0, traditionalNotation), QStringLiteral("Am"));

    // INVALID key returns empty string
    EXPECT_EQ(m_qmlKeyUtils->keyToString(0.0, traditionalNotation), QStringLiteral(""));
}

TEST_F(QmlKeyUtilsTest, KeyToStringUsesCustomNotation) {
    // Set up a custom notation map
    QMap<mixxx::track::io::key::ChromaticKey, QString> customNotation;
    for (int i = 1; i <= 24; ++i) {
        customNotation[static_cast<mixxx::track::io::key::ChromaticKey>(i)] =
                QStringLiteral("Custom%1").arg(i);
    }
    KeyUtils::setNotation(customNotation);

    // C_MAJOR (1) -> "Custom1"
    EXPECT_EQ(m_qmlKeyUtils->keyToString(1.0), QStringLiteral("Custom1"));
    // A_MINOR (22) -> "Custom22"
    EXPECT_EQ(m_qmlKeyUtils->keyToString(22.0), QStringLiteral("Custom22"));

    // INVALID key still returns empty string
    EXPECT_EQ(m_qmlKeyUtils->keyToString(0.0), QStringLiteral(""));
}

} // namespace

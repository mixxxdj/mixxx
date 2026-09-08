#include <gtest/gtest.h>

#include <QMetaObject>
#include <QTest>
#include <memory>

#include "qml/qmlspinnyposition.h"
#include "test/mixxxtest.h"
#include "waveform/visualplayposition.h"

namespace {

class QmlSpinnyPositionTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_group = QStringLiteral("[QmlSpinnyPositionTest]");
        m_visualPlayPosition = VisualPlayPosition::getVisualPlayPosition(m_group);
        m_position = std::make_unique<mixxx::qml::QmlSpinnyPosition>();
        m_position->setGroup(m_group);
    }

    void setPosition(double playPosition, double slipPosition, SlipModeState state) {
        m_visualPlayPosition->set(playPosition,
                1.0,
                0.0,
                slipPosition,
                1.0,
                state,
                false,
                false,
                false,
                0.0,
                0.0,
                0.0,
                0.0);
    }

    void refresh() {
        ASSERT_TRUE(QMetaObject::invokeMethod(
                m_position.get(), "refresh", Qt::DirectConnection));
    }

    QString m_group;
    QSharedPointer<VisualPlayPosition> m_visualPlayPosition;
    std::unique_ptr<mixxx::qml::QmlSpinnyPosition> m_position;
};

TEST_F(QmlSpinnyPositionTest, UsesForegroundPositionOutsideSlipPlayback) {
    setPosition(0.25, 0.75, SlipModeState::Disabled);
    refresh();

    EXPECT_TRUE(m_position->isValid());
    EXPECT_DOUBLE_EQ(0.25, m_position->getPlayPosition());
    EXPECT_DOUBLE_EQ(0.25, m_position->getSlipPosition());

    setPosition(0.25, 0.75, SlipModeState::Armed);
    refresh();
    EXPECT_DOUBLE_EQ(0.25, m_position->getSlipPosition());
}

TEST_F(QmlSpinnyPositionTest, UsesSlipPositionWhileRunning) {
    setPosition(0.25, 0.75, SlipModeState::Running);
    refresh();

    EXPECT_TRUE(m_position->isValid());
    EXPECT_DOUBLE_EQ(0.25, m_position->getPlayPosition());
    EXPECT_DOUBLE_EQ(0.75, m_position->getSlipPosition());
}

TEST_F(QmlSpinnyPositionTest, ClearsPositionsWhenSnapshotBecomesInvalid) {
    setPosition(0.25, 0.75, SlipModeState::Running);
    refresh();
    m_visualPlayPosition->setInvalid();
    refresh();

    EXPECT_FALSE(m_position->isValid());
    EXPECT_DOUBLE_EQ(0.0, m_position->getPlayPosition());
    EXPECT_DOUBLE_EQ(0.0, m_position->getSlipPosition());
}

TEST_F(QmlSpinnyPositionTest, AdaptsSyncIntervalToFrameCadence) {
    m_position->setVisible(true);
    ASSERT_TRUE(QMetaObject::invokeMethod(
            m_position.get(), "slotFrameSwapped", Qt::DirectConnection));

    QTest::qWait(30);

    ASSERT_TRUE(QMetaObject::invokeMethod(
            m_position.get(), "slotFrameSwapped", Qt::DirectConnection));
    EXPECT_GT(m_position->getSyncInterval().count(), 20000);
}

} // namespace

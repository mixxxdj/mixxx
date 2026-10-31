#pragma once

#include <QMetaObject>
#include <QQuickItem>
#include <QSharedPointer>
#include <QString>
#include <chrono>

#include "util/performancetimer.h"
#include "waveform/isynctimeprovider.h"

class QQuickWindow;
class VisualPlayPosition;

namespace mixxx {
namespace qml {

class QmlSpinnyPosition : public QQuickItem, public VSyncTimeProvider {
    Q_OBJECT
    Q_PROPERTY(QString group READ getGroup WRITE setGroup NOTIFY groupChanged REQUIRED)
    Q_PROPERTY(double playPosition READ getPlayPosition NOTIFY playPositionChanged)
    Q_PROPERTY(double slipPosition READ getSlipPosition NOTIFY slipPositionChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    QML_NAMED_ELEMENT(SpinnyPosition)

  public:
    explicit QmlSpinnyPosition(QQuickItem* parent = nullptr);
    ~QmlSpinnyPosition() override;

    QString getGroup() const {
        return m_group;
    }
    void setGroup(const QString& group);

    double getPlayPosition() const {
        return m_playPosition;
    }
    double getSlipPosition() const {
        return m_slipPosition;
    }
    bool isValid() const {
        return m_valid;
    }

    std::chrono::microseconds fromTimerToNextSync(const PerformanceTimer& timer) override;
    std::chrono::microseconds getSyncInterval() const override;

  protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

  private slots:
    void slotFrameSwapped();
    void slotWindowChanged(QQuickWindow* window);
    void refresh();

  signals:
    void groupChanged(const QString& group);
    void playPositionChanged(double position);
    void slipPositionChanged(double position);
    void validChanged(bool valid);

  private:
    void resetFrameTiming();
    void resetPositions();
    void updatePositions();

    static constexpr auto kDefaultSyncInterval = std::chrono::microseconds(16667);

    QString m_group;
    PerformanceTimer m_timer;
    QSharedPointer<VisualPlayPosition> m_visualPlayPosition;
    QMetaObject::Connection m_frameConnection;
    std::chrono::microseconds m_syncInterval{kDefaultSyncInterval};
    double m_playPosition{0.0};
    double m_slipPosition{0.0};
    bool m_haveFrameInterval{false};
    bool m_valid{false};
};

} // namespace qml
} // namespace mixxx

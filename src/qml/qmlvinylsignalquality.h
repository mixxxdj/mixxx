#pragma once

#include <QImage>
#include <QPointer>
#include <QQuickPaintedItem>

#include "vinylcontrol/vinylsignalquality.h"

class VinylControlManager;

namespace mixxx {
namespace qml {

class QmlVinylSignalQuality : public QQuickPaintedItem,
                              public VinylSignalQualityListener {
    Q_OBJECT
    Q_PROPERTY(QString group READ group WRITE setGroup NOTIFY groupChanged REQUIRED)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    QML_NAMED_ELEMENT(VinylSignalQuality)

  public:
    explicit QmlVinylSignalQuality(QQuickItem* parent = nullptr);
    ~QmlVinylSignalQuality() override;

    QString group() const {
        return m_group;
    }
    void setGroup(const QString& group);

    bool active() const {
        return m_active;
    }
    void setActive(bool active);

    void paint(QPainter* painter) override;
    void onVinylSignalQualityUpdate(const VinylSignalQualityReport& report) override;

  signals:
    void groupChanged(const QString& group);
    void activeChanged();

  private:
    void updateSignalQualityListener();

    QString m_group;
    QPointer<VinylControlManager> m_pVinylControlManager;
    QImage m_signalImage;
    bool m_active{false};
    bool m_listenerRegistered{false};
    bool m_hasReport{false};
    int m_vinylInput{-1};
};

} // namespace qml
} // namespace mixxx

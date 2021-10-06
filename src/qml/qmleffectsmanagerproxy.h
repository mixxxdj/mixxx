#pragma once
#include <QObject>

#include "effects/effectsmanager.h"
#include "qml/qmlvisibleeffectsmodel.h"

namespace mixxx {
namespace qml {

class QmlEffectSlotProxy;

class QmlEffectsManagerProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(mixxx::qml::QmlVisibleEffectsModel* visibleEffectsModel
                    MEMBER m_pVisibleEffectsModel CONSTANT);

  public:
    explicit QmlEffectsManagerProxy(
            std::shared_ptr<EffectsManager> pEffectsManager,
            QObject* parent = nullptr);

    Q_INVOKABLE mixxx::qml::QmlEffectSlotProxy* getEffectSlot(
            int rackNumber, int unitNumber, int effectNumber) const;

  private:
    const std::shared_ptr<EffectsManager> m_pEffectsManager;
    QmlVisibleEffectsModel* m_pVisibleEffectsModel;
};

} // namespace qml
} // namespace mixxx

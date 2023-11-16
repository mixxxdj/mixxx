#pragma once
#include <QObject>
#include <QtQml>

#include "effects/effectsmanager.h"
#include "qml/qmleffectslotproxy.h"
#include "qml/qmlvisibleeffectsmodel.h"

namespace mixxx {
namespace qml {

class QmlEffectsManagerProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(mixxx::qml::QmlVisibleEffectsModel* visibleEffectsModel
                    MEMBER m_pVisibleEffectsModel CONSTANT);
    QML_NAMED_ELEMENT(EffectsManager)
    QML_SINGLETON

  public:
    explicit QmlEffectsManagerProxy(
            std::shared_ptr<EffectsManager> pEffectsManager,
            QObject* parent = nullptr);

    Q_INVOKABLE mixxx::qml::QmlEffectSlotProxy* getEffectSlot(
            int unitNumber, int effectNumber) const;

    static QmlEffectsManagerProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static void registerEffectsManager(std::shared_ptr<EffectsManager> pEffectsManager) {
        s_pEffectManager = std::move(pEffectsManager);
    }

  private:
    static inline std::shared_ptr<EffectsManager> s_pEffectManager;

    const std::shared_ptr<EffectsManager> m_pEffectsManager;
    QmlVisibleEffectsModel* m_pVisibleEffectsModel;
};

} // namespace qml
} // namespace mixxx

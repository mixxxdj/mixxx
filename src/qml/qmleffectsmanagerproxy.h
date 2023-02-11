#pragma once
#include <QObject>
#include <QtQml>

#include "effects/effectsmanager.h"
#include "qml/qmlvisibleeffectsmodel.h"

namespace mixxx {
namespace qml {

class QmlEffectSlotProxy;

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
    static inline QmlEffectsManagerProxy* s_pInstance = nullptr;

  private:
    static inline QJSEngine* s_pJsEngine = nullptr;
    const std::shared_ptr<EffectsManager> m_pEffectsManager;
    QmlVisibleEffectsModel* m_pVisibleEffectsModel;
};

} // namespace qml
} // namespace mixxx

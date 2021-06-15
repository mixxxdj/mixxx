#pragma once
#include <QObject>

#include "effects/effectsmanager.h"

QT_FORWARD_DECLARE_CLASS(QAbstractListModel);

namespace mixxx {
namespace skin {
namespace qml {

class QmlVisibleEffectsModel;

class QmlEffectsManagerProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(mixxx::skin::qml::QmlVisibleEffectsModel* visibleEffectsModel
                    MEMBER m_pVisibleEffectsModel CONSTANT);

  public:
    explicit QmlEffectsManagerProxy(
            std::shared_ptr<EffectsManager> pEffectsManager,
            QObject* parent = nullptr);

    Q_INVOKABLE void loadEffect(int rack, int unit, int effect, const QString& effectId);

  private:
    const std::shared_ptr<EffectsManager> m_pEffectsManager;
    QmlVisibleEffectsModel* m_pVisibleEffectsModel;
};

} // namespace qml
} // namespace skin
} // namespace mixxx

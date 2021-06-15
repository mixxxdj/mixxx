#include "skin/qml/qmleffectsmanagerproxy.h"

#include <memory>

#include "skin/qml/qmlvisibleeffectsmodel.h"

namespace mixxx {
namespace skin {
namespace qml {

QmlEffectsManagerProxy::QmlEffectsManagerProxy(
        std::shared_ptr<EffectsManager> pEffectsManager, QObject* parent)
        : QObject(parent),
          m_pEffectsManager(pEffectsManager),
          m_pVisibleEffectsModel(
                  new QmlVisibleEffectsModel(pEffectsManager, this)) {
}

} // namespace qml
} // namespace skin
} // namespace mixxx

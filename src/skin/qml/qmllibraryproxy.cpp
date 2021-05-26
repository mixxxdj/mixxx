#include "skin/qml/qmllibraryproxy.h"

#include <QAbstractItemModel>

#include "library/library.h"

namespace mixxx {
namespace skin {
namespace qml {

QmlLibraryProxy::QmlLibraryProxy(Library* pLibrary, QObject* parent)
        : QObject(parent), m_pLibrary(pLibrary) {
}

QAbstractItemModel* QmlLibraryProxy::model() {
    // TODO
    return nullptr;
}

} // namespace qml
} // namespace skin
} // namespace mixxx

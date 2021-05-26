#include "skin/qml/qmllibraryproxy.h"

#include <QAbstractItemModel>

#include "library/library.h"
#include "library/sidebarmodel.h"

namespace mixxx {
namespace skin {
namespace qml {

QmlLibraryProxy::QmlLibraryProxy(std::shared_ptr<Library> pLibrary, QObject* parent)
        : QObject(parent),
          m_pLibrary(pLibrary),
          m_pModel(make_parented<QmlLibraryTrackListModel>(m_pLibrary->trackTableModel(), this)) {
}

QAbstractItemModel* QmlLibraryProxy::getSidebarModel() {
    return m_pLibrary->sidebarModel();
}

} // namespace qml
} // namespace skin
} // namespace mixxx

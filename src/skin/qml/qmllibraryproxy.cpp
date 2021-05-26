#include "skin/qml/qmllibraryproxy.h"

#include "library/library.h"
#include "library/libraryfeature.h"
#include "library/librarytablemodel.h"
#include "library/sidebarmodel.h"

namespace mixxx {
namespace skin {
namespace qml {

QmlLibraryProxy::QmlLibraryProxy(Library* pLibrary, QObject* parent)
        : QObject(parent), m_pLibrary(pLibrary) {
}

QObject* QmlLibraryProxy::getSidebarModel() {
    return m_pLibrary->sidebarModel();
}

QObject* QmlLibraryProxy::getLibraryModel() {
    return new LibraryTableModel(m_pLibrary,
            m_pLibrary->trackCollectionManager(),
            "mixxx.db.model.library");
}

} // namespace qml
} // namespace skin
} // namespace mixxx

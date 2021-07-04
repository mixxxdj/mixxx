#include "skin/qml/qmllibraryproxy.h"

#include <QAbstractItemModel>

#include "library/library.h"
#include "skin/qml/qmllibrarytracklistmodel.h"

namespace mixxx {
namespace skin {
namespace qml {

QmlLibraryProxy::QmlLibraryProxy(Library* pLibrary, QObject* parent)
        : QObject(parent),
          m_pLibrary(pLibrary),
          m_pModel(
                  new QmlLibraryTrackListModel(m_pLibrary->trackTableModel())) {
}

QmlLibraryProxy::~QmlLibraryProxy() {
    delete m_pModel;
}

} // namespace qml
} // namespace skin
} // namespace mixxx

#include "qml/qmllibraryproxy.h"

#include <QAbstractItemModel>

#include "library/library.h"

namespace mixxx {
namespace qml {

QmlLibraryProxy::QmlLibraryProxy(std::shared_ptr<Library> pLibrary, QObject* parent)
        : QObject(parent),
          m_pLibrary(pLibrary),
          m_pModelProperty(new QmlLibraryTrackListModel(m_pLibrary->trackTableModel(), this)) {
}

} // namespace qml
} // namespace mixxx

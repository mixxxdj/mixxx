#include "qml/qmllibraryproxy.h"

#include <QAbstractItemModel>
#include <QQmlEngine>

#include "library/library.h"
#include "library/librarytablemodel.h"
#include "qml/qmllibrarytracklistmodel.h"
#include "qml/qmltrackproxy.h"
#include "track/track.h"
#include "util/assert.h"
#include "widget/wtrackmenu.h"

namespace mixxx {
namespace qml {

QmlLibraryProxy::QmlLibraryProxy(QObject* parent)
        : QObject(parent) {
}

QmlLibraryTrackListModel* QmlLibraryProxy::model() const {
    return make_qml_owned<QmlLibraryTrackListModel>(
            QList<QmlLibraryTrackListColumn*>{}, s_pLibrary->trackTableModel())
            .get();
}

void QmlLibraryProxy::analyze(const QmlTrackProxy* track) const {
    VERIFY_OR_DEBUG_ASSERT(track && track->internal()) {
        return;
    }
    Q_EMIT s_pLibrary->analyzeTracks({track->internal()->getId()});
}

// static
QmlLibraryProxy* QmlLibraryProxy::create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine) {
    Q_UNUSED(pJsEngine);
    VERIFY_OR_DEBUG_ASSERT(s_pLibrary) {
        qWarning() << "Library hasn't been registered yet";
        return nullptr;
    }
    return new QmlLibraryProxy(pQmlEngine);
}

} // namespace qml
} // namespace mixxx

#include "moc_qmllibraryproxy.cpp"

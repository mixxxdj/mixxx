#include "qml/qmlsidebarmodelproxy.h"

#include <qalgorithms.h>

#include <QAbstractListModel>
#include <QVariant>
#include <QtDebug>

#include "library/treeitem.h"
#include "moc_qmlsidebarmodelproxy.cpp"
#include "qml/qmllibrarysource.h"
#include "util/assert.h"
#include "util/parented_ptr.h"

namespace mixxx {
namespace qml {

namespace {
const QHash<int, QByteArray> kRoleNames = {
        {Qt::DisplayRole, "label"},
        {QmlSidebarModelProxy::IconRole, "icon"},
};
} // namespace

QHash<int, QByteArray> QmlSidebarModelProxy::roleNames() const {
    return kRoleNames;
}

QVariant QmlSidebarModelProxy::get(int row) const {
    QModelIndex idx = index(row, 0);
    QVariantMap dataMap;
    for (auto it = kRoleNames.constBegin(); it != kRoleNames.constEnd(); it++) {
        dataMap.insert(it.value(), data(idx, it.key()));
    }
    return dataMap;
}

void QmlSidebarModelProxy::activate(const QModelIndex& index) {
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return;
    }
    if (index.internalPointer() == this) {
        VERIFY_OR_DEBUG_ASSERT(index.row() >= 0 && index.row() < m_sFeatures.length()) {
            return;
        }
        m_sFeatures[index.row()]->activate();
    } else {
        TreeItem* pTreeItem = static_cast<TreeItem*>(index.internalPointer());
        VERIFY_OR_DEBUG_ASSERT(pTreeItem != nullptr) {
            return;
        }
        LibraryFeature* pFeature = pTreeItem->feature();
        DEBUG_ASSERT(pFeature);
        pFeature->activateChild(index);
        pFeature->onLazyChildExpandation(index);
    }
}

QmlSidebarModelProxy::QmlSidebarModelProxy(QObject* parent)
        : SidebarModel(parent),
          m_tracklist(nullptr) {
}
QmlSidebarModelProxy::~QmlSidebarModelProxy() = default;

void QmlSidebarModelProxy::update(const QList<QmlLibrarySource*>& sources) {
    beginResetModel();
    qDeleteAll(m_sFeatures);
    for (const auto& librarySource : sources) {
        VERIFY_OR_DEBUG_ASSERT(librarySource) {
            continue;
        }
        connect(librarySource,
                &QmlLibrarySource::requestTrackModel,
                this,
                &QmlSidebarModelProxy::slotShowTrackModel);
        auto* pLibrarySource = librarySource->internal();
        addLibraryFeature(pLibrarySource);
    }
    endResetModel();
}

void QmlSidebarModelProxy::slotShowTrackModel(std::shared_ptr<QmlLibraryTrackListModel> pModel) {
    m_tracklist = pModel;
    emit tracklistChanged();
}

} // namespace qml
} // namespace mixxx

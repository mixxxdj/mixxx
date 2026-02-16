#include "qml/qmllibrarysourcetree.h"

#include <qalgorithms.h>
#include <qvariant.h>

#include <QAbstractListModel>
#include <QVariant>

#include "library/library.h"
#include "library/librarytablemodel.h"
#include "moc_qmllibrarysourcetree.cpp"
#include "qml_owned_ptr.h"
#include "qmllibraryproxy.h"
#include "qmllibrarytracklistmodel.h"
#include "qmlsidebarmodelproxy.h"

namespace mixxx {
namespace qml {

QmlLibrarySourceTree::QmlLibrarySourceTree(QQuickItem* parent)
        : QQuickItem(parent),
          m_model(new QmlSidebarModelProxy(this)) {
}
QmlLibrarySourceTree::~QmlLibrarySourceTree() = default;

Q_INVOKABLE QmlLibraryTrackListModel* QmlLibrarySourceTree::allTracks() const {
    return make_qml_owned<QmlLibraryTrackListModel>(
            m_defaultColumns, QmlLibraryProxy::get()->trackTableModel());
};

void QmlLibrarySourceTree::append_source(
        QQmlListProperty<QmlLibraryAbstractSource>* list, QmlLibraryAbstractSource* source) {
    reinterpret_cast<QList<QmlLibraryAbstractSource*>*>(list->data)->append(source);
    QmlLibrarySourceTree* librarySourceTree = qobject_cast<QmlLibrarySourceTree*>(list->object);
    if (librarySourceTree && librarySourceTree->isComponentComplete()) {
        librarySourceTree->m_model->update(librarySourceTree->m_sources);
    }
}

void QmlLibrarySourceTree::clear_source(QQmlListProperty<QmlLibraryAbstractSource>* p) {
    reinterpret_cast<QList<QmlLibraryAbstractSource*>*>(p->data)->clear();
    QmlLibrarySourceTree* librarySourceTree = qobject_cast<QmlLibrarySourceTree*>(p->object);
    if (librarySourceTree) {
        librarySourceTree->m_model->update(librarySourceTree->m_sources);
    }
}
void QmlLibrarySourceTree::replace_source(QQmlListProperty<QmlLibraryAbstractSource>* p,
        qsizetype idx,
        QmlLibraryAbstractSource* v) {
    return reinterpret_cast<QList<QmlLibraryAbstractSource*>*>(p->data)->replace(idx, v);
    QmlLibrarySourceTree* librarySourceTree = qobject_cast<QmlLibrarySourceTree*>(p->object);
    if (librarySourceTree && librarySourceTree->isComponentComplete()) {
        librarySourceTree->m_model->update(librarySourceTree->m_sources);
    }
}
void QmlLibrarySourceTree::removeLast_source(QQmlListProperty<QmlLibraryAbstractSource>* p) {
    return reinterpret_cast<QList<QmlLibraryAbstractSource*>*>(p->data)->removeLast();
    QmlLibrarySourceTree* librarySourceTree = qobject_cast<QmlLibrarySourceTree*>(p->object);
    if (librarySourceTree && librarySourceTree->isComponentComplete()) {
        librarySourceTree->m_model->update(librarySourceTree->m_sources);
    }
}

QQmlListProperty<QmlLibraryAbstractSource> QmlLibrarySourceTree::sources() {
    return QQmlListProperty<QmlLibraryAbstractSource>(this,
            &m_sources,
            &QmlLibrarySourceTree::append_source,
            &QmlLibrarySourceTree::count_source,
            &QmlLibrarySourceTree::at_source,
            &QmlLibrarySourceTree::clear_source,
            &QmlLibrarySourceTree::replace_source,
            &QmlLibrarySourceTree::removeLast_source);
}

void QmlLibrarySourceTree::componentComplete() {
    m_model->update(m_sources);
}

} // namespace qml
} // namespace mixxx

#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QQmlEngine>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QQuickItem>
#include <QVariant>

#include "qmllibrarytracklistcolumn.h"
#include "qmlsidebarmodelproxy.h"
#include "util/parented_ptr.h"

namespace mixxx {
namespace qml {

class QmlLibrarySourceTree : public QQuickItem {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlListProperty<QmlLibraryAbstractSource> sources READ sources)
    Q_PROPERTY(QQmlListProperty<QmlLibraryTrackListColumn> defaultColumns READ
                    defaultColumns CONSTANT)
    Q_CLASSINFO("DefaultProperty", "sources")
    QML_NAMED_ELEMENT(LibrarySourceTree)

  public:
    Q_DISABLE_COPY_MOVE(QmlLibrarySourceTree)
    explicit QmlLibrarySourceTree(QQuickItem* parent = nullptr);
    ~QmlLibrarySourceTree() override;

    void componentComplete() override;

    QQmlListProperty<QmlLibraryTrackListColumn> defaultColumns() {
        return {this, &m_defaultColumns};
    }

    QQmlListProperty<QmlLibraryAbstractSource> sources();
    Q_INVOKABLE mixxx::qml::QmlSidebarModelProxy* sidebar() const {
        return m_model.get();
    };
    Q_INVOKABLE mixxx::qml::QmlLibraryTrackListModel* allTracks() const;

  private:
    static void append_source(QQmlListProperty<QmlLibraryAbstractSource>* list,
            QmlLibraryAbstractSource* slice);
    static qsizetype count_source(QQmlListProperty<QmlLibraryAbstractSource>* p) {
        return reinterpret_cast<QList<QmlLibraryAbstractSource*>*>(p->data)->size();
    }
    static QmlLibraryAbstractSource* at_source(
            QQmlListProperty<QmlLibraryAbstractSource>* p, qsizetype idx) {
        return reinterpret_cast<QList<QmlLibraryAbstractSource*>*>(p->data)->at(idx);
    }
    static void clear_source(QQmlListProperty<QmlLibraryAbstractSource>* p);
    static void replace_source(QQmlListProperty<QmlLibraryAbstractSource>* p,
            qsizetype idx,
            QmlLibraryAbstractSource* v);
    static void removeLast_source(QQmlListProperty<QmlLibraryAbstractSource>* p);

    QList<QmlLibraryAbstractSource*> m_sources;
    parented_ptr<QmlSidebarModelProxy> m_model;
    QList<QmlLibraryTrackListColumn*> m_defaultColumns;
};

} // namespace qml
} // namespace mixxx

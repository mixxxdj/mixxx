#pragma once

#include <qobject.h>

#include <QAbstractItemModel>
#include <QObject>
#include <QQmlEngine>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QQuickItem>
#include <QVariant>
#include <memory>

#include "library/browse/browsefeature.h"
#include "library/libraryfeature.h"
#include "library/sidebarmodel.h"
#include "library/trackset/crate/cratefeature.h"
#include "library/trackset/playlistfeature.h"
#include "library/treeitem.h"
#include "qmlcrateproxy.h"
#include "qmllibraryproxy.h"
#include "qmlplaylistproxy.h"
#include "util/parented_ptr.h"

class LibraryTableModel;
class TreeItemModel;
class AllTrackLibraryFeature final : public LibraryFeature {
    Q_OBJECT
  public:
    AllTrackLibraryFeature(Library* pLibrary,
            UserSettingsPointer pConfig);
    ~AllTrackLibraryFeature() override = default;

    QVariant title() override {
        return tr("All...");
    }
    TreeItemModel* sidebarModel() const override {
        return m_pSidebarModel;
    }

    bool hasTrackTable() override {
        return true;
    }

    LibraryTableModel* trackTableModel() const {
        return m_pLibraryTableModel;
    }

    void searchAndActivate(const QString& query);

  public slots:
    void activate() override;

  private:
    LibraryTableModel* m_pLibraryTableModel;

    parented_ptr<TreeItemModel> m_pSidebarModel;
};

namespace mixxx {
namespace qml {

class QmlLibraryTrackListColumn;

class QmlLibrarySource : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString label MEMBER m_label NOTIFY labelChanged)
    Q_PROPERTY(QString itemName MEMBER m_itemName NOTIFY itemNameChanged)
    Q_PROPERTY(QString icon MEMBER m_icon NOTIFY iconChanged)
    Q_PROPERTY(bool creatable MEMBER m_creatable NOTIFY creatableChanged)
    Q_PROPERTY(QQmlListProperty<QmlLibraryTrackListColumn> columns READ columnsQml)
    Q_CLASSINFO("DefaultProperty", "columns")
    QML_NAMED_ELEMENT(LibrarySource)
    QML_UNCREATABLE("Only accessible via its specialization")
  public:
    explicit QmlLibrarySource(QObject* parent = nullptr,
            const QList<QmlLibraryTrackListColumn*>& columns = {});

    QQmlListProperty<QmlLibraryTrackListColumn> columnsQml() {
        return {this, &m_columns};
    }

    const QList<QmlLibraryTrackListColumn*>& columns() const {
        return m_columns;
    }

    const QString& label() const {
        return m_label;
    }

    const QString& icon() const {
        return m_icon;
    }

    const QString& itemName() const {
        return m_itemName;
    }

    bool creatable() const {
        return m_creatable;
    }
    virtual LibraryFeature* internal() = 0;
  public slots:
    void slotShowTrackModel(QAbstractItemModel* pModel);

  signals:
    void requestTrackModel(std::shared_ptr<QmlLibraryTrackListModel> pModel);
    void labelChanged();
    void itemNameChanged();
    void iconChanged();
    void creatableChanged();

  protected:
    QString m_label;
    QString m_itemName;
    QString m_icon;
    bool m_creatable;
    QList<QmlLibraryTrackListColumn*> m_columns;
};

class QmlLibraryPlaylistSource : public QmlLibrarySource {
    Q_OBJECT
    QML_NAMED_ELEMENT(LibraryPlaylistSource)
  public:
    explicit QmlLibraryPlaylistSource(QObject* parent = nullptr,
            const QList<QmlLibraryTrackListColumn*>& columns = {});

    LibraryFeature* internal() override {
        return m_pLibraryFeature.get();
    }
    Q_INVOKABLE void create(const QString& name) const;
    Q_INVOKABLE QList<mixxx::qml::QmlPlaylistProxy*> list();

  private:
    std::unique_ptr<PlaylistFeature> m_pLibraryFeature;
};
class QmlLibraryAllTrackSource : public QmlLibrarySource {
    Q_OBJECT
    QML_NAMED_ELEMENT(LibraryAllTrackSource)
  public:
    explicit QmlLibraryAllTrackSource(QObject* parent = nullptr,
            const QList<QmlLibraryTrackListColumn*>& columns = {});

    LibraryFeature* internal() override {
        return m_pLibraryFeature.get();
    }

  private:
    std::unique_ptr<AllTrackLibraryFeature> m_pLibraryFeature;
};

class QmlLibraryCrateSource : public QmlLibrarySource {
    Q_OBJECT
    QML_NAMED_ELEMENT(LibraryCrateSource)
  public:
    explicit QmlLibraryCrateSource(QObject* parent = nullptr,
            const QList<QmlLibraryTrackListColumn*>& columns = {});

    LibraryFeature* internal() override {
        return m_pLibraryFeature.get();
    }
    Q_INVOKABLE void create(const QString& name) const;
    Q_INVOKABLE QList<mixxx::qml::QmlCrateProxy*> list(
            const QList<mixxx::qml::QmlTrackProxy*>& tracks);

  private:
    std::unique_ptr<CrateFeature> m_pLibraryFeature;
};

class QmlLibraryExplorerSource : public QmlLibrarySource {
    Q_OBJECT
    QML_NAMED_ELEMENT(LibraryExplorerSource)
  public:
    explicit QmlLibraryExplorerSource(QObject* parent = nullptr,
            const QList<QmlLibraryTrackListColumn*>& columns = {});

    LibraryFeature* internal() override {
        return m_pLibraryFeature.get();
    }

  private:
    std::unique_ptr<BrowseFeature> m_pLibraryFeature;
};

} // namespace qml
} // namespace mixxx

class LibraryTableModel;
class TreeItemModel;

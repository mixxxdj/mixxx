#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QQmlEngine>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QQuickItem>
#include <QVariant>
#include <memory>

#include "library/libraryfeature.h"
#include "library/sidebarmodel.h"
#include "qmllibrarytracklistmodel.h"
#include "util/parented_ptr.h"

namespace mixxx {
namespace qml {

class QmlLibrarySource;

class QmlSidebarModelProxy : public SidebarModel {
    Q_OBJECT
    Q_PROPERTY(QmlLibraryTrackListModel* tracklist READ tracklist NOTIFY tracklistChanged)
    QML_ANONYMOUS
  public:
    enum Roles {
        LabelRole = SidebarModel::DataRole + 1,
        IconRole,
        ItemNameRole,
        CapabilitiesRole,
    };
    Q_ENUM(Roles);
    Q_DISABLE_COPY_MOVE(QmlSidebarModelProxy)
    explicit QmlSidebarModelProxy(QObject* parent = nullptr);
    ~QmlSidebarModelProxy() override;

    QmlLibraryTrackListModel* tracklist() const {
        return m_tracklist.get();
    }

    QVariant data(const QModelIndex& index,
            int role = Qt::DisplayRole) const override;
    void update(const QList<QmlLibrarySource*>& sources);
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant get(int row) const;
    Q_INVOKABLE void activate(const QModelIndex& index);
  signals:
    void tracklistChanged();

  protected slots:
    void slotShowTrackModel(std::shared_ptr<mixxx::qml::QmlLibraryTrackListModel> pModel);

  private:
    std::shared_ptr<QmlLibraryTrackListModel> m_tracklist;
    QList<QmlLibrarySource*> m_pQmlFeatures;
};

} // namespace qml
} // namespace mixxx

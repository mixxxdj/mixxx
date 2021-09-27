#pragma once
#include <QIdentityProxyModel>

class LibraryTableModel;

namespace mixxx {
namespace skin {
namespace qml {

class QmlLibraryTrackListModel : public QIdentityProxyModel {
  public:
    enum Roles {
        TitleRole = Qt::UserRole,
        ArtistRole,
        AlbumRole,
        AlbumArtistRole,
        FileUrlRole,
    };
    Q_ENUM(Roles);

    QmlLibraryTrackListModel(LibraryTableModel* pModel, QObject* pParent = nullptr);
    ~QmlLibraryTrackListModel() = default;

    QVariant data(const QModelIndex& index, int role) const override;
    int columnCount(const QModelIndex& index = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
};

} // namespace qml
} // namespace skin
} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::skin::qml::QmlLibraryTrackListModel*)

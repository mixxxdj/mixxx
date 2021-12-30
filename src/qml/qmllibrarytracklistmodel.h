#pragma once
#include <QIdentityProxyModel>
#include <QtQml>

class LibraryTableModel;

namespace mixxx {
namespace qml {

class QmlLibraryTrackListModel : public QIdentityProxyModel {
    Q_OBJECT
    QML_NAMED_ELEMENT(LibraryTrackListModel)
    QML_UNCREATABLE("Only accessible via Mixxx.Library.model")

  public:
    enum Roles {
        AlbumArtistRole = Qt::UserRole,
        AlbumRole,
        ArtistRole,
        BitrateRole,
        BpmLockRole,
        BpmRole,
        ColorRole,
        CommentRole,
        ComposerRole,
        CoverArtColorRole,
        CoverArtUrlRole,
        DatetimeAddedRole,
        DeletedRole,
        DurationSecondsRole,
        FileTypeRole,
        FileUrlRole,
        GenreRole,
        GroupingRole,
        KeyIdRole,
        KeyRole,
        LastPlayedAtRole,
        PlayedRole,
        RatingRole,
        ReplayGainRole,
        TimesPlayedRole,
        TitleRole,
        TrackNumberRole,
        YearRole,
    };
    Q_ENUM(Roles);

    QmlLibraryTrackListModel(LibraryTableModel* pModel, QObject* pParent = nullptr);
    ~QmlLibraryTrackListModel() = default;

    QVariant data(const QModelIndex& index, int role) const override;
    int columnCount(const QModelIndex& index = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant get(int row) const;
};

} // namespace qml
} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::qml::QmlLibraryTrackListModel*)

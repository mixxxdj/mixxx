#pragma once

#include "library/trackset/genre/genreid.h"
#include "library/trackset/tracksettablemodel.h"

class GenreTableModel final : public TrackSetTableModel {
    Q_OBJECT

  public:
    GenreTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager);
    ~GenreTableModel() final = default;

    void selectGenre(GenreId genreId = GenreId());
    GenreId selectedGenre() const {
        return m_selectedGenre;
    }

    bool addTrack(const QModelIndex& index, const QString& location);

    void removeTracks(const QModelIndexList& indices) final;
    /// Returns the number of unsuccessful additions.
    int addTracksWithTrackIds(const QModelIndex& index,
            const QList<TrackId>& tracks,
            int* pOutInsertionPos) final;
    bool isLocked() final;

    Capabilities getCapabilities() const final;
    QString modelKey(bool noSearch) const override;
    void importModelFromCsv();
    void rebuildGenreNames();
    // void editGenre(const Genre& genre);
    void editGenre(GenreId genreId);
    void setAllGenresVisible();
    void setGenreInvisible(const GenreId& genreId);
    void EditGenresMulti();
    void EditOrphanTrackGenres();
    void exportGenresToCsv();
    void importGenresFromCsv();

  private:
    GenreId m_selectedGenre;
    QHash<GenreId, QString> m_searchTexts;
    // QMap<QString, QVariantList> m_orphanToTrackIds;
    QMap<QString, QList<int>> m_orphanToTrackIds;
};

#pragma once
#include <QString>

#include "library/dao/dao.h"
#include "library/trackset/genre/genreid.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/class.h"

class QSqlDatabase;
class Genre;

class GenreDao : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    explicit GenreDao(QObject* parent = nullptr);
    ~GenreDao() override = default;

    bool readGenreById(GenreId id, Genre* pGenre) const;
    bool insertGenre(const Genre& genre, GenreId* pInsertedId);

    void initialize(const QSqlDatabase& database) override;
    void loadGenres2QVL(QVariantList& m_genreData);
    // QString getDisplayGenreNameForGenreID(const QString& rawGenre) const;
    QString getDisplayGenreNameForGenreID(const QString& rawGenre);

    // QString getIdsForGenreNames(const QString& genreText) const;
    QString getIdsForGenreNames(const QString& genreText);

    QStringList getGenreNameList() const;
    QMap<QString, QString> getAllGenres();

    // qint64 getGenreId(const QString& genreName) const;
    qint64 getGenreId(const QString& genreName);

    QHash<QString, qint64> getNameToIdMap() const;
    QHash<qint64, QString> getIdToNameMap() const;
    QList<GenreId> getGenreIdsFromIdString(const QString& genreIdString);

    bool updateGenreTracksForTrack(
            TrackId trackId,
            const QList<GenreId>& genreIds);

  private:
    QVariantList m_genreData;
};

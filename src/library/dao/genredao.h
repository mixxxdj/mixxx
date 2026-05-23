#pragma once

#include <QMap>
#include <QRegularExpression>
#include <QString>
#include <QVariantList>

#include "library/dao/dao.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"
#include "util/class.h"

class QSqlDatabase;

class GenreDao : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    explicit GenreDao(QObject* parent = nullptr);
    ~GenreDao() override = default;

    void initialize(const QSqlDatabase& database) override;
    void loadGenres2QVL(QVariantList& genreData);
    QString getDisplayGenreNameForGenreID(const QString& rawGenre) const;
    QMap<QString, QString> getAllGenres();

    QStringList getAllGenreNames() const;
    QStringList getGenresForTrack(TrackId trackId) const;
    bool setGenresForTrack(TrackId trackId, const QStringList& genreNames);
    int createGenre(const QString& genreName);

  private:
    QVariantList m_genreData;

    DISALLOW_COPY_AND_ASSIGN(GenreDao);
};

#pragma once
#include <QString>

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
    void loadGenres2QVL(QVariantList& m_genreData);
    QString getDisplayGenreNameForGenreID(const QString& rawGenre) const;
    QString getIdsForGenreNames(const QString& genreText) const;
    QStringList getGenreNameList() const;
    QMap<QString, QString> getAllGenres();
    qint64 getGenreId(const QString& genreName) const;
    QHash<QString, qint64> getNameToIdMap() const;
    QHash<qint64, QString> getIdToNameMap() const;

  private:
    QVariantList m_genreData;
};

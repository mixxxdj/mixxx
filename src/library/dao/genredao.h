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
    QMap<QString, QString> getAllGenres();

  private:
    QVariantList m_genreData;
};

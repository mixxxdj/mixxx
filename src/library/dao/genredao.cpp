#include "library/dao/genredao.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "library/dao/trackschema.h"
#include "util/db/dbconnection.h"
#include "util/db/sqlite.h" // For SQuery and LOG_FAILED_QUERY
#include "util/db/sqltransaction.h"
#include "util/logger.h"

namespace {
mixxx::Logger kLogger("GenreDAO");
}

// base DAO constructor
GenreDAO::GenreDAO(DbConnection* dbConnection)
        : DAO() {
    setDb(dbConnection);
}

DbId GenreDAO::addGenre(const QString& name) {
    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        return invalidId();
    }

    Genre existingGenre = getGenreByName(trimmedName);
    if (existingGenre.id != invalidId()) {
        return existingGenre.id;
    }

    QSqlQuery query = SQuery(m_pDb);
    query.prepare(QString("INSERT INTO %1 (%2) VALUES (:name)")
                    .arg(TableGenres, GenresName));
    query.bindValue(":name", trimmedName);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Failed to insert new genre:" << trimmedName;
        return invalidId();
    }
    return query.lastInsertId().toLongLong();
}

Genre GenreDAO::getGenreByName(const QString& name) {
    QSqlQuery query = SQuery(m_pDb);
    query.prepare(QString("SELECT %1, %2 FROM %3 WHERE %2 = :name")
                    .arg(GenresId, GenresName, TableGenres));
    query.bindValue(":name", name);

    if (!query.exec() || !query.next()) {
        if (query.lastError().type() != QSqlError::NoError) {
            LOG_FAILED_QUERY(query) << "Failed to get genre by name:" << name;
        }
        return Genre();
    }
    return Genre{query.value(0).toLongLong(), query.value(1).toString()};
}

Genre GenreDAO::getGenreById(DbId id) {
    if (id == invalidId()) {
        return Genre();
    }
    QSqlQuery query = SQuery(m_pDb);
    query.prepare(QString("SELECT %1, %2 FROM %3 WHERE %1 = :id")
                    .arg(GenresId, GenresName, TableGenres));
    query.bindValue(":id", QVariant::fromValue(id));

    if (!query.exec() || !query.next()) {
        return Genre();
    }
    return Genre{query.value(0).toLongLong(), query.value(1).toString()};
}

QList<Genre> GenreDAO::getGenresForTrack(DbId trackId) {
    QList<Genre> genres;
    if (trackId == invalidId()) {
        return genres;
    }

    QSqlQuery query = SQuery(m_pDb);
    query.prepare(
            QString("SELECT g.%1, g.%2 FROM %3 AS g "
                    "JOIN %4 AS gt ON g.%1 = gt.%5 "
                    "WHERE gt.%6 = :trackId "
                    "ORDER BY g.%2")
                    .arg(GenresId,
                            GenresName,
                            TableGenres,
                            TableGenreTracks,
                            GenreTracksGenreId,
                            GenreTracksTrackId));
    query.bindValue(":trackId", QVariant::fromValue(trackId));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Failed to get genres for track id:" << trackId;
        return genres;
    }

    while (query.next()) {
        genres.append(Genre{query.value(0).toLongLong(), query.value(1).toString()});
    }
    return genres;
}

bool GenreDAO::setGenresForTrack(DbId trackId, const QList<DbId>& genreIds) {
    if (trackId == invalidId()) {
        return false;
    }

    SqlTransaction transaction(m_pDb);
    if (!transaction.begin()) {
        kLogger.warning() << "Failed to begin transaction for setGenresForTrack";
        return false;
    }

    QSqlQuery deleteQuery = SQuery(m_pDb);
    deleteQuery.prepare(QString("DELETE FROM %1 WHERE %2 = :trackId")
                    .arg(TableGenreTracks, GenreTracksTrackId));
    deleteQuery.bindValue(":trackId", QVariant::fromValue(trackId));
    if (!deleteQuery.exec()) {
        LOG_FAILED_QUERY(deleteQuery);
        transaction.rollback();
        return false;
    }

    if (!genreIds.isEmpty()) {
        QSqlQuery insertQuery = SQuery(m_pDb);
        insertQuery.prepare(QString("INSERT INTO %1 (%2, %3) VALUES (:trackId, :genreId)")
                        .arg(TableGenreTracks, GenreTracksTrackId, GenreTracksGenreId));

        for (const DbId genreId : genreIds) {
            if (genreId != invalidId()) {
                insertQuery.bindValue(":trackId", QVariant::fromValue(trackId));
                insertQuery.bindValue(":genreId", QVariant::fromValue(genreId));
                if (!insertQuery.exec()) {
                    LOG_FAILED_QUERY(insertQuery);
                    transaction.rollback();
                    return false;
                }
            }
        }
    }

    return transaction.commit();
}

QList<Genre> GenreDAO::getAllGenres() {
    QList<Genre> genres;
    QSqlQuery query = SQuery(m_pDb);
    query.prepare(QString("SELECT %1, %2 FROM %3 ORDER BY %2")
                    .arg(GenresId, GenresName, TableGenres));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Failed to get all genres";
        return genres;
    }

    while (query.next()) {
        genres.append(Genre{query.value(0).toLongLong(), query.value(1).toString()});
    }
    return genres;
}

int GenreDAO::deleteUnusedGenres() {
    const QString sql = QString("DELETE FROM %1 WHERE %2 NOT IN (SELECT DISTINCT %3 FROM %4)")
                                .arg(TableGenres, GenresId, GenreTracksGenreId, TableGenreTracks);

    QSqlQuery query = SQuery(m_pDb);
    if (!query.exec(sql)) {
        LOG_FAILED_QUERY(query) << "Failed to delete unused genres";
        return -1;
    }
    return query.numRowsAffected();
}

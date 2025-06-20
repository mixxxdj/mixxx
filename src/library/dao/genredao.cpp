#include "library/dao/genredao.h"

#include "library/dao/trackschema.h"
#include "util/db/dbconnection.h"
#include "util/db/sqlite.h" // Include the SQLite header for SQuery
#include "util/db/sqltransaction.h"
#include "util/logger.h"

namespace {
mixxx::Logger kLogger("GenreDAO");
}

GenreDAO::GenreDAO(DbConnection* dbConnection)
        : DAO(dbConnection) {
}

DbId GenreDAO::addGenre(const QString& name) {
    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        return DbId_Invalid;
    }

    Genre existingGenre = getGenreByName(trimmedName);
    if (existingGenre.id != DbId_Invalid) {
        return existingGenre.id;
    }

    QSqlQuery query = SQuery(m_pDb);
    query.prepare(QString("INSERT INTO %1 (%2) VALUES (:name)")
                    .arg(mixxx::trackschema::TableGenres,
                            mixxx::trackschema::GenresName));
    query.bindValue(":name", trimmedName);

    if (!exec(query)) {
        LOG_FAILED_QUERY(query) << "Failed to insert new genre:" << trimmedName;
        return DbId_Invalid;
    }
    return query.lastInsertId().toLongLong();
}

Genre GenreDAO::getGenreByName(const QString& name) {
    QSqlQuery query = SQuery(m_pDb);
    query.prepare(QString("SELECT %1, %2 FROM %3 WHERE %2 = :name")
                    .arg(mixxx::trackschema::GenresId,
                            mixxx::trackschema::GenresName,
                            mixxx::trackschema::TableGenres));
    query.bindValue(":name", name);

    if (!exec(query)) {
        LOG_FAILED_QUERY(query) << "Failed to get genre by name:" << name;
        return Genre();
    }
    if (!query.next()) {
        return Genre(); // Not found, return invalid genre
    }
    return Genre{query.value(0).toLongLong(), query.value(1).toString()};
}

Genre GenreDAO::getGenreById(DbId id) {
    if (id == DbId_Invalid) {
        return Genre();
    }
    QSqlQuery query = SQuery(m_pDb);
    query.prepare(QString("SELECT %1, %2 FROM %3 WHERE %1 = :id")
                    .arg(mixxx::trackschema::GenresId,
                            mixxx::trackschema::GenresName,
                            mixxx::trackschema::TableGenres));
    query.bindValue(":id", id);

    if (!exec(query) || !query.next()) {
        return Genre();
    }
    return Genre{query.value(0).toLongLong(), query.value(1).toString()};
}

QList<Genre> GenreDAO::getGenresForTrack(DbId trackId) {
    QList<Genre> genres;
    if (trackId == DbId_Invalid) {
        return genres;
    }

    QSqlQuery query = SQuery(m_pDb);
    query.prepare(QString(
            "SELECT g.%1, g.%2 FROM %3 g "
            "JOIN %4 gt ON g.%1 = gt.%5 "
            "WHERE gt.%6 = :trackId "
            "ORDER BY g.%2")
                    .arg(mixxx::trackschema::GenresId,
                            mixxx::trackschema::GenresName,
                            mixxx::trackschema::TableGenres,
                            mixxx::trackschema::TableGenreTracks,
                            mixxx::trackschema::GenreTracksGenreId,
                            mixxx::trackschema::GenreTracksTrackId));
    query.bindValue(":trackId", trackId);

    if (!exec(query)) {
        LOG_FAILED_QUERY(query) << "Failed to get genres for track id:" << trackId;
        return genres;
    }

    while (query.next()) {
        genres.append(Genre{query.value(0).toLongLong(), query.value(1).toString()});
    }
    return genres;
}

bool GenreDAO::setGenresForTrack(DbId trackId, const QList<DbId>& genreIds) {
    if (trackId == DbId_Invalid) {
        return false;
    }

    SqlTransaction transaction(m_pDb);
    if (!transaction.begin()) {
        kLogger.warning() << "Failed to begin transaction for setGenresForTrack";
        return false;
    }

    // 1. Delete all existing genre associations for this track.
    QSqlQuery deleteQuery = SQuery(m_pDb);
    deleteQuery.prepare(QString("DELETE FROM %1 WHERE %2 = :trackId")
                    .arg(mixxx::trackschema::TableGenreTracks,
                            mixxx::trackschema::GenreTracksTrackId));
    deleteQuery.bindValue(":trackId", trackId);
    if (!exec(deleteQuery)) {
        transaction.rollback();
        return false;
    }

    // 2. Insert the new associations.
    if (!genreIds.isEmpty()) {
        QSqlQuery insertQuery = SQuery(m_pDb);
        insertQuery.prepare(QString("INSERT INTO %1 (%2, %3) VALUES (:trackId, :genreId)")
                        .arg(mixxx::trackschema::TableGenreTracks,
                                mixxx::trackschema::GenreTracksTrackId,
                                mixxx::trackschema::GenreTracksGenreId));

        for (const DbId genreId : genreIds) {
            if (genreId != DbId_Invalid) {
                insertQuery.bindValue(":trackId", trackId);
                insertQuery.bindValue(":genreId", genreId);
                if (!exec(insertQuery)) {
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
    // Using a direct query as shorthand is not available/needed.
    QSqlQuery query = SQuery(m_pDb);
    query.prepare(QString("SELECT %1, %2 FROM %3 ORDER BY %2")
                    .arg(mixxx::trackschema::GenresId,
                            mixxx::trackschema::GenresName,
                            mixxx::trackschema::TableGenres));

    if (!exec(query)) {
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
                                .arg(mixxx::trackschema::TableGenres,
                                        mixxx::trackschema::GenresId,
                                        mixxx::trackschema::GenreTracksGenreId,
                                        mixxx::trackschema::TableGenreTracks);

    QSqlQuery query = SQuery(m_pDb);
    if (!query.exec(sql)) {
        LOG_FAILED_QUERY(query) << "Failed to delete unused genres";
        return -1;
    }
    return query.numRowsAffected();
}

#include "library/dao/genredao.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "library/dao/trackschema.h"
#include "util/assert.h"
#include "util/logger.h"

namespace {
mixxx::Logger kLogger("GenreDAO");

#define LOG_FAILED_QUERY(query) qDebug() << __FILE__ << __LINE__ << "FAILED QUERY [" \
                                         << (query).executedQuery() << "]" << (query).lastError()
}

GenreDAO::GenreDAO()
        : DAO() {
}

DbId GenreDAO::addGenre(const QString& name) {
    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        return DbId();
    }

    // Insert new genre
    QSqlQuery query(database());
    query.prepare(QString("INSERT INTO %1 (%2) VALUES (:name)")
                    .arg(TableGenres, GenresName));
    query.bindValue(":name", trimmedName);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Failed to insert new genre:" << trimmedName;
        return DbId();
    }

    return DbId(query.lastInsertId());
}

Genre GenreDAO::getGenreByName(const QString& name) {
    QSqlQuery query(database());
    query.prepare(QString("SELECT %1, %2 FROM %3 WHERE %2 = :name COLLATE NOCASE")
                    .arg(GenresId, GenresName, TableGenres));
    query.bindValue(":name", name);

    if (!query.exec() || !query.next()) {
        if (query.lastError().type() != QSqlError::NoError) {
            LOG_FAILED_QUERY(query) << "Failed to get genre by name:" << name;
        }
        return Genre();
    }

    return Genre{DbId(query.value(0)), query.value(1).toString()};
}

Genre GenreDAO::getGenreById(DbId id) {
    if (!id.isValid()) {
        return Genre();
    }

    QSqlQuery query(database());
    query.prepare(QString("SELECT %1, %2 FROM %3 WHERE %1 = :id")
                    .arg(GenresId, GenresName, TableGenres));
    query.bindValue(":id", id.toVariant());

    if (!query.exec() || !query.next()) {
        return Genre();
    }

    return Genre{DbId(query.value(0)), query.value(1).toString()};
}

QList<Genre> GenreDAO::getGenresForTrack(DbId trackId) {
    QList<Genre> genres;
    if (!trackId.isValid()) {
        return genres;
    }

    QSqlQuery query(database());
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
    query.bindValue(":trackId", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Failed to get genres for track id:" << trackId;
        return genres;
    }

    while (query.next()) {
        genres.append(Genre{DbId(query.value(0)), query.value(1).toString()});
    }

    return genres;
}

bool GenreDAO::setGenresForTrack(DbId trackId, const QList<DbId>& genreIds) {
    if (!trackId.isValid()) {
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        kLogger.warning() << "Failed to begin transaction for setGenresForTrack";
        return false;
    }

    // First remove all existing associations
    QSqlQuery deleteQuery(database());
    deleteQuery.prepare(QString("DELETE FROM %1 WHERE %2 = :trackId")
                    .arg(TableGenreTracks, GenreTracksTrackId));
    deleteQuery.bindValue(":trackId", trackId.toVariant());

    if (!deleteQuery.exec()) {
        LOG_FAILED_QUERY(deleteQuery) << "Failed to delete existing genre associations";
        db.rollback();
        return false;
    }

    // Then add new associations
    if (!genreIds.isEmpty()) {
        QSqlQuery insertQuery(database());
        insertQuery.prepare(QString("INSERT INTO %1 (%2, %3) VALUES (:trackId, :genreId)")
                        .arg(TableGenreTracks, GenreTracksTrackId, GenreTracksGenreId));

        for (const DbId genreId : genreIds) {
            if (genreId.isValid()) {
                insertQuery.bindValue(":trackId", trackId.toVariant());
                insertQuery.bindValue(":genreId", genreId.toVariant());
                if (!insertQuery.exec()) {
                    LOG_FAILED_QUERY(insertQuery) << "Failed to insert genre association";
                    db.rollback();
                    return false;
                }
            }
        }
    }

    // Transaction commit
    if (!db.commit()) {
        kLogger.warning() << "Failed to commit transaction for setGenresForTrack";
        db.rollback();
        return false;
    }

    return true;
}

QList<Genre> GenreDAO::getAllGenres() {
    QList<Genre> genres;

    QSqlQuery query(database());
    query.prepare(QString("SELECT %1, %2 FROM %3 ORDER BY %2")
                    .arg(GenresId, GenresName, TableGenres));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Failed to get all genres";
        return genres;
    }

    while (query.next()) {
        genres.append(Genre{DbId(query.value(0)), query.value(1).toString()});
    }

    return genres;
}

int GenreDAO::deleteUnusedGenres() {
    const QString sql = QString(
            "DELETE FROM %1 WHERE %2 NOT IN "
            "(SELECT DISTINCT %3 FROM %4)")
                                .arg(TableGenres, GenresId, GenreTracksGenreId, TableGenreTracks);

    QSqlQuery query(database());
    if (!query.exec(sql)) {
        LOG_FAILED_QUERY(query) << "Failed to delete unused genres";
        return -1;
    }

    return query.numRowsAffected();
}

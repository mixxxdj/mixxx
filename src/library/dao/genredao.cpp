#include "library/dao/genredao.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QtDebug>

#include "library/queryutil.h"
#include "moc_genredao.cpp"
#include "util/db/dbconnection.h"

GenreDao::GenreDao(QObject* parent)
        : QObject(parent) {
}

void GenreDao::initialize(const QSqlDatabase& database) {
    DAO::initialize(database);
    loadGenres2QVL(m_genreData);
}

void GenreDao::loadGenres2QVL(QVariantList& genreData) {
    genreData.clear();
    QSqlQuery query(m_database);
    query.prepare("SELECT id, name FROM genres ORDER BY name ASC");

    if (query.exec()) {
        while (query.next()) {
            QVariantMap entry;
            entry["id"] = query.value("id").toInt();
            entry["name"] = query.value("name").toString();
            genreData.append(entry);
        }
        qDebug() << "GenreDao::loadGenres2QVL loaded" << genreData.size() << "genres";
    } else {
        qWarning() << "GenreDao::loadGenres2QVL failed:" << query.lastError();
    }
}

QString GenreDao::getDisplayGenreNameForGenreID(const QString& rawGenre) const {
    return rawGenre;
}

QMap<QString, QString> GenreDao::getAllGenres() {
    QMap<QString, QString> genreMap;
    QSqlQuery query(m_database);
    query.prepare("SELECT id, name FROM genres");

    if (query.exec()) {
        while (query.next()) {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();
            genreMap.insert(QString::number(id), name);
        }
    } else {
        qWarning() << "GenreDao::getAllGenres failed:" << query.lastError();
    }

    return genreMap;
}

QStringList GenreDao::getAllGenreNames() const {
    QStringList names;
    for (const QVariant& entry : m_genreData) {
        QVariantMap map = entry.toMap();
        names << map["name"].toString();
    }
    return names;
}

QStringList GenreDao::getGenresForTrack(TrackId trackId) const {
    QStringList genres;
    QSqlQuery query(m_database);
    query.prepare(
            "SELECT g.name FROM genres g "
            "JOIN genre_tracks gt ON g.id = gt.genre_id "
            "WHERE gt.track_id = ? ORDER BY g.name");
    query.bindValue(0, trackId.toVariant());

    if (query.exec()) {
        while (query.next()) {
            genres << query.value(0).toString();
        }
    }

    return genres;
}

bool GenreDao::setGenresForTrack(TrackId trackId, const QStringList& genreNames) {
    if (!m_database.transaction()) {
        return false;
    }

    QSqlQuery deleteQuery(m_database);
    deleteQuery.prepare("DELETE FROM genre_tracks WHERE track_id = ?");
    deleteQuery.bindValue(0, trackId.toVariant());

    if (!deleteQuery.exec()) {
        m_database.rollback();
        return false;
    }

    for (const QString& genreName : genreNames) {
        if (genreName.trimmed().isEmpty()) {
            continue;
        }

        int genreId = createGenre(genreName.trimmed());
        if (genreId > 0) {
            QSqlQuery insertQuery(m_database);
            insertQuery.prepare("INSERT INTO genre_tracks (track_id, genre_id) VALUES (?, ?)");
            insertQuery.bindValue(0, trackId.toVariant());
            insertQuery.bindValue(1, genreId);

            if (!insertQuery.exec()) {
                m_database.rollback();
                return false;
            }
        }
    }

    return m_database.commit();
}

int GenreDao::createGenre(const QString& genreName) {
    QSqlQuery checkQuery(m_database);
    checkQuery.prepare("SELECT id FROM genres WHERE name = ? COLLATE NOCASE");
    checkQuery.bindValue(0, genreName);

    if (checkQuery.exec() && checkQuery.next()) {
        return checkQuery.value(0).toInt();
    }

    QSqlQuery insertQuery(m_database);
    insertQuery.prepare("INSERT INTO genres (name) VALUES (?)");
    insertQuery.bindValue(0, genreName);

    if (insertQuery.exec()) {
        return insertQuery.lastInsertId().toInt();
    }

    return -1;
}

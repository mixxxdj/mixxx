#include "library/dao/genredao.h"

#include <QDebug>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QtDebug>
#include <utility>

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackset/genre/genre.h"
#include "library/trackset/genre/genreschema.h"
#include "moc_genredao.cpp"
#include "util/db/dbconnection.h"
#include "util/db/fwdsqlquery.h"
#include "util/make_const_iterator.h"
#include "util/math.h"

GenreDao::GenreDao(
        QObject* parent)
        : QObject(parent) {
}

void GenreDao::initialize(const QSqlDatabase& database) {
    DAO::initialize(database);
    m_genreData.clear();
    loadGenres2QVL(m_genreData);
}

bool GenreDao::readGenreById(GenreId id, Genre* pGenre) const {
    QSqlQuery query(m_database);
    query.prepare(
            "SELECT id, name, name_level_1, name_level_2, name_level_3, "
            "name_level_4, name_level_5, display_group, display_order, "
            "is_visible, is_model_defined, count, show, locked, autodj_source "
            "FROM genres WHERE id = :id");

    query.bindValue(":id", id.toVariant());

    if (!query.exec()) {
        qWarning() << "[GenreDao] Failed to query genre by ID:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        return false;
    }

    if (pGenre) {
        pGenre->setId(GenreId(query.value(0)));
        pGenre->setName(query.value(1).toString());
        pGenre->setNameLevel1(query.value(2).toString());
        pGenre->setNameLevel2(query.value(3).toString());
        pGenre->setNameLevel3(query.value(4).toString());
        pGenre->setNameLevel4(query.value(5).toString());
        pGenre->setNameLevel5(query.value(6).toString());
        pGenre->setDisplayGroup(query.value(7).toString());
        pGenre->setDisplayOrder(query.value(8).toInt());
        pGenre->setVisible(query.value(9).toBool());
        pGenre->setModelDefined(query.value(10).toBool());
        pGenre->setCount(query.value(11).toInt());
        pGenre->setShow(query.value(12).toInt());
        pGenre->setLocked(query.value(13).toBool());
        pGenre->setAutoDjSource(query.value(14).toBool());
    }
    return true;
}

bool GenreDao::insertGenre(const Genre& genre, GenreId* pInsertedId) {
    QSqlQuery query(m_database);
    query.prepare(
            "INSERT INTO genres (name, name_level_1, name_level_2, name_level_3, "
            "name_level_4, name_level_5, display_group, display_order, "
            "is_visible, is_model_defined, count, show, locked, autodj_source) "
            "VALUES (:name, :lvl1, :lvl2, :lvl3, :lvl4, :lvl5, :display_group, "
            ":display_order, :is_visible, :is_model_defined, :count, :show, :locked, :autodj)");

    query.bindValue(":name", genre.getName());
    query.bindValue(":lvl1", genre.getNameLevel1());
    query.bindValue(":lvl2", genre.getNameLevel2());
    query.bindValue(":lvl3", genre.getNameLevel3());
    query.bindValue(":lvl4", genre.getNameLevel4());
    query.bindValue(":lvl5", genre.getNameLevel5());
    query.bindValue(":display_group", genre.getDisplayGroup());
    query.bindValue(":display_order", genre.getDisplayOrder());
    query.bindValue(":is_visible", genre.isVisible());
    query.bindValue(":is_model_defined", genre.isModelDefined());
    query.bindValue(":count", genre.getCount());
    query.bindValue(":show", genre.getShow());
    query.bindValue(":locked", genre.isLocked());
    query.bindValue(":autodj", genre.isAutoDjSource());

    if (!query.exec()) {
        qWarning() << "[GenreDao] Failed to insert genre:" << query.lastError().text();
        return false;
    }

    if (pInsertedId) {
        *pInsertedId = GenreId(query.lastInsertId());
    }

    return true;
}

void GenreDao::loadGenres2QVL(QVariantList& m_genreData) {
    m_genreData.clear();
    QSqlQuery queryGenre(m_database);
    queryGenre.prepare(
            "SELECT id, name, name_level_1, name_level_2, name_level_3, "
            "name_level_4, name_level_5, display_order, display_group, is_visible, "
            "is_model_defined FROM genres ORDER BY id ASC");
    if (queryGenre.exec()) {
        while (queryGenre.next()) {
            QVariantMap genresEntry;
            genresEntry["id"] = queryGenre.value("id");
            genresEntry["name"] = queryGenre.value("name");
            genresEntry["name_level_1"] = queryGenre.value("name_level_1");
            genresEntry["name_level_2"] = queryGenre.value("name_level_2");
            genresEntry["name_level_3"] = queryGenre.value("name_level_3");
            genresEntry["name_level_4"] = queryGenre.value("name_level_4");
            genresEntry["name_level_5"] = queryGenre.value("name_level_5");
            genresEntry["display_order"] = queryGenre.value("display_order");
            genresEntry["display_group"] = queryGenre.value("display_group");
            genresEntry["is_visible"] = queryGenre.value("is_visible");
            genresEntry["is_model_defined"] = queryGenre.value("is_model_defined");
            m_genreData.append(genresEntry);
        }
    } else {
        qWarning() << "[GenreDao] -> loadGenres2QVL -> Failed:"
                   << queryGenre.lastError();
    }
    // qDebug() << "[GenreDao] -> loadGenres2QVL -> Finished";
    // qDebug() << "[GenreDao] -> loadGenres2QVL contains" << m_genreData;
}

// QString GenreDao::getDisplayGenreNameForGenreID(const QString& rawGenre) const {
QString GenreDao::getDisplayGenreNameForGenreID(const QString& rawGenre) {
    m_genreData.clear();
    loadGenres2QVL(m_genreData);
    // qDebug() << "[GenreDao] -> getDisplayGenreNameForGenreID called";
    const QStringList parts = rawGenre.split(';', Qt::SkipEmptyParts);
    QStringList resolved;

    static QRegularExpression kGenreIdPattern(QStringLiteral(R"(##(\d+)##)"));
    for (const QString& part : std::as_const(parts)) {
        const QString trimmed = part.trimmed();
        const auto match = kGenreIdPattern.match(trimmed);

        if (match.hasMatch()) {
            bool ok = false;
            const qint64 genreId = match.captured(1).toLongLong(&ok);
            // qDebug() << "[GenreDao] -> getDisplayGenreNameForGenreID "
            //            "searching for -> genreId:"
            //         << genreId;
            if (ok) {
                QString name;
                for (const QVariant& entryVar : std::as_const(m_genreData)) {
                    const QVariantMap entry = entryVar.toMap();
                    // qDebug() << "------" << entry["id"].typeName();
                    // qDebug() << "------" << entry["id"].toString();
                    if (entry["id"].toLongLong() == genreId) {
                        // name = entry["name_level_1"].toString();
                        name = entry[GENRETABLE_NAME].toString();

                        // name = entry["custom_name"].toString();
                        break;
                    }
                }
                //    qDebug() << "[GenreDao] -> getDisplayGenreNameForGenreID "
                //                "called -> genreId -> name:"
                //             << name;
                if (!name.isEmpty()) {
                    resolved << name;
                    continue;
                }
            }
        }
        resolved << trimmed; // fallback
    }
    return resolved.join(QStringLiteral("; "));
}

// qint64 GenreDao::getGenreId(const QString& genreName) const {
qint64 GenreDao::getGenreId(const QString& genreName) {
    m_genreData.clear();
    loadGenres2QVL(m_genreData);
    for (const QVariant& var : std::as_const(m_genreData)) {
        const QVariantMap entry = var.toMap();
        if (QString::compare(entry[GENRETABLE_NAME].toString(),
                    genreName,
                    Qt::CaseInsensitive) == 0) {
            return entry["id"].toLongLong();
        }
    }
    return -1;
}

QHash<QString, qint64> GenreDao::getNameToIdMap() const {
    QHash<QString, qint64> map;
    for (const QVariant& var : std::as_const(m_genreData)) {
        const QVariantMap entry = var.toMap();
        map.insert(entry[GENRETABLE_NAME].toString(), entry["id"].toLongLong());
    }
    return map;
}

QHash<qint64, QString> GenreDao::getIdToNameMap() const {
    QHash<qint64, QString> map;
    for (const QVariant& var : std::as_const(m_genreData)) {
        const QVariantMap entry = var.toMap();
        map.insert(entry["id"].toLongLong(), entry[GENRETABLE_NAME].toString());
    }
    return map;
}

QMap<QString, QString> GenreDao::getAllGenres() {
    QMap<QString, QString> genreMap;

    QSqlQuery query(m_database);
    query.prepare("SELECT id, name FROM genres");
    if (query.exec()) {
        while (query.next()) {
            int id = query.value(0).toInt();
            QString name = query.value(1).toString();
            genreMap.insert(QString("##%1##").arg(id), name);
        }
    } else {
        qWarning() << "Failed to fetch genres:" << query.lastError();
    }

    return genreMap;
}

QString GenreDao::getIdsForGenreNames(const QString& genreText) {
    const QStringList parts = genreText.split(';', Qt::SkipEmptyParts);
    QStringList resolved;

    for (const QString& part : parts) {
        const QString trimmed = part.trimmed();
        bool matched = false;

        for (const QVariant& entryVar : std::as_const(m_genreData)) {
            const QVariantMap entry = entryVar.toMap();
            QString name = entry[GENRETABLE_NAME].toString();

            if (QString::compare(name, trimmed, Qt::CaseInsensitive) == 0) {
                qint64 id = entry["id"].toLongLong();
                resolved << QString("##%1##").arg(id);
                matched = true;
                break;
            }
        }
        if (!matched) {
            resolved << trimmed;
        }
    }

    return resolved.join("; ");
}

QStringList GenreDao::getGenreNameList() const {
    QStringList genreNames;
    for (const QVariant& entryVar : m_genreData) {
        const QVariantMap entry = entryVar.toMap();
        // is_visible = 0 -> skip genre
        if (!entry["is_visible"].toBool()) {
            continue;
        }
        genreNames << entry[GENRETABLE_NAME].toString();
    }
    genreNames.removeDuplicates();
    genreNames.sort(Qt::CaseInsensitive);
    return genreNames;
}

QList<GenreId> GenreDao::getGenreIdsFromIdString(const QString& genreIdString) {
    QList<GenreId> genreIds;
    // const QRegularExpression regex(R"(##(\d+)##)");
    static const QRegularExpression regex(R"(##(\d+)##)");
    auto matches = regex.globalMatch(genreIdString);
    while (matches.hasNext()) {
        const auto match = matches.next();
        bool ok = false;
        const auto id = match.captured(1).toLongLong(&ok);
        if (ok) {
            genreIds.append(GenreId(QVariant(id)));
        }
    }
    return genreIds;
}

bool GenreDao::updateGenreTracksForTrack(TrackId trackId, const QList<GenreId>& genreIds) {
    QSqlQuery deleteQuery(m_database);
    deleteQuery.prepare("DELETE FROM genre_tracks WHERE track_id = :trackId");
    deleteQuery.bindValue(":trackId", trackId.toVariant());
    if (!deleteQuery.exec()) {
        qWarning() << "[GenreDao] -> Failed to delete existing genre_tracks for track:" << trackId;
        return false;
    }

    QSqlQuery insertQuery(m_database);
    insertQuery.prepare(
            "INSERT OR IGNORE INTO genre_tracks (track_id, genre_id) "
            "VALUES (:trackId, :genreId)");

    for (const GenreId& genreId : genreIds) {
        insertQuery.bindValue(":trackId", trackId.toVariant());
        insertQuery.bindValue(":genreId", genreId.toVariant());
        if (!insertQuery.exec()) {
            qWarning() << "[GenreDao] -> Failed to insert genre_track:" << insertQuery.lastError();
            return false;
        }
    }

    return true;
}

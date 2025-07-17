#include "library/dao/genredao.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QtDebug>

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "moc_genredao.cpp"
#include "util/db/dbconnection.h"
#include "util/db/fwdsqlquery.h"
#include "util/make_const_iterator.h"
#include "util/math.h"

GenreDao::GenreDao(
        QObject* parent)
        : QObject(parent) {
    m_genreData.clear();
    loadGenres2QVL(m_genreData);
}

void GenreDao::initialize(const QSqlDatabase& database) {
    DAO::initialize(database);
}

void GenreDao::loadGenres2QVL(QVariantList& m_genreData) {
    m_genreData.clear();
    QSqlQuery queryGenre(m_database);
    queryGenre.prepare(
            "SELECT id, name_level_1, name_level_2, name_level_3, "
            "name_level_4, name_level_5, display_order, is_visible, "
            "is_user_defined FROM genres ORDER BY id ASC");
    if (queryGenre.exec()) {
        while (queryGenre.next()) {
            QVariantMap genresEntry;
            genresEntry["id"] = queryGenre.value("id");
            genresEntry["name_level_1"] = queryGenre.value("name_level_1");
            genresEntry["name_level_2"] = queryGenre.value("name_level_2");
            genresEntry["name_level_3"] = queryGenre.value("name_level_3");
            genresEntry["name_level_4"] = queryGenre.value("name_level_4");
            genresEntry["name_level_5"] = queryGenre.value("name_level_5");
            genresEntry["display_order"] = queryGenre.value("display_order");
            genresEntry["is_visible"] = queryGenre.value("is_visible");
            genresEntry["is_user_defined"] = queryGenre.value("is_user_defined");
            m_genreData.append(genresEntry);
        }
    } else {
        qWarning() << "[GenreDao] -> loadGenres2QVL -> Failed:"
                   << queryGenre.lastError();
    }
    qDebug() << "[GenreDao] -> loadGenres2QVL -> Finished";
    qDebug() << "[GenreDao] -> loadGenres2QVL contains" << m_genreData;
}

QString GenreDao::getDisplayGenreNameForGenreID(const QString& rawGenre) const {
    qDebug() << "[GenreDao] -> getDisplayGenreNameForGenreID called";
    const QStringList parts = rawGenre.split(';', Qt::SkipEmptyParts);
    QStringList resolved;

    static QRegularExpression kGenreIdPattern(QStringLiteral(R"(##(\d+)##)"));
    for (const QString& part : parts) {
        const QString trimmed = part.trimmed();
        const auto match = kGenreIdPattern.match(trimmed);

        if (match.hasMatch()) {
            bool ok = false;
            const qint64 genreId = match.captured(1).toLongLong(&ok);
            qDebug() << "[GenreDao] -> getDisplayGenreNameForGenreID "
                        "searching for -> genreId:"
                     << genreId;
            if (ok) {
                QString name;
                for (const QVariant& entryVar : m_genreData) {
                    const QVariantMap entry = entryVar.toMap();
                    // qDebug() << "------" << entry["id"].typeName();
                    // qDebug() << "------" << entry["id"].toString();
                    if (entry["id"].toLongLong() == genreId) {
                        name = entry["name_level_1"].toString();
                        break;
                    }
                }
                qDebug() << "[GenreDao] -> getDisplayGenreNameForGenreID "
                            "called -> genreId -> name:"
                         << name;
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

QMap<QString, QString> GenreDao::getAllGenres() {
    QMap<QString, QString> genreMap;

    QSqlQuery query(m_database);
    query.prepare("SELECT id, name FROM genres"); // or whatever your genre table is
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

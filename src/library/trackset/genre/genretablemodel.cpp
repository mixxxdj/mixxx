#include "library/trackset/genre/genretablemodel.h"

#include <QtDebug>

#include "library/dao/trackschema.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/genre/genre.h"
#include "moc_genretablemodel.cpp"
#include "track/track.h"
#include "util/db/fwdsqlquery.h"

namespace {

const QString kModelName = QStringLiteral("genre");

} // anonymous namespace

GenreTableModel::GenreTableModel(
        QObject* pParent,
        TrackCollectionManager* pTrackCollectionManager)
        : TrackSetTableModel(
                  pParent,
                  pTrackCollectionManager,
                  "mixxx.db.model.genre") {
}

void GenreTableModel::selectGenre(GenreId genreId) {
    // qDebug() << "GenreTableModel::setGenre()" << genreId;
    if (genreId == m_selectedGenre) {
        qDebug() << "Already focused on genre " << genreId;
        // return;
    }
    // Store search text
    QString currSearch = currentSearch();
    if (m_selectedGenre.isValid()) {
        if (!currSearch.trimmed().isEmpty()) {
            m_searchTexts.insert(m_selectedGenre, currSearch);
        } else {
            m_searchTexts.remove(m_selectedGenre);
        }
    }

    m_selectedGenre = genreId;

    QString tableName = QStringLiteral("genre_%1").arg(m_selectedGenre.toString());
    QStringList columns;
    columns << LIBRARYTABLE_ID
            << "'' AS " + LIBRARYTABLE_PREVIEW
            // For sorting the cover art column we give LIBRARYTABLE_COVERART
            // the same value as the cover digest.
            << LIBRARYTABLE_COVERART_DIGEST + " AS " + LIBRARYTABLE_COVERART;
    // We hide files that have been explicitly deleted in the library
    // (mixxx_deleted = 0) from the view.
    // They are kept in the database, because we treat genre membership as a
    // track property, which persist over a hide / unhide cycle.
    QString queryString =
            QString("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
                    "SELECT %2 FROM %3 "
                    "WHERE %4 IN (%5) "
                    "AND %6=0")
                    .arg(tableName,
                            columns.join(","),
                            LIBRARY_TABLE,
                            LIBRARYTABLE_ID,
                            GenreStorage::formatSubselectQueryForGenreTrackIds(
                                    genreId),
                            LIBRARYTABLE_MIXXXDELETED);
    FwdSqlQuery(m_database, queryString).execPrepared();

    columns[0] = LIBRARYTABLE_ID;
    columns[1] = LIBRARYTABLE_PREVIEW;
    columns[2] = LIBRARYTABLE_COVERART;
    setTable(tableName,
            LIBRARYTABLE_ID,
            columns,
            m_pTrackCollectionManager->internalCollection()->getTrackSource());

    // Restore search text
    setSearch(m_searchTexts.value(m_selectedGenre));
    setDefaultSort(fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST), Qt::AscendingOrder);
}

bool GenreTableModel::addTrack(const QModelIndex& index, const QString& location) {
    Q_UNUSED(index);

    // This will only succeed if the file actually exist.
    mixxx::FileInfo fileInfo(location);
    if (!fileInfo.checkFileExists()) {
        qDebug() << "GenreTableModel::addTrack:"
                 << "File" << location << "not found";
        return false;
    }

    // If a track is dropped but it isn't in the library, then add it because
    // the user probably dropped a file from outside Mixxx into this genre.
    // If the track is already contained in the library it will not insert
    // a duplicate. It also handles unremoving logic if the track has been
    // removed from the library recently and re-adds it.
    const TrackPointer pTrack = m_pTrackCollectionManager->getOrAddTrack(
            TrackRef::fromFileInfo(fileInfo));
    if (!pTrack) {
        qDebug() << "GenreTableModel::addTrack:"
                 << "Failed to add track" << location << "to library";
        return false;
    }

    QList<TrackId> trackIds;
    trackIds.append(pTrack->getId());
    if (!m_pTrackCollectionManager->internalCollection()->addGenreTracks(
                m_selectedGenre, trackIds)) {
        qDebug() << "GenreTableModel::addTrack:"
                 << "Failed to add track" << location << "to genre"
                 << m_selectedGenre;
        return false;
    }

    // TODO(rryan) just add the track don't select
    select();
    return true;
}

TrackModel::Capabilities GenreTableModel::getCapabilities() const {
    Capabilities caps =
            Capability::ReceiveDrops |
            Capability::AddToTrackSet |
            Capability::AddToAutoDJ |
            Capability::EditMetadata |
            Capability::LoadToDeck |
            Capability::LoadToSampler |
            Capability::LoadToPreviewDeck |
            Capability::RemoveCrate |
            Capability::ResetPlayed |
            Capability::Hide |
            Capability::RemoveFromDisk |
            Capability::Analyze |
            Capability::Properties |
            Capability::Sorting;

    if (m_selectedGenre.isValid()) {
        Genre genre;
        if (m_pTrackCollectionManager->internalCollection()
                        ->genres()
                        .readGenreById(m_selectedGenre, &genre)) {
            if (genre.isLocked()) {
                caps |= Capability::Locked;
            }
        } else {
            qWarning() << "Failed to read create" << m_selectedGenre;
        }
    }

    return caps;
}

int GenreTableModel::addTracksWithTrackIds(
        const QModelIndex& index, const QList<TrackId>& trackIds, int* pOutInsertionPos) {
    Q_UNUSED(index);

    if (pOutInsertionPos != nullptr) {
        // genre insertion is not done by position, and no duplicates will be added,.
        // 0 indicates this to the caller.
        *pOutInsertionPos = 0;
    }

    // If a track is dropped but it isn't in the library, then add it because
    // the user probably dropped a file from outside Mixxx into this genre.
    if (!m_pTrackCollectionManager->internalCollection()->addGenreTracks(
                m_selectedGenre, trackIds)) {
        qWarning() << "GenreTableModel::addTracks could not add"
                   << trackIds.size() << "tracks to genre" << m_selectedGenre;
        return 0;
    }

    select();
    return trackIds.size();
}

bool GenreTableModel::isLocked() {
    Genre genre;
    if (!m_pTrackCollectionManager->internalCollection()
                    ->genres()
                    .readGenreById(m_selectedGenre, &genre)) {
        qWarning() << "Failed to read create" << m_selectedGenre;
        return false;
    }
    return genre.isLocked();
}

void GenreTableModel::removeTracks(const QModelIndexList& indices) {
    VERIFY_OR_DEBUG_ASSERT(m_selectedGenre.isValid()) {
        return;
    }
    if (indices.empty()) {
        return;
    }

    Genre genre;
    if (!m_pTrackCollectionManager->internalCollection()
                    ->genres()
                    .readGenreById(m_selectedGenre, &genre)) {
        qWarning() << "Failed to read create" << m_selectedGenre;
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(!genre.isLocked()) {
        return;
    }

    QList<TrackId> trackIds;
    trackIds.reserve(indices.size());
    for (const QModelIndex& index : indices) {
        trackIds.append(getTrackId(index));
    }
    if (!m_pTrackCollectionManager->internalCollection()->removeGenreTracks(
                genre.getId(), trackIds)) {
        qWarning() << "Failed to remove tracks from genre" << genre;
        return;
    }

    select();
}

QString GenreTableModel::modelKey(bool noSearch) const {
    if (m_selectedGenre.isValid()) {
        if (noSearch) {
            return kModelName + QChar(':') + m_selectedGenre.toString();
        }
        return kModelName + QChar(':') +
                m_selectedGenre.toString() +
                QChar('#') +
                currentSearch();
    } else {
        if (noSearch) {
            return kModelName;
        }
        return kModelName + QChar('#') +
                currentSearch();
    }
}

int GenreTableModel::importFromCsv(const QString& csvFileName) {
    QFile file(csvFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[GenreTableModel] Failed to open CSV file:" << csvFileName;
        return 0;
    }

    QTextStream in(&file);
    QString header = in.readLine();
    QStringList headerFields = header.split(',');
    if (headerFields.isEmpty() || headerFields[0].trimmed().toLower() != "name_level_1") {
        qWarning() << "[GenreTableModel] Invalid CSV header";
        return 0;
    }

    QSqlQuery selectQuery(m_database);
    selectQuery.prepare(
            "SELECT id FROM genres WHERE "
            "name_level_1 = :lvl1 AND "
            "COALESCE(name_level_2, '') = :lvl2 AND "
            "COALESCE(name_level_3, '') = :lvl3 AND "
            "COALESCE(name_level_4, '') = :lvl4 AND "
            "COALESCE(name_level_5, '') = :lvl5");

    QSqlQuery insertQuery(m_database);
    insertQuery.prepare(
            "INSERT INTO genres (name_level_1, name_level_2, name_level_3, "
            "name_level_4, name_level_5, custom_name, is_visible, is_user_defined) "
            "VALUES (:lvl1, :lvl2, :lvl3, :lvl4, :lvl5, :custom_name, 1, 0)");

    int insertedCount = 0;
    int lineNumber = 1;
    m_database.transaction();

    while (!in.atEnd()) {
        QString line = in.readLine();
        lineNumber++;

        if (line.trimmed().isEmpty()) {
            continue;
        }

        QStringList fields = line.split(',');
        while (fields.size() < 5) {
            fields.append("");
        }

        QStringList levels;
        for (int i = 0; i < 5; ++i) {
            levels << fields[i].trimmed();
        }

        QStringList nonEmptyLevels;
        for (const QString& part : levels) {
            if (!part.isEmpty()) {
                nonEmptyLevels << part;
            }
        }
        if (nonEmptyLevels.isEmpty()) {
            qWarning() << "[GenreTableModel] Line" << lineNumber
                       << "has no non-empty levels, skipping";
            continue;
        }

        const QString customName = nonEmptyLevels.join("//");
        const QString lvl1 = customName;
        const QString& lvl2 = levels[1];
        const QString& lvl3 = levels[2];
        const QString& lvl4 = levels[3];
        const QString& lvl5 = levels[4];

        selectQuery.bindValue(":lvl1", lvl1);
        selectQuery.bindValue(":lvl2", lvl2);
        selectQuery.bindValue(":lvl3", lvl3);
        selectQuery.bindValue(":lvl4", lvl4);
        selectQuery.bindValue(":lvl5", lvl5);

        if (!selectQuery.exec()) {
            qWarning() << "[GenreTableModel] Select failed at line"
                       << lineNumber << ":" << selectQuery.lastError().text();
            continue;
        }

        if (selectQuery.next()) {
            continue; // Already exists
        }

        insertQuery.bindValue(":lvl1", lvl1);
        insertQuery.bindValue(":lvl2", lvl2.isEmpty() ? QVariant(QVariant::String) : lvl2);
        insertQuery.bindValue(":lvl3", lvl3.isEmpty() ? QVariant(QVariant::String) : lvl3);
        insertQuery.bindValue(":lvl4", lvl4.isEmpty() ? QVariant(QVariant::String) : lvl4);
        insertQuery.bindValue(":lvl5", lvl5.isEmpty() ? QVariant(QVariant::String) : lvl5);
        insertQuery.bindValue(":custom_name", customName);

        if (!insertQuery.exec()) {
            qWarning() << "[GenreTableModel] Insert failed at line"
                       << lineNumber << ":" << insertQuery.lastError().text();
            continue;
        }

        insertedCount++;
    }

    if (!m_database.commit()) {
        qWarning() << "[GenreTableModel] Commit failed:" << m_database.lastError().text();
        return insertedCount;
    }

    return insertedCount;
}

void GenreTableModel::rebuildCustomNames() {
    QSqlQuery selectQuery(m_database);
    QSqlQuery updateQuery(m_database);

    selectQuery.prepare(
            "SELECT id, name_level_1, name_level_2, name_level_3, "
            "name_level_4, name_level_5, custom_name "
            "FROM genres");

    updateQuery.prepare(
            "UPDATE genres SET name_level_1 = :lvl1, custom_name = :custom_name WHERE id = :id");

    if (!selectQuery.exec()) {
        qWarning() << "[GenreTableModel] Failed to SELECT genres:"
                   << selectQuery.lastError().text();
        return;
    }

    m_database.transaction();
    int updatedCount = 0;

    while (selectQuery.next()) {
        int id = selectQuery.value(0).toInt();
        QString lvl1 = selectQuery.value(1).toString().trimmed();
        QString lvl2 = selectQuery.value(2).toString().trimmed();
        QString lvl3 = selectQuery.value(3).toString().trimmed();
        QString lvl4 = selectQuery.value(4).toString().trimmed();
        QString lvl5 = selectQuery.value(5).toString().trimmed();
        QString existingCustomName = selectQuery.value(6).toString().trimmed();

        if (lvl1.isEmpty() && !existingCustomName.isEmpty()) {
            lvl1 = existingCustomName;
        }

        QStringList parts;
        for (const QString& part : {lvl1, lvl2, lvl3, lvl4, lvl5}) {
            if (!part.isEmpty()) {
                parts << part;
            }
        }
        const QString newCustomName = parts.join("//");

        updateQuery.bindValue(":lvl1", lvl1);
        updateQuery.bindValue(":custom_name", newCustomName);
        updateQuery.bindValue(":id", id);

        if (!updateQuery.exec()) {
            qWarning() << "[GenreTableModel] Failed to UPDATE genre ID" << id
                       << ":" << updateQuery.lastError().text();
            continue;
        }

        updatedCount++;
    }

    if (!m_database.commit()) {
        qWarning() << "[GenreTableModel] Failed to COMMIT updated custom_name "
                      "values:"
                   << m_database.lastError().text();
        return;
    }

    qDebug() << "[GenreTableModel] Updated custom_name for" << updatedCount << "rows.";
}

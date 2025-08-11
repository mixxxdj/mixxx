#include "library/trackset/genre/genretablemodel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyledItemDelegate>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QVariantList>
#include <QVariantMap>
#include <QtDebug>

#include "library/dao/genredao.h"
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

    // qDebug() << "GenreTableModel::selectGenre -> queryString: " << queryString;
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

void GenreTableModel::importModelFromCsv() {
    int insertedCount = 0;

    QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
            QObject::tr("Confirm CSV-Import"),
            QObject::tr("This action will add all genres of the model defined in the CSV. \n "
                        "It's not possible to reverse this action. \n\n "
                        "The format of the CSV you want to import is very strict,  \n "
                        "the first line of your CSV needs to contain the field_level_names. \n "
                        "(name_level_1 ... name_level_5) \n\n "
                        "Are you sure you want to continue?"),
            QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        qDebug() << "[GenreTableModel] -> importCSV -> Action cancelled by user.";
        QMessageBox::information(nullptr,
                tr("Import Cancelled"),
                tr("Import cancelled by user, no changes made to genres table"));
        return;
    }

    QString csvFileName = QFileDialog::getOpenFileName(
            nullptr,
            tr("Select Genre Model CSV"),
            QString(),
            tr("CSV files (*.csv);;All files (*.*)"));

    if (csvFileName.isEmpty()) {
        return;
    }

    QFile file(csvFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[GenreTableModel] Failed to open CSV file:" << csvFileName;
        QMessageBox::information(nullptr,
                tr("Import Cancelled"),
                tr("Import cancelled, CSV-file can't be opened."));
        return;
    }

    QTextStream in(&file);
    const QString headerLine = in.readLine();
    const QStringList headerFields = headerLine.split(',');
    if (headerFields.isEmpty() || headerFields[0].trimmed().toLower() != "name_level_1") {
        qWarning() << "[GenreTableModel] Invalid CSV header";
        QMessageBox::information(nullptr,
                tr("Import Cancelled"),
                tr("Import cancelled, Invalid CSV header."));
        return;
    }

    QStringList lines;
    QString line;

    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            lines.append(line);
        }
    }
    file.close();

    const int totalLines = lines.size();
    QList<QStringList> newEntries;
    QStringList fields;
    QStringList levels;
    QStringList nonEmpty;
    int lineNumber = 1;

    QSqlQuery selectQuery(m_database);
    selectQuery.prepare(
            "SELECT id FROM genres WHERE "
            "name_level_1 = :lvl1 AND "
            "COALESCE(name_level_2, '') = :lvl2 AND "
            "COALESCE(name_level_3, '') = :lvl3 AND "
            "COALESCE(name_level_4, '') = :lvl4 AND "
            "COALESCE(name_level_5, '') = :lvl5");

    for (const QString& line : std::as_const(lines)) {
        lineNumber++;
        fields.clear();
        fields = line.split(',');
        fields.reserve(5);
        while (fields.size() < 5) {
            fields.append("");
        }

        levels.clear();
        levels.reserve(5);
        for (int i = 0; i < 5; ++i) {
            levels << fields[i].trimmed();
        }

        const QString& lvl1 = levels[0];
        const QString& lvl2 = levels[1];
        const QString& lvl3 = levels[2];
        const QString& lvl4 = levels[3];
        const QString& lvl5 = levels[4];

        if (lvl1.isEmpty()) {
            continue;
        }

        nonEmpty.clear();
        for (const QString& level : std::as_const(levels)) {
            if (!level.isEmpty()) {
                nonEmpty << level;
            }
        }
        const QString name = nonEmpty.join("//");

        QSqlQuery nameCheckQuery(m_database);
        nameCheckQuery.prepare("SELECT id FROM genres WHERE name = :name");
        nameCheckQuery.bindValue(":name", name);
        if (!nameCheckQuery.exec()) {
            qWarning() << "[GenreTableModel] Name check failed at line" << lineNumber
                       << ":" << nameCheckQuery.lastError().text();
            continue;
        }
        if (nameCheckQuery.next()) {
            continue;
        }

        // check if this exact level combination already exists or was added with the csv import
        selectQuery.bindValue(":lvl1", lvl1);
        selectQuery.bindValue(":lvl2", lvl2);
        selectQuery.bindValue(":lvl3", lvl3);
        selectQuery.bindValue(":lvl4", lvl4);
        selectQuery.bindValue(":lvl5", lvl5);

        if (!selectQuery.exec()) {
            qWarning() << "[GenreTableModel] Select failed at line" << lineNumber
                       << ":" << selectQuery.lastError().text();
            continue;
        }

        if (!selectQuery.next()) {
            newEntries.append(levels);
        }
    }

    if (newEntries.isEmpty()) {
        QMessageBox::information(nullptr,
                tr("Import Genres"),
                tr("No new genres found to import."));
        return;
    }

    QMessageBox::StandardButton proceed = QMessageBox::question(nullptr,
            tr("Import Genres from CSV"),
            tr("CSV contains %1 genre rows.\n%2 of them are new and will be "
               "added.\n\nProceed with import?")
                    .arg(totalLines)
                    .arg(newEntries.size()),
            QMessageBox::Yes | QMessageBox::Cancel);

    if (proceed != QMessageBox::Yes) {
        return;
    }

    QSqlQuery insertQuery(m_database);
    insertQuery.prepare(
            "INSERT INTO genres (name_level_1, name_level_2, name_level_3, "
            "name_level_4, name_level_5, name, is_visible, is_model_defined) "
            "VALUES (:lvl1, :lvl2, :lvl3, :lvl4, :lvl5, :name, 1, 1)");

    if (!m_database.transaction()) {
        qWarning() << "[GenreTableModel] Failed to start transaction:"
                   << m_database.lastError().text();
        return;
    }

    for (const QStringList& levels : std::as_const(newEntries)) {
        nonEmpty.clear();
        for (const QString& level : std::as_const(levels)) {
            if (!level.isEmpty()) {
                nonEmpty << level;
            }
        }

        const QString name = nonEmpty.join("//");
        const QString& lvl1 = levels[0];
        const QString& lvl2 = levels[1];
        const QString& lvl3 = levels[2];
        const QString& lvl4 = levels[3];
        const QString& lvl5 = levels[4];

        insertQuery.bindValue(":lvl1", lvl1);
        insertQuery.bindValue(":lvl2", lvl2.isEmpty() ? QVariant() : QVariant(lvl2));
        insertQuery.bindValue(":lvl3", lvl3.isEmpty() ? QVariant() : QVariant(lvl3));
        insertQuery.bindValue(":lvl4", lvl4.isEmpty() ? QVariant() : QVariant(lvl4));
        insertQuery.bindValue(":lvl5", lvl5.isEmpty() ? QVariant() : QVariant(lvl5));
        insertQuery.bindValue(":name", name);

        if (!insertQuery.exec()) {
            qWarning() << "[GenreTableModel] Insert failed:"
                       << insertQuery.lastError().text();
            continue;
        }

        insertedCount++;
    }

    if (!m_database.commit()) {
        qWarning() << "[GenreTableModel] Commit failed:" << m_database.lastError().text();
        QMessageBox::information(nullptr,
                tr("Import Failed"),
                tr("Import failed, no changes were made."));
        return;
    }

    QMessageBox::information(nullptr,
            tr("Import Complete"),
            tr("Imported %1 new genres from CSV.").arg(insertedCount));
}

void GenreTableModel::rebuildGenreNames() {
    QSqlQuery selectQuery(m_database);
    QSqlQuery updateQuery(m_database);

    selectQuery.prepare(
            "SELECT id, name_level_1, name_level_2, name_level_3, "
            "name_level_4, name_level_5, name "
            "FROM genres");

    updateQuery.prepare(
            "UPDATE genres SET name_level_1 = :lvl1, name = :name WHERE id = :id");

    if (!selectQuery.exec()) {
        qWarning() << "[GenreTableModel] Failed to SELECT genres:"
                   << selectQuery.lastError().text();
        return;
    }

    m_database.transaction();
    int updatedCount = 0;

    QStringList parts;
    while (selectQuery.next()) {
        parts.clear();
        int id = selectQuery.value(0).toInt();
        QString lvl1 = selectQuery.value(1).toString().trimmed();
        QString lvl2 = selectQuery.value(2).toString().trimmed();
        QString lvl3 = selectQuery.value(3).toString().trimmed();
        QString lvl4 = selectQuery.value(4).toString().trimmed();
        QString lvl5 = selectQuery.value(5).toString().trimmed();
        QString existingName = selectQuery.value(6).toString().trimmed();

        // If lvl1 is missing but name exists -> possibly flat genre? -> use that as lvl1
        if (lvl1.isEmpty() && !existingName.isEmpty()) {
            lvl1 = existingName;
        }

        for (const QString& part : {lvl1, lvl2, lvl3, lvl4, lvl5}) {
            if (!part.isEmpty()) {
                parts << part;
            }
        }

        const QString concatenatedName = parts.join("//");

        updateQuery.bindValue(":lvl1", lvl1);
        updateQuery.bindValue(":name", concatenatedName);
        updateQuery.bindValue(":id", id);

        if (!updateQuery.exec()) {
            qWarning() << "[GenreTableModel] Failed to UPDATE genre ID" << id
                       << ":" << updateQuery.lastError().text();
            continue;
        }

        updatedCount++;
    }

    if (!m_database.commit()) {
        qWarning() << "[GenreTableModel] Failed to COMMIT updated genre name values:"
                   << m_database.lastError().text();
        return;
    }

    qDebug() << "[GenreTableModel] Updated name for" << updatedCount << "rows.";
}

void GenreTableModel::editGenre(GenreId genreId) {
    qDebug() << "[GenreTableModel] -> editGenre -> genreId:" << genreId;

    Genre genre;
    if (!m_pTrackCollectionManager->internalCollection()->genres().readGenreById(genreId, &genre)) {
        qDebug() << "[GenreTableModel] -> editGenre -> error getting genre";
        return;
    }

    QDialog dialog;
    dialog.setWindowTitle(QObject::tr("Edit Genre"));
    QFormLayout* formLayout = new QFormLayout(&dialog);

    QLineEdit* lvl1Edit = new QLineEdit(genre.getNameLevel1());
    QLineEdit* lvl2Edit = new QLineEdit(genre.getNameLevel2());
    QLineEdit* lvl3Edit = new QLineEdit(genre.getNameLevel3());
    QLineEdit* lvl4Edit = new QLineEdit(genre.getNameLevel4());
    QLineEdit* lvl5Edit = new QLineEdit(genre.getNameLevel5());

    formLayout->addRow(QObject::tr("Level 1:"), lvl1Edit);
    formLayout->addRow(QObject::tr("Level 2:"), lvl2Edit);
    formLayout->addRow(QObject::tr("Level 3:"), lvl3Edit);
    formLayout->addRow(QObject::tr("Level 4:"), lvl4Edit);
    formLayout->addRow(QObject::tr("Level 5:"), lvl5Edit);

    // genres -> display_group combobox
    QVariantList genreData;
    GenreDao& genreDao = m_pTrackCollectionManager->internalCollection()->getGenreDao();
    genreDao.loadGenres2QVL(genreData);

    // convert to list of QVariantMaps for sorting
    QList<QVariantMap> genresList;
    for (const QVariant& entry : std::as_const(genreData)) {
        genresList.append(entry.toMap());
    }

    // sort genre name alphabetically
    std::sort(genresList.begin(), genresList.end(), [](const QVariantMap& a, const QVariantMap& b) {
        return a.value("name").toString().toLower() < b.value("name").toString().toLower();
    });

    QComboBox* displayGroupCombo = new QComboBox();
    displayGroupCombo->addItem(QObject::tr("(None)"), QVariant());

    // add sorted genres, skipping current genre itself and is_visible = 0
    for (const QVariantMap& map : genresList) {
        const QString name = map.value("name").toString();
        const QString id = map.value("id").toString();
        const bool isVisible = map.value("is_visible").toBool();

        // skip self and invisible genres
        if (id == genreId.toString() || !isVisible) {
            continue;
        }
        const QString displayValue = QStringLiteral("##%1##").arg(id);
        displayGroupCombo->addItem(name, displayValue);
    }

    // set current display group selection
    int currentIndex = displayGroupCombo->findData(genre.getDisplayGroup());
    if (currentIndex >= 0) {
        displayGroupCombo->setCurrentIndex(currentIndex);
    }

    formLayout->addRow(QObject::tr("Display Group:"), displayGroupCombo);

    QCheckBox* concatNameCheckbox = new QCheckBox(
            QObject::tr("Concatenate levelnames into genre name"));
    concatNameCheckbox->setChecked(false);
    formLayout->addRow(QString(), concatNameCheckbox);

    QCheckBox* visibleCheckbox = new QCheckBox(QObject::tr("Don't show genre, show grouped genre"));
    visibleCheckbox->setChecked(!genre.isVisible());
    formLayout->addRow(QString(), visibleCheckbox);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    formLayout->addWidget(buttonBox);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Genre updatedGenre = genre;
    updatedGenre.setNameLevel1(lvl1Edit->text().trimmed());
    updatedGenre.setNameLevel2(lvl2Edit->text().trimmed());
    updatedGenre.setNameLevel3(lvl3Edit->text().trimmed());
    updatedGenre.setNameLevel4(lvl4Edit->text().trimmed());
    updatedGenre.setNameLevel5(lvl5Edit->text().trimmed());

    const QString groupId = displayGroupCombo->currentData().toString().trimmed();
    updatedGenre.setDisplayGroup(groupId.isEmpty() ? QString() : groupId);

    updatedGenre.setVisible(!visibleCheckbox->isChecked());

    QStringList parts;
    for (const QString& part : {
                 updatedGenre.getNameLevel1(),
                 updatedGenre.getNameLevel2(),
                 updatedGenre.getNameLevel3(),
                 updatedGenre.getNameLevel4(),
                 updatedGenre.getNameLevel5()}) {
        if (!part.isEmpty()) {
            parts << part;
        }
    }

    if (concatNameCheckbox->isChecked()) {
        updatedGenre.setName(parts.join("//"));
    } else {
        updatedGenre.setName(genre.getName());
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
            "UPDATE genres SET name = :name, name_level_1 = :lvl1, name_level_2 = :lvl2, "
            "name_level_3 = :lvl3, name_level_4 = :lvl4, name_level_5 = :lvl5, "
            "display_group = :display_group, is_visible = :is_visible WHERE id = :id"));

    query.bindValue(":name", updatedGenre.getName());
    query.bindValue(":lvl1", updatedGenre.getNameLevel1());
    query.bindValue(":lvl2", updatedGenre.getNameLevel2());
    query.bindValue(":lvl3", updatedGenre.getNameLevel3());
    query.bindValue(":lvl4", updatedGenre.getNameLevel4());
    query.bindValue(":lvl5", updatedGenre.getNameLevel5());
    // query.bindValue(":display_group",
    //         updatedGenre.getDisplayGroup().isEmpty()
    //                 ? QVariant(QVariant::String)
    //                 : updatedGenre.getDisplayGroup());
    query.bindValue(":display_group",
            updatedGenre.getDisplayGroup().isEmpty()
                    ? QVariant(QMetaType(QMetaType::QString))
                    : QVariant(updatedGenre.getDisplayGroup()));

    query.bindValue(":is_visible", updatedGenre.isVisible());
    query.bindValue(":id", genreId.toString());

    if (!query.exec()) {
        qWarning() << "[GenreTableModel] -> Failed to update genre:" << query.lastError().text();
    } else {
        qDebug() << "[GenreTableModel] -> Updated genre ID:" << genreId.toString();

        // successful genre update -> extract the target genre ID from displayGroup (format: ##id##)
        QString targetGenreId = updatedGenre.getDisplayGroup();
        targetGenreId.remove("##");

        // update genre_tracks table: replace old genreId with targetGenreId
        QSqlQuery updateQuery(m_database);
        updateQuery.prepare(QStringLiteral(
                "UPDATE genre_tracks SET genre_id = :newGenreId WHERE genre_id = :oldGenreId"));
        updateQuery.bindValue(":newGenreId", targetGenreId);
        updateQuery.bindValue(":oldGenreId", genreId.toString());

        if (!updateQuery.exec()) {
            qWarning() << "[GenreTableModel] -> Failed to update genre_tracks:"
                       << updateQuery.lastError().text();
        } else {
            qDebug() << "[GenreTableModel] -> Updated genre_tracks from"
                     << genreId.toString() << "to" << targetGenreId;
            if (!updatedGenre.getDisplayGroup().isEmpty()) {
                QString targetGenreId = updatedGenre.getDisplayGroup();
                targetGenreId.remove("##");
                QString oldGenreId = genreId.toString();

                // update genre_tracks table
                QSqlQuery updateGenreTracksQuery(m_database);
                updateGenreTracksQuery.prepare(QStringLiteral(
                        "UPDATE genre_tracks SET genre_id = :newGenreId WHERE "
                        "genre_id = :oldGenreId"));
                updateGenreTracksQuery.bindValue(":newGenreId", targetGenreId);
                updateGenreTracksQuery.bindValue(":oldGenreId", oldGenreId);

                if (!updateGenreTracksQuery.exec()) {
                    qWarning() << "[GenreTableModel] -> Failed to update "
                                  "genre_tracks:"
                               << updateGenreTracksQuery.lastError().text();
                } else {
                    qDebug() << "[GenreTableModel] -> Updated genre_tracks from"
                             << oldGenreId << "to" << targetGenreId;
                }

                // update library.genre string
                const QString oldTag = QStringLiteral("##%1##").arg(oldGenreId);
                const QString newTag = QStringLiteral("##%1##").arg(targetGenreId);

                QSqlQuery updateLibraryQuery(m_database);
                updateLibraryQuery.prepare(QStringLiteral(
                        "UPDATE library SET genre = REPLACE(genre, :oldTag, :newTag) "
                        "WHERE genre LIKE '%' || :oldTag || '%'"));
                updateLibraryQuery.bindValue(":oldTag", oldTag);
                updateLibraryQuery.bindValue(":newTag", newTag);

                if (!updateLibraryQuery.exec()) {
                    qWarning() << "[GenreTableModel] -> Failed to update "
                                  "library.genre:"
                               << updateLibraryQuery.lastError().text();
                } else {
                    qDebug()
                            << "[GenreTableModel] -> Replaced genre tag"
                            << oldTag << "with" << newTag << "in library table";
                }
            }
        }
    }
}

void GenreTableModel::setAllGenresVisible() {
    QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
            QObject::tr("Confirm Visibility Change"),
            QObject::tr("This will make all genres visible again. Are you sure? \n "
                        "Do you want to continue?"),
            QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        qDebug() << "[GenreTableModel] -> allGenresVisible -> Action cancelled by user.";
        return;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("UPDATE genres SET is_visible = 1"));

    if (!query.exec()) {
        qWarning() << "[GenreTableModel] -> Failed to set all genres visible:"
                   << query.lastError().text();
    } else {
        qDebug() << "[GenreTableModel] -> Set is_visible = 1 for all genres.";
    }
}

void GenreTableModel::setGenreInvisible(const GenreId& genreId) {
    qDebug() << "[GenreTableModel] -> setGenreInvisible -> genreId:" << genreId;

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("UPDATE genres SET is_visible = 0 WHERE id = :id"));
    query.bindValue(":id", genreId.toString());

    if (!query.exec()) {
        qWarning() << "[GenreTableModel] -> Failed to set genre invisible:"
                   << query.lastError().text();
    } else {
        qDebug() << "[GenreTableModel] -> Set is_visible = 0 for genreId:" << genreId.toString();
    }
}

void GenreTableModel::EditGenresMulti() {
    qDebug() << "[GenreTableModel] -> EditGenresMulti";

    QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
            QObject::tr("Multiple Genres Edit"),
            QObject::tr("Building the table can take some time (minutes) if "
                        "you have defined many genres."
                        "eg if you imported a huge external model) \n"
                        "Are you sure you want to continue?"),
            QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        qDebug() << "[GenreTableModel] -> EditGenresMulti -> Action cancelled by user.";
        return;
    }

    QVariantList genreData;
    GenreDao& genreDao = m_pTrackCollectionManager->internalCollection()->getGenreDao();
    genreDao.loadGenres2QVL(genreData);

    QList<QVariantMap> genres;
    for (const QVariant& v : std::as_const(genreData)) {
        genres << v.toMap();
    }

    // sort alphabetically genres
    std::sort(genres.begin(), genres.end(), [](const QVariantMap& a, const QVariantMap& b) {
        return a.value("name").toString().toLower() < b.value("name").toString().toLower();
    });

    QDialog dialog;
    dialog.setWindowTitle(QObject::tr("Edit Multiple Genres"));
    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // filter combo box on visibility
    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* filterLabel = new QLabel(QObject::tr("Filter by visibility:"));
    QComboBox* filterCombo = new QComboBox();
    filterCombo->addItem(QObject::tr("Visible genres only"), 1);
    filterCombo->addItem(QObject::tr("Invisible genres only"), 0);
    filterCombo->addItem(QObject::tr("All genres"), 2);
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(filterCombo);
    layout->addLayout(filterLayout);

    QHBoxLayout* userDefinedLayout = new QHBoxLayout();
    QLabel* userDefinedLabel = new QLabel(QObject::tr("Filter by User-Defined / Model Imported:"));
    QComboBox* userDefinedCombo = new QComboBox();
    userDefinedCombo->addItem("All genres", QVariant(-1));
    userDefinedCombo->addItem("Only User-Defined genres", QVariant(0));
    userDefinedCombo->addItem("Only Model-Defined genres", QVariant(1));
    userDefinedLayout->addWidget(userDefinedLabel);
    userDefinedLayout->addWidget(userDefinedCombo);
    layout->addLayout(userDefinedLayout);

    // table  with max possible rows
    QTableWidget* table = new QTableWidget();
    table->setColumnCount(9);
    table->setHorizontalHeaderLabels(QStringList()
            << "Genre" << "Name_Level_1"
            << "Name_Level_2" << "Name_Level_3" << "Name_Level_4"
            << "Name_Level_5"
            << "Model Def" << "Visible" << "Display Group");
    table->setColumnWidth(0, 150);
    table->setColumnWidth(1, 100);
    table->setColumnWidth(2, 100);
    table->setColumnWidth(3, 100);
    table->setColumnWidth(4, 100);
    table->setColumnWidth(5, 100);
    table->setColumnWidth(6, 60);
    table->setColumnWidth(7, 60);
    table->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);
    for (int col = 0; col < table->columnCount(); ++col) {
        QTableWidgetItem* headerItem = table->horizontalHeaderItem(col);
        if (headerItem) {
            headerItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }
    layout->addWidget(table);

    // pre-sort visible genres for combo box
    QList<QPair<QString, QString>> visibleGenreList;
    for (const QVariantMap& map : std::as_const(genres)) {
        if (map.value("is_visible").toBool()) {
            visibleGenreList << qMakePair(map.value("name").toString(), map.value("id").toString());
        }
    }
    std::sort(visibleGenreList.begin(), visibleGenreList.end(), [](const auto& a, const auto& b) {
        return a.first.toLower() < b.first.toLower();
    });

    // populate table based on filter
    auto populateTable = [&](int visibilityFilter, int userDefinedFilter) {
        // Clear previous rows
        table->setRowCount(0);

        for (const QVariantMap& genre : std::as_const(genres)) {
            bool isVisible = genre.value("is_visible").toBool();
            bool isUserDefined = genre.value("is_model_defined").toBool();

            // apply visibility filter
            if ((visibilityFilter == 1 && !isVisible) ||
                    (visibilityFilter == 0 && isVisible)) {
                continue;
            }

            // apply user-defined filter
            if ((userDefinedFilter == 1 && !isUserDefined) ||
                    (userDefinedFilter == 0 && isUserDefined)) {
                continue;
            }

            int row = table->rowCount();
            table->insertRow(row);

            const QString genreId = genre.value("id").toString();
            const QString name = genre.value("name").toString();
            const QString name_level_1 = genre.value("name_level_1").toString();
            const QString name_level_2 = genre.value("name_level_2").toString();
            const QString name_level_3 = genre.value("name_level_3").toString();
            const QString name_level_4 = genre.value("name_level_4").toString();
            const QString name_level_5 = genre.value("name_level_5").toString();
            const QString displayGroup = genre.value("display_group").toString();

            // name (non-editable)
            QTableWidgetItem* nameItem = new QTableWidgetItem(name);
            nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
            table->setItem(row, 0, nameItem);

            // name_level_1_Item (non-editable)
            QTableWidgetItem* name_level_1_Item = new QTableWidgetItem(name_level_1);
            name_level_1_Item->setFlags(name_level_1_Item->flags() & ~Qt::ItemIsEditable);
            table->setItem(row, 1, name_level_1_Item);

            // name_level_2_Item (non-editable)
            QTableWidgetItem* name_level_2_Item = new QTableWidgetItem(name_level_2);
            name_level_2_Item->setFlags(name_level_2_Item->flags() & ~Qt::ItemIsEditable);
            table->setItem(row, 2, name_level_2_Item);

            // name_level_3_Item (non-editable)
            QTableWidgetItem* name_level_3_Item = new QTableWidgetItem(name_level_3);
            name_level_3_Item->setFlags(name_level_3_Item->flags() & ~Qt::ItemIsEditable);
            table->setItem(row, 3, name_level_3_Item);

            // name_level_4_Item (non-editable)
            QTableWidgetItem* name_level_4_Item = new QTableWidgetItem(name_level_4);
            name_level_4_Item->setFlags(name_level_4_Item->flags() & ~Qt::ItemIsEditable);
            table->setItem(row, 4, name_level_4_Item);

            // name_level_5_Item (non-editable)
            QTableWidgetItem* name_level_5_Item = new QTableWidgetItem(name_level_5);
            name_level_5_Item->setFlags(name_level_5_Item->flags() & ~Qt::ItemIsEditable);
            table->setItem(row, 5, name_level_5_Item);

            // is_model_defined checkbox (non-editable)
            QCheckBox* userDefinedCheckbox = new QCheckBox();
            userDefinedCheckbox->setChecked(isUserDefined);
            userDefinedCheckbox->setEnabled(false); // read-only

            QWidget* checkboxContainer = new QWidget();
            QHBoxLayout* layout = new QHBoxLayout(checkboxContainer);
            layout->addWidget(userDefinedCheckbox);
            layout->setAlignment(Qt::AlignCenter);
            layout->setContentsMargins(0, 0, 0, 0);
            table->setCellWidget(row, 6, checkboxContainer);

            // is_visible checkbox (editable)
            QCheckBox* visibleCheckbox = new QCheckBox();
            visibleCheckbox->setChecked(isVisible);

            QWidget* visibleContainer = new QWidget();
            QHBoxLayout* visibleLayout = new QHBoxLayout(visibleContainer);
            visibleLayout->addWidget(visibleCheckbox);
            visibleLayout->setAlignment(Qt::AlignCenter);
            visibleLayout->setContentsMargins(0, 0, 0, 0);
            table->setCellWidget(row, 7, visibleContainer);

            // display_group combo box (editable)
            QComboBox* combo = new QComboBox();
            combo->addItem(QObject::tr("(None)"), QString());

            for (const auto& pair : std::as_const(visibleGenreList)) {
                if (pair.second == genreId)
                    continue;
                QString tag = QString("##%1##").arg(pair.second);
                combo->addItem(pair.first, tag);
            }

            int currentIndex = combo->findData(displayGroup);
            if (currentIndex >= 0) {
                combo->setCurrentIndex(currentIndex);
            }
            table->setCellWidget(row, 8, combo);
        }
    };

    // populate genres
    populateTable(1, -1);

    QObject::connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {
        int visibilityFilter = filterCombo->currentData().toInt();
        int userDefinedFilter = userDefinedCombo->currentData().toInt();
        populateTable(visibilityFilter, userDefinedFilter);
    });

    QObject::connect(userDefinedCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [=]() {
                int visibilityFilter = filterCombo->currentData().toInt();
                int userDefinedFilter = userDefinedCombo->currentData().toInt();
                populateTable(visibilityFilter, userDefinedFilter);
            });

    // dialog buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton* applyButton = new QPushButton(QObject::tr("Apply"));
    buttonBox->addButton(applyButton, QDialogButtonBox::ApplyRole);
    layout->addWidget(buttonBox);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // apply routine
    auto applyChanges = [&]() {
        for (int row = 0; row < table->rowCount(); ++row) {
            QTableWidgetItem* nameItem = table->item(row, 0);
            QString genreName = nameItem->text();

            auto it = std::find_if(genres.begin(), genres.end(), [&](const QVariantMap& g) {
                return g.value("name").toString() == genreName;
            });
            if (it == genres.end()) {
                continue;
            }
            QVariantMap genre = *it;
            QString genreId = genre.value("id").toString();
            QString oldDisplayGroup = genre.value("display_group").toString();
            bool oldVisible = genre.value("is_visible").toBool();

            QCheckBox* visibleCheckbox = table->cellWidget(row, 7)->findChild<QCheckBox*>();
            bool newVisible = visibleCheckbox && visibleCheckbox->isChecked();

            QComboBox* combo = qobject_cast<QComboBox*>(table->cellWidget(row, 8));
            QString newDisplayGroup = combo ? combo->currentData().toString() : QString();

            if (oldVisible == newVisible && oldDisplayGroup == newDisplayGroup) {
                continue;
            }

            QSqlQuery updateQuery(m_database);
            updateQuery.prepare(QStringLiteral(
                    "UPDATE genres SET is_visible = :visible, display_group = "
                    ":displayGroup WHERE id = :id"));
            updateQuery.bindValue(":visible", newVisible);
            updateQuery.bindValue(":displayGroup",
                    newDisplayGroup.isEmpty() ? QVariant() : newDisplayGroup);
            updateQuery.bindValue(":id", genreId);

            if (!updateQuery.exec()) {
                qWarning() << "[GenreTableModel] -> EditGenresMulti -> Failed "
                              "to update genre"
                           << genreId << ":" << updateQuery.lastError().text();
                continue;
            }

            if (oldDisplayGroup != newDisplayGroup) {
                QString oldTag = QString("##%1##").arg(genreId);
                QString newTag = newDisplayGroup;

                if (!newTag.isEmpty()) {
                    QString newGenreId = newTag;
                    newGenreId.remove("##");

                    QSqlQuery updateGT(m_database);
                    updateGT.prepare(QStringLiteral(
                            "UPDATE genre_tracks SET genre_id = :newGenreId "
                            "WHERE genre_id = :oldGenreId"));
                    updateGT.bindValue(":newGenreId", newGenreId);
                    updateGT.bindValue(":oldGenreId", genreId);
                    if (!updateGT.exec()) {
                        qWarning() << "[GenreTableModel] -> Failed to update "
                                      "genre_tracks for genre"
                                   << genreId;
                    }

                    QSqlQuery updateLib(m_database);
                    updateLib.prepare(QStringLiteral(
                            "UPDATE library SET genre = REPLACE(genre, :oldTag, :newTag) "
                            "WHERE genre LIKE '%' || :oldTag || '%'"));
                    updateLib.bindValue(":oldTag", oldTag);
                    updateLib.bindValue(":newTag", newTag);
                    if (!updateLib.exec()) {
                        qWarning() << "[GenreTableModel] -> Failed to update "
                                      "library.genre for genre"
                                   << genreId;
                    }
                }
            }

            qDebug() << "[GenreTableModel] -> EditGenresMulti -> Updated genre" << genreId;
        }

        // refresh table
        genreData.clear();
        genreDao.loadGenres2QVL(genreData);
        genres.clear();
        for (const QVariant& v : std::as_const(genreData)) {
            genres << v.toMap();
        }
        std::sort(genres.begin(), genres.end(), [](const QVariantMap& a, const QVariantMap& b) {
            return a.value("name").toString().toLower() < b.value("name").toString().toLower();
        });
        visibleGenreList.clear();
        for (const QVariantMap& map : std::as_const(genres)) {
            if (map.value("is_visible").toBool()) {
                visibleGenreList << qMakePair(map.value("name").toString(),
                        map.value("id").toString());
            }
        }
        std::sort(visibleGenreList.begin(),
                visibleGenreList.end(),
                [](const auto& a, const auto& b) {
                    return a.first.toLower() < b.first.toLower();
                });

        int visibilityFilter = filterCombo->currentData().toInt();
        int userDefinedFilter = userDefinedCombo->currentData().toInt();
        populateTable(visibilityFilter, userDefinedFilter);
    };

    QObject::connect(applyButton, &QPushButton::clicked, applyChanges);
    dialog.resize(1000, 600);
    table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    qDebug() << "[GenreTableModel] -> EditGenresMulti -> Done";
    applyChanges();
}

void GenreTableModel::EditOrphanTrackGenres() {
    qDebug() << "[GenreTableModel] -> EditOrphanTrackGenres";
    QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
            QObject::tr("EditOrphanTrackGenres"),
            QObject::tr("Building the table can take some time (minutes) if "
                        "you have a lot of orphaned genres in your tracks. "
                        "This usually happens the first time you run this function.\n\n"
                        "Do you want to continue?"),
            QMessageBox::Ok | QMessageBox::Cancel);

    if (reply != QMessageBox::Ok) {
        qDebug() << "[GenreTableModel] -> EditOrphanTrackGenres -> Action cancelled by user.";
        return;
    }

    //    QVariantList genreData;
    QList<QVariantMap> genres;
    //    QVariantMap newEntry;
    static const QRegularExpression tagRegex("##(\\d+)##");
    static const QRegularExpression nonAlphaNum("[^a-z0-9]");

    QDialog dialog;
    dialog.setWindowTitle(QObject::tr("Edit Orphan Genres"));
    dialog.resize(800, 500);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QTableWidget* table = new QTableWidget(&dialog);

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({QObject::tr("Found Orphan Genre"),
            QObject::tr("Create new Genre"),
            QObject::tr("Link other Genre"),
            QObject::tr("Linked Genre")});
    table->setColumnWidth(0, 250);
    table->setColumnWidth(1, 125);
    table->setColumnWidth(2, 125);
    table->setColumnWidth(3, 250);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    for (int col = 0; col < table->columnCount(); ++col) {
        QTableWidgetItem* headerItem = table->horizontalHeaderItem(col);
        if (headerItem) {
            headerItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(table);

    auto populateTable = [&]() {
        m_orphanToTrackIds.clear();
        table->clearContents();
        table->setRowCount(0);

        // find orphans drom genre fields
        QSqlQuery query(m_database);
        QSet<QString> orphanGenreStrings;

        if (!query.prepare("SELECT id, genre FROM library WHERE genre IS NOT "
                           "NULL AND genre != ''") ||
                !query.exec()) {
            qWarning() << "[GenreTableModel] Genre query error:" << query.lastError();
            return;
        }

        while (query.next()) {
            TrackId trackId(query.value(0));
            QString genreField = query.value(1).toString();

            QSet<QString> tags;
            auto it = tagRegex.globalMatch(genreField);
            while (it.hasNext())
                tags.insert(it.next().captured(0));
            for (const QString& tag : tags)
                genreField.replace(tag, " ");

            // adapted to add the long strings in the orphan editor too
            QStringList semicolonParts = genreField.split(';', Qt::SkipEmptyParts);
            for (QString part : std::as_const(semicolonParts)) {
                part = part.trimmed();
                if (part.isEmpty())
                    continue;

                // 1st:  full part like "classica - opera"
                orphanGenreStrings.insert(part);
                m_orphanToTrackIds[part].append(trackId.toVariant().toInt());

                // now we don't see short strings or meaningful words as possible orphans
                if (part.contains(' ') && !part.contains(';')) {
                    QStringList spaceParts = part.split(' ', Qt::SkipEmptyParts);
                    for (const QString& wordRef : std::as_const(spaceParts)) {
                        QString word = wordRef.trimmed();

                        // short meaningless words or symbols like "'n" in "rock 'n roll"
                        if (word.isEmpty() || word.length() <= 2) {
                            continue;
                        }

                        // list of words in genres that have no meaning and
                        // don't need to be shown as orphans
                        static const QSet<QString> ignoredWords = {
                                "n", "'n", "and", "&", "feat", "ft", "vs", "vs.", "with"};
                        QString cleaned = word.toLower().remove(nonAlphaNum);
                        if (ignoredWords.contains(cleaned)) {
                            continue;
                        }

                        orphanGenreStrings.insert(word);
                        m_orphanToTrackIds[word].append(trackId.toVariant().toInt());
                    }
                }
            }
        }
        qDebug() << "[GenreTableModel] -> EditOrphanTrackGenres -> Populate: "
                    "orphanToTrackIds: "
                 << m_orphanToTrackIds;

        // load is_visible genres for link-combobox
        genres.clear();
        QVariantList genreData;
        GenreDao& genreDao = m_pTrackCollectionManager->internalCollection()->getGenreDao();
        genreDao.loadGenres2QVL(genreData);

        for (const QVariant& v : std::as_const(genreData)) {
            QVariantMap map = v.toMap();
            if (map["is_visible"].toBool()) {
                genres << map;
            }
        }

        std::sort(genres.begin(), genres.end(), [](const QVariantMap& a, const QVariantMap& b) {
            return a["name"].toString().compare(b["name"].toString(), Qt::CaseInsensitive) < 0;
        });

        // construction of the table
        QStringList sortedOrphans = orphanGenreStrings.values();
        std::sort(sortedOrphans.begin(),
                sortedOrphans.end(),
                [](const QString& a, const QString& b) {
                    return a.compare(b, Qt::CaseInsensitive) < 0;
                });

        for (const QString& orphan : std::as_const(sortedOrphans)) {
            int newRow = table->rowCount();
            table->insertRow(newRow);

            QTableWidgetItem* orphanItem = new QTableWidgetItem(orphan);
            orphanItem->setFlags(Qt::ItemIsEnabled);
            table->setItem(newRow, 0, orphanItem);

            // column 1 checkbox 'create new genre'
            QWidget* createCheckboxContainer = new QWidget(table);
            QCheckBox* createCheckbox = new QCheckBox(createCheckboxContainer);
            createCheckbox->setChecked(false);

            // disable if genre already exists
            bool alreadyExists = std::any_of(genres.begin(),
                    genres.end(),
                    [&](const QVariantMap& genre) {
                        return genre["name"].toString().compare(
                                       orphan, Qt::CaseInsensitive) == 0;
                    });
            createCheckbox->setEnabled(!alreadyExists);

            QHBoxLayout* createLayout = new QHBoxLayout(createCheckboxContainer);
            createLayout->addWidget(createCheckbox);
            createLayout->setAlignment(Qt::AlignCenter);
            createLayout->setContentsMargins(0, 0, 0, 0);
            createCheckboxContainer->setLayout(createLayout);
            table->setCellWidget(newRow, 1, createCheckboxContainer);

            // column 2 checkbox 'link to existing genre'
            QWidget* linkCheckboxContainer = new QWidget(table);
            QCheckBox* linkCheckbox = new QCheckBox(linkCheckboxContainer);
            linkCheckbox->setChecked(false);
            QHBoxLayout* linkLayout = new QHBoxLayout(linkCheckboxContainer);
            linkLayout->addWidget(linkCheckbox);
            linkLayout->setAlignment(Qt::AlignCenter);
            linkLayout->setContentsMargins(0, 0, 0, 0);
            linkCheckboxContainer->setLayout(linkLayout);
            table->setCellWidget(newRow, 2, linkCheckboxContainer);

            // column 3 combox 'linked genre'
            QComboBox* combo = new QComboBox(table);
            combo->setEnabled(false);
            combo->setMinimumWidth(200);
            combo->addItem("", -1);

            for (const QVariantMap& genre : std::as_const(genres)) {
                combo->addItem(genre["name"].toString(), genre["id"]);
            }
            table->setCellWidget(newRow, 3, combo);

            QObject::connect(linkCheckbox, &QCheckBox::toggled, this, [combo](bool checked) {
                combo->setEnabled(checked);
            });
        }
    };

    auto applyChanges = [&]() {
        GenreDao& genreDao = m_pTrackCollectionManager->internalCollection()->getGenreDao();
        qDebug() << "[GenreTableModel] -> EditOrphanTrackGenres -> Apply: "
                    "orphanToTrackIds:"
                 << m_orphanToTrackIds;

        for (int row = 0; row < table->rowCount(); ++row) {
            const QString orphanStr = table->item(row, 0)->text().trimmed();

            QWidget* createWidget = table->cellWidget(row, 1);
            QCheckBox* createCheckbox = createWidget
                    ? createWidget->findChild<QCheckBox*>()
                    : nullptr;

            QWidget* linkWidget = table->cellWidget(row, 2);
            QCheckBox* linkCheckbox = linkWidget ? linkWidget->findChild<QCheckBox*>() : nullptr;

            bool createNewGenre = createCheckbox && createCheckbox->isChecked();
            bool linkOtherGenre = linkCheckbox && linkCheckbox->isChecked();

            if (!createNewGenre && !linkOtherGenre) {
                continue;
            }

            GenreId genreIdToUse;

            if (createNewGenre) {
                Genre newGenre;
                newGenre.setName(orphanStr);
                newGenre.setVisible(true);
                newGenre.setModelDefined(false);
                newGenre.setDisplayOrder(0);
                newGenre.setCount(0);
                newGenre.setShow(0);

                GenreId newId;
                if (genreDao.insertGenre(newGenre, &newId)) {
                    genreIdToUse = newId;
                    qDebug() << "[GenreTableModel] -> EditOrphanTrackGenres -> "
                                "Apply: Created new genre:"
                             << newId << "for string:" << orphanStr;
                } else {
                    qWarning() << "[GenreTableModel] -> EditOrphanTrackGenres "
                                  "-> Apply: Failed to insert new genre for:"
                               << orphanStr;
                    continue;
                }
            } else if (linkOtherGenre) {
                QWidget* widget = table->cellWidget(row, 3);
                QComboBox* combo = qobject_cast<QComboBox*>(widget);
                if (!combo) {
                    qWarning() << "[GenreTableModel] -> EditOrphanTrackGenres "
                                  "-> Apply: Missing combo box for linked "
                                  "genre at row"
                               << row;
                    continue;
                }
                QVariant data = combo->currentData();
                if (!data.isValid() || data.toInt() < 0) {
                    continue;
                }
                genreIdToUse = GenreId(data);
            }

            if (!genreIdToUse.isValid()) {
                continue;
            }

            const QString tag = QString("##%1##").arg(genreIdToUse.toVariant().toLongLong());
            const QList<int> trackIdList = m_orphanToTrackIds.value(orphanStr);
            qDebug() << "[GenreTableModel] -> EditOrphanTrackGenres -> Apply:  "
                        "Updating"
                     << trackIdList.size() << "tracks with orphan:" << orphanStr
                     << "-> tag:" << tag << "trackIdList" << trackIdList;

            const QList<int> trackIds = m_orphanToTrackIds.value(orphanStr);
            for (int id : trackIds) {
                qDebug() << "[GenreTableModel] -> EditOrphanTrackGenres -> Apply: trackId" << id;

                QString selectQueryString =
                        QStringLiteral("SELECT genre FROM %1 WHERE id = %2")
                                .arg(LIBRARY_TABLE)
                                .arg(id);
                qDebug() << "[GenreTableModel] -> EditOrphanTrackGenres -> "
                            "Apply: selectQueryString"
                         << selectQueryString;
                FwdSqlQuery selectQuery(m_database, selectQueryString);
                QString genreString;
                if (selectQuery.execPrepared() && selectQuery.next()) {
                    genreString = selectQuery.fieldValue(0).toString();
                } else {
                    qWarning() << "[GenreTableModel] -> EditOrphanTrackGenres "
                                  "-> Apply: Failed to load genre for track"
                               << id;
                    continue;
                }

                QStringList parts = genreString.split(';', Qt::SkipEmptyParts);
                bool changed = false;

                for (int i = 0; i < parts.size(); ++i) {
                    QString trimmed = parts[i].trimmed();
                    qDebug() << "[GenreTableModel] -> EditOrphanTrackGenres -> "
                                "Apply: Matching part:"
                             << trimmed << "against orphanStr:" << orphanStr;
                    if (trimmed == orphanStr) {
                        parts[i] = tag;
                        changed = true;
                    }
                }

                if (changed) {
                    QString newGenre = parts.join("; ");
                    qDebug() << "[GenreTableModel] -> EditOrphanTrackGenres -> "
                                "Apply: Updating track"
                             << id << ":"
                             << "Old genre:" << genreString
                             << "New genre:" << newGenre;
                    QString updateQueryString = QString("UPDATE %1 SET genre = '%2' WHERE id = %3")
                                                        .arg(LIBRARY_TABLE,
                                                                newGenre.replace("'", "''"),
                                                                QString::number(id));
                    qDebug() << "[GenreTableModel] -> EditOrphanTrackGenres -> "
                                "Apply: updateQueryString"
                             << updateQueryString;
                    FwdSqlQuery updateQuery(m_database, updateQueryString);

                    if (!updateQuery.execPrepared()) {
                        qWarning() << "[GenreTableModel] -> "
                                      "EditOrphanTrackGenres -> Apply: Failed "
                                      "to update genre field for track"
                                   << id << ":" << updateQuery.lastError();
                    }
                }

                // insert into genre_tracks
                QSqlQuery insertQuery(m_database);
                insertQuery.prepare(
                        "INSERT OR IGNORE INTO genre_tracks (genre_id, "
                        "track_id) VALUES (:genreId, :trackId)");
                insertQuery.bindValue(":genreId", genreIdToUse.toVariant());
                insertQuery.bindValue(":trackId", id);

                if (!insertQuery.exec()) {
                    qWarning() << "[GenreTableModel] -> EditOrphanTrackGenres "
                                  "-> Apply: Failed to insert genre_tracks "
                                  "entry for track"
                               << id << ":" << insertQuery.lastError();
                }
            }
        }
        populateTable();
    };

    populateTable();
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton* applyButton = new QPushButton(QObject::tr("Apply"));
    buttonBox->addButton(applyButton, QDialogButtonBox::ApplyRole);
    layout->addWidget(buttonBox);

    QObject::connect(applyButton, &QPushButton::clicked, applyChanges);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void GenreTableModel::exportGenresToCsv() {
    const QString defaultFilename = QDate::currentDate().toString("yyyyMMdd") + " - Genres.csv";
    const QString fileName = QFileDialog::getSaveFileName(
            nullptr,
            tr("Export Genres to CSV"),
            QDir::homePath() + "/" + defaultFilename,
            tr("CSV Files (*.csv)"));

    if (fileName.isEmpty()) {
        qDebug() << "[GenreTableModel] Export cancelled by user";
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "[GenreTableModel] Could not open file for writing:" << file.errorString();
        return;
    }

    QTextStream out(&file);
    QStringList row;

    // headers
    out << "id,name,name_level_1,name_level_2,name_level_3,"
           "name_level_4,name_level_5,display_group,display_order,"
           "is_visible,is_model_defined,count,show,locked,autodj_source\n";

    QSqlQuery query(m_database);
    if (!query.exec("SELECT id, name, name_level_1, name_level_2, name_level_3, "
                    "name_level_4, name_level_5, display_group, display_order, "
                    "is_visible, is_model_defined, count, show, locked, autodj_source "
                    "FROM genres")) {
        qWarning() << "[GenreTableModel] Failed to query genres table:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        row.clear();
        for (int i = 0; i < query.record().count(); ++i) {
            QString value = query.value(i).toString().replace('"', "\"\"");
            if (value.contains(',') || value.contains('"') || value.contains('\n')) {
                value = "\"" + value + "\"";
            }
            row << value;
        }
        out << row.join(",") << "\n";
    }

    file.close();
    qDebug() << "[GenreTableModel] Exported genres to:" << fileName;

    QMessageBox::information(nullptr,
            tr("Export Complete"),
            tr("Genres have been successfully exported to:\n%1").arg(fileName));
}

void GenreTableModel::importGenresFromCsv() {
    QString filePath = QFileDialog::getOpenFileName(
            nullptr,
            tr("Import Genres from CSV"),
            QDir::homePath(),
            tr("CSV Files (*.csv)"));

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, tr("Import Genres"), tr("Failed to open the selected file."));
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QStringList lines;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            lines.append(line);
        }
    }
    file.close();

    if (lines.isEmpty()) {
        QMessageBox::information(nullptr,
                tr("Import Genres from CSV"),
                tr("The CSV file is empty."));
        return;
    }

    // Remove header if present
    if (lines.first().contains("name", Qt::CaseInsensitive)) {
        lines.removeFirst();
    }

    const int totalRecords = lines.size();

    QSet<QString> existingNames;
    QSqlQuery selectQuery(m_database);
    if (!selectQuery.exec(QStringLiteral("SELECT name FROM genres"))) {
        qWarning() << "[GenreTableModel] -> importGenresFromCsv -> Failed to "
                      "query existing genres:"
                   << selectQuery.lastError().text();
        return;
    }
    while (selectQuery.next()) {
        existingNames.insert(selectQuery.value(0).toString().trimmed().toLower());
    }

    QList<QStringList> rowsToInsert;
    for (const QString& line : lines) {
        const QStringList parts = line.split(",", Qt::KeepEmptyParts);
        if (parts.isEmpty()) {
            continue;
        }
        const QString name = parts.value(1).trimmed(); // Skip id
        if (name.isEmpty()) {
            continue;
        }
        if (!existingNames.contains(name.toLower())) {
            rowsToInsert.append(parts);
        }
    }

    const int newRecords = rowsToInsert.size();

    if (newRecords == 0) {
        QMessageBox::information(nullptr,
                tr("Import Genres from CSV"),
                tr("No new genres found to import."));
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
            tr("Import Genres from CSV"),
            tr("CSV contains %1 genres.\n%2 of them are new and will be "
               "added.\n\nProceed with import?")
                    .arg(totalRecords)
                    .arg(newRecords),
            QMessageBox::Yes | QMessageBox::Cancel);

    if (reply != QMessageBox::Yes) {
        return;
    }

    QSqlQuery insertQuery(m_database);
    insertQuery.prepare(QStringLiteral(
            "INSERT INTO genres (name, name_level_1, name_level_2, "
            "name_level_3, name_level_4, name_level_5, display_group, "
            "display_order, is_visible, is_model_defined, count, "
            "show, locked, autodj_source) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

    int insertedCount = 0;
    for (const QStringList& parts : rowsToInsert) {
        // Skip id column (index 0)
        for (int i = 1; i < 15; ++i) {
            insertQuery.bindValue(i - 1, parts.value(i).trimmed());
        }
        if (!insertQuery.exec()) {
            qWarning() << "[GenreTableModel] -> importGenresFromCsv -> Failed "
                          "to insert genre row:"
                       << insertQuery.lastError().text();
        } else {
            ++insertedCount;
        }
    }

    QMessageBox::information(
            nullptr,
            tr("Import Complete"),
            tr("%1 new genres successfully imported.").arg(insertedCount));
}

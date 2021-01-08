#include "library/columncache.h"

#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "moc_columncache.cpp"
#include "util/db/dbconnection.h"

namespace {

const QString kSortInt = QStringLiteral("cast(%1 as integer)");
const QString kSortNoCase = QStringLiteral("lower(%1)");
const QString kSortNoCaseLex = mixxx::DbConnection::collateLexicographically(
        QStringLiteral("lower(%1)"));

} // namespace

ColumnCache::ColumnCache(const QStringList& columns) {
    m_pKeyNotationCP = new ControlProxy("[Library]", "key_notation", this);
    m_pKeyNotationCP->connectValueChanged(this, &ColumnCache::slotSetKeySortOrder);

    // ColumnCache is initialized before the preferences, so slotSetKeySortOrder is called
    // for again if DlgPrefKey sets the [Library]. key_notation CO to a value other than
    // KeyUtils::CUSTOM as Mixxx is starting.

    setColumns(columns);
}

void ColumnCache::setColumns(const QStringList& columns) {
    m_columnsByIndex.clear();
    m_columnsByIndex.append(columns);

    m_columnIndexByName.clear();
    for (int i = 0; i < columns.size(); ++i) {
        QString column = columns[i];
        DEBUG_ASSERT(!m_columnIndexByName.contains(column));
        m_columnIndexByName[column] = i;
    }

    m_columnNameByEnum.clear();
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_ID, LIBRARYTABLE_ID);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_ARTIST, LIBRARYTABLE_ARTIST);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_TITLE, LIBRARYTABLE_TITLE);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_ALBUM, LIBRARYTABLE_ALBUM);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_ALBUMARTIST, LIBRARYTABLE_ALBUMARTIST);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_YEAR, LIBRARYTABLE_YEAR);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_GENRE, LIBRARYTABLE_GENRE);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_COMPOSER, LIBRARYTABLE_COMPOSER);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_GROUPING, LIBRARYTABLE_GROUPING);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_TRACKNUMBER, LIBRARYTABLE_TRACKNUMBER);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_FILETYPE, LIBRARYTABLE_FILETYPE);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_COMMENT, LIBRARYTABLE_COMMENT);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_DURATION, LIBRARYTABLE_DURATION);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_BITRATE, LIBRARYTABLE_BITRATE);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_BPM, LIBRARYTABLE_BPM);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_REPLAYGAIN, LIBRARYTABLE_REPLAYGAIN);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_CUEPOINT, LIBRARYTABLE_CUEPOINT);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_URL, LIBRARYTABLE_URL);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_SAMPLERATE, LIBRARYTABLE_SAMPLERATE);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_WAVESUMMARYHEX, LIBRARYTABLE_WAVESUMMARYHEX);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_CHANNELS, LIBRARYTABLE_CHANNELS);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_MIXXXDELETED, LIBRARYTABLE_MIXXXDELETED);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_DATETIMEADDED, LIBRARYTABLE_DATETIMEADDED);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_HEADERPARSED, LIBRARYTABLE_HEADERPARSED);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_TIMESPLAYED, LIBRARYTABLE_TIMESPLAYED);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_LAST_PLAYED_AT, LIBRARYTABLE_LAST_PLAYED_AT);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_PLAYED, LIBRARYTABLE_PLAYED);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_RATING, LIBRARYTABLE_RATING);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_KEY, LIBRARYTABLE_KEY);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_KEY_ID, LIBRARYTABLE_KEY_ID);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_BPM_LOCK, LIBRARYTABLE_BPM_LOCK);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_PREVIEW, LIBRARYTABLE_PREVIEW);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_COLOR, LIBRARYTABLE_COLOR);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_COVERART, LIBRARYTABLE_COVERART);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_COVERART_SOURCE, LIBRARYTABLE_COVERART_SOURCE);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_COVERART_TYPE, LIBRARYTABLE_COVERART_TYPE);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_COVERART_LOCATION, LIBRARYTABLE_COVERART_LOCATION);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_COVERART_DIGEST, LIBRARYTABLE_COVERART_DIGEST);
    insertColumnNameByEnum(COLUMN_LIBRARYTABLE_COVERART_HASH, LIBRARYTABLE_COVERART_HASH);

    insertColumnNameByEnum(COLUMN_TRACKLOCATIONSTABLE_LOCATION, TRACKLOCATIONSTABLE_LOCATION);
    insertColumnNameByEnum(COLUMN_TRACKLOCATIONSTABLE_FSDELETED, TRACKLOCATIONSTABLE_FSDELETED);

    insertColumnNameByEnum(COLUMN_PLAYLISTTRACKSTABLE_TRACKID, PLAYLISTTRACKSTABLE_TRACKID);
    insertColumnNameByEnum(COLUMN_PLAYLISTTRACKSTABLE_POSITION, PLAYLISTTRACKSTABLE_POSITION);
    insertColumnNameByEnum(COLUMN_PLAYLISTTRACKSTABLE_PLAYLISTID, PLAYLISTTRACKSTABLE_PLAYLISTID);

    insertColumnNameByEnum(
            COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED,
            PLAYLISTTRACKSTABLE_DATETIMEADDED);

    insertColumnNameByEnum(COLUMN_REKORDBOX_ANALYZE_PATH, REKORDBOX_ANALYZE_PATH);

    for (int i = 0; i < NUM_COLUMNS; ++i) {
        m_columnIndexByEnum[i] = -1;
    }
    for (auto it = m_columnNameByEnum.constBegin(); it != m_columnNameByEnum.constEnd(); ++it) {
        DEBUG_ASSERT(it.key() >= 0);
        DEBUG_ASSERT(it.key() < NUM_COLUMNS);
        DEBUG_ASSERT(m_columnIndexByEnum[it.key()] == -1);
        m_columnIndexByEnum[it.key()] = fieldIndex(it.value());
    }

    m_columnSortByIndex.clear();
    // Add the columns that requires a special sort
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_ARTIST, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_TITLE, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_ALBUM, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_ALBUMARTIST, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_YEAR, kSortNoCase);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_GENRE, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_COMPOSER, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_GROUPING, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_TRACKNUMBER, kSortInt);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_FILETYPE, kSortNoCase);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_COMMENT, kSortNoCaseLex);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_BITRATE, kSortInt);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_BPM, kSortInt);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_SAMPLERATE, kSortInt);
    insertColumnSortByEnum(COLUMN_LIBRARYTABLE_TIMESPLAYED, kSortInt);

    insertColumnSortByEnum(COLUMN_TRACKLOCATIONSTABLE_LOCATION, kSortNoCase);

    slotSetKeySortOrder(m_pKeyNotationCP->get());
}

void ColumnCache::slotSetKeySortOrder(double notationValue) {
    const int keyColumnIndex = m_columnIndexByEnum[COLUMN_LIBRARYTABLE_KEY];
    if (keyColumnIndex < 0) {
        return;
    }

    // A custom COLLATE function was tested, but using CASE ... WHEN was found to be faster
    // see GitHub PR#649
    // https://github.com/mixxxdj/mixxx/pull/649#discussion_r34863809
    const auto notation =
            KeyUtils::keyNotationFromNumericValue(notationValue);
    // The placeholder %1 will be replaced by the "key" column. The actual
    // key code needed for sorting is stored in the column "key_id".
    DEBUG_ASSERT(LIBRARYTABLE_KEY_ID == LIBRARYTABLE_KEY + QStringLiteral("_id"));
    QString keySortSQL = QStringLiteral("CASE %1_id WHEN NULL THEN 0");
    for (int i = 0; i <= 24; ++i) {
        const auto sortOrder = KeyUtils::keyToCircleOfFifthsOrder(
                static_cast<mixxx::track::io::key::ChromaticKey>(i),
                notation);
        keySortSQL +=
                QStringLiteral(" WHEN ") +
                QString::number(i) +
                QStringLiteral(" THEN ") +
                QString::number(sortOrder);
    }
    keySortSQL.append(" END");

    // Replace the existing sort order
    m_columnSortByIndex[keyColumnIndex] = keySortSQL;
}

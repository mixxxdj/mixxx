#include "library/columncache.h"

#include "library/dao/trackschema.h"
#include "library/dao/playlistdao.h"

namespace {

const QString kSortInt = QStringLiteral("cast(%1 as integer)");

const QString kSortNoCase = QStringLiteral("lower(%1)");

} // anonymous namespace

ColumnCache::ColumnCache(const QStringList& columns) {
    m_pKeyNotationCP = new ControlProxy("[Library]", "key_notation", this);
    m_pKeyNotationCP->connectValueChanged(this, &ColumnCache::slotSetKeySortOrder);

    // ColumnCache is initialized before the preferences, so slotSetKeySortOrder is called
    // for again if DlgPrefKey sets the [Library]. key_notation CO to a value other than
    // KeyUtils::CUSTOM as Mixxx is starting.

    setColumns(columns);
}

void ColumnCache::initColumnIndexByEnum(
        Column columnEnum,
        const QString& columnName) {
    // Verify valid and uninitialized
    DEBUG_ASSERT(!columnName.isEmpty());
    DEBUG_ASSERT(m_columnIndexByEnum[columnEnum] == -1);
    const int columnIndex = fieldIndex(columnName);
    // Verify unambiguous
    VERIFY_OR_DEBUG_ASSERT(columnIndex == -1 ||
            std::count(m_columnIndexByEnum,
                    m_columnIndexByEnum +
                            sizeof(m_columnIndexByEnum) /
                                    sizeof(m_columnIndexByEnum[0]),
                    columnIndex) == 0) {
        qCritical() << "ColumnCache: Column name" << columnName << "with index"
                    << columnIndex << "is already mapped to column enum"
                    << (std::find(m_columnIndexByEnum,
                                m_columnIndexByEnum +
                                        sizeof(m_columnIndexByEnum) /
                                                sizeof(m_columnIndexByEnum[0]),
                                columnIndex) -
                               m_columnIndexByEnum)
                    << "and cannot be remapped to column enum" << columnEnum;
        return;
    }
    m_columnIndexByEnum[columnEnum] = columnIndex;
}

void ColumnCache::setColumns(const QStringList& columnNames) {
    m_columnsByIndex = columnNames;

    m_columnIndexByName.clear();
    for (int i = 0; i < columnNames.size(); ++i) {
        const QString& columnName = columnNames[i];
        VERIFY_OR_DEBUG_ASSERT(!columnName.isEmpty()) {
            continue;
        }
        VERIFY_OR_DEBUG_ASSERT(!m_columnIndexByName.contains(columnName)) {
            qCritical()
                    << "ColumnCache: Skipping duplicate column name"
                    << columnName
                    << "column index"
                    << i
                    << "that is used for column index"
                    << m_columnIndexByName[columnName];
            continue;
        }
        m_columnIndexByName[columnName] = i;
    }

    std::fill(
            m_columnIndexByEnum,
            m_columnIndexByEnum + sizeof(m_columnIndexByEnum) / sizeof(m_columnIndexByEnum[0]),
            -1);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_ID, LIBRARYTABLE_ID);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_ARTIST, LIBRARYTABLE_ARTIST);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_TITLE, LIBRARYTABLE_TITLE);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_ALBUM, LIBRARYTABLE_ALBUM);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_ALBUMARTIST, LIBRARYTABLE_ALBUMARTIST);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_YEAR, LIBRARYTABLE_YEAR);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_GENRE, LIBRARYTABLE_GENRE);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_COMPOSER, LIBRARYTABLE_COMPOSER);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_GROUPING, LIBRARYTABLE_GROUPING);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_TRACKNUMBER, LIBRARYTABLE_TRACKNUMBER);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_FILETYPE, LIBRARYTABLE_FILETYPE);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_NATIVELOCATION, LIBRARYTABLE_LOCATION);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_COMMENT, LIBRARYTABLE_COMMENT);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_DURATION, LIBRARYTABLE_DURATION);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_BITRATE, LIBRARYTABLE_BITRATE);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_BPM, LIBRARYTABLE_BPM);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_REPLAYGAIN, LIBRARYTABLE_REPLAYGAIN);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_CUEPOINT, LIBRARYTABLE_CUEPOINT);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_URL, LIBRARYTABLE_URL);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_SAMPLERATE, LIBRARYTABLE_SAMPLERATE);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_WAVESUMMARYHEX, LIBRARYTABLE_WAVESUMMARYHEX);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_CHANNELS, LIBRARYTABLE_CHANNELS);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_MIXXXDELETED, LIBRARYTABLE_MIXXXDELETED);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_DATETIMEADDED, LIBRARYTABLE_DATETIMEADDED);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_HEADERPARSED, LIBRARYTABLE_HEADERPARSED);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_TIMESPLAYED, LIBRARYTABLE_TIMESPLAYED);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_PLAYED, LIBRARYTABLE_PLAYED);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_RATING, LIBRARYTABLE_RATING);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_KEY, LIBRARYTABLE_KEY);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_KEY_ID, LIBRARYTABLE_KEY_ID);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_BPM_LOCK, LIBRARYTABLE_BPM_LOCK);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_PREVIEW, LIBRARYTABLE_PREVIEW);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_COLOR, LIBRARYTABLE_COLOR);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_COVERART, LIBRARYTABLE_COVERART);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_COVERART_SOURCE, LIBRARYTABLE_COVERART_SOURCE);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_COVERART_TYPE, LIBRARYTABLE_COVERART_TYPE);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_COVERART_LOCATION, LIBRARYTABLE_COVERART_LOCATION);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_COVERART_COLOR, LIBRARYTABLE_COVERART_COLOR);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_COVERART_DIGEST, LIBRARYTABLE_COVERART_DIGEST);
    initColumnIndexByEnum(COLUMN_LIBRARYTABLE_COVERART_HASH, LIBRARYTABLE_COVERART_HASH);

    initColumnIndexByEnum(COLUMN_TRACKLOCATIONSTABLE_FSDELETED, TRACKLOCATIONSTABLE_FSDELETED);

    initColumnIndexByEnum(COLUMN_PLAYLISTTRACKSTABLE_TRACKID, PLAYLISTTRACKSTABLE_TRACKID);
    initColumnIndexByEnum(COLUMN_PLAYLISTTRACKSTABLE_POSITION, PLAYLISTTRACKSTABLE_POSITION);
    initColumnIndexByEnum(COLUMN_PLAYLISTTRACKSTABLE_PLAYLISTID, PLAYLISTTRACKSTABLE_PLAYLISTID);
    initColumnIndexByEnum(COLUMN_PLAYLISTTRACKSTABLE_LOCATION, PLAYLISTTRACKSTABLE_LOCATION);
    initColumnIndexByEnum(COLUMN_PLAYLISTTRACKSTABLE_ARTIST, PLAYLISTTRACKSTABLE_ARTIST);
    initColumnIndexByEnum(COLUMN_PLAYLISTTRACKSTABLE_TITLE, PLAYLISTTRACKSTABLE_TITLE);
    initColumnIndexByEnum(COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED,
            PLAYLISTTRACKSTABLE_DATETIMEADDED);

    initColumnIndexByEnum(COLUMN_REKORDBOX_ANALYZE_PATH, REKORDBOX_ANALYZE_PATH);

    m_columnSortByIndex.clear();
    // Add the columns that requires a special sort
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ARTIST], kSortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_TITLE], kSortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ALBUM], kSortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ALBUMARTIST], kSortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_YEAR], kSortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_GENRE], kSortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COMPOSER], kSortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_GROUPING], kSortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_TRACKNUMBER], kSortInt);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_FILETYPE], kSortNoCase);
    m_columnSortByIndex.insert(
            m_columnIndexByEnum[COLUMN_LIBRARYTABLE_NATIVELOCATION],
            kSortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COMMENT], kSortNoCase);

    m_columnSortByIndex.insert(
            m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_LOCATION],
            kSortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_ARTIST], kSortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_TITLE], kSortNoCase);

    slotSetKeySortOrder(m_pKeyNotationCP->get());
}

void ColumnCache::slotSetKeySortOrder(double notationValue) {
    if (m_columnIndexByEnum[COLUMN_LIBRARYTABLE_KEY] < 0) return;

    // A custom COLLATE function was tested, but using CASE ... WHEN was found to be faster
    // see GitHub PR#649
    // https://github.com/mixxxdj/mixxx/pull/649#discussion_r34863809
    KeyUtils::KeyNotation notation =
            KeyUtils::keyNotationFromNumericValue(notationValue);
    QString keySortSQL("CASE %1_id WHEN NULL THEN 0 ");
    for (int i = 0; i <= 24; ++i) {
        keySortSQL.append(QString("WHEN %1 THEN %2 ")
            .arg(QString::number(i),
                 QString::number(KeyUtils::keyToCircleOfFifthsOrder(
                                     static_cast<mixxx::track::io::key::ChromaticKey>(i),
                                     notation))));
    }
    keySortSQL.append("END");

    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_KEY], keySortSQL);
}

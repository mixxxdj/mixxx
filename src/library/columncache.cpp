#include "library/columncache.h"

#include "library/dao/trackschema.h"
#include "library/dao/playlistdao.h"


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
        m_columnIndexByName[column] = i;
    }

    m_columnNameByEnum.clear();
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_ID] = LIBRARYTABLE_ID;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_ARTIST] = LIBRARYTABLE_ARTIST;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_TITLE] = LIBRARYTABLE_TITLE;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_ALBUM] = LIBRARYTABLE_ALBUM;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_ALBUMARTIST] = LIBRARYTABLE_ALBUMARTIST;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_YEAR] = LIBRARYTABLE_YEAR;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_GENRE] = LIBRARYTABLE_GENRE;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_COMPOSER] = LIBRARYTABLE_COMPOSER;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_GROUPING] = LIBRARYTABLE_GROUPING;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_TRACKNUMBER] = LIBRARYTABLE_TRACKNUMBER;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_FILETYPE] = LIBRARYTABLE_FILETYPE;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_NATIVELOCATION] = LIBRARYTABLE_LOCATION;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_COMMENT] = LIBRARYTABLE_COMMENT;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_DURATION] = LIBRARYTABLE_DURATION;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_BITRATE] = LIBRARYTABLE_BITRATE;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_BPM] = LIBRARYTABLE_BPM;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_REPLAYGAIN] = LIBRARYTABLE_REPLAYGAIN;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_CUEPOINT] = LIBRARYTABLE_CUEPOINT;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_URL] = LIBRARYTABLE_URL;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_SAMPLERATE] = LIBRARYTABLE_SAMPLERATE;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_WAVESUMMARYHEX] = LIBRARYTABLE_WAVESUMMARYHEX;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_CHANNELS] = LIBRARYTABLE_CHANNELS;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_MIXXXDELETED] = LIBRARYTABLE_MIXXXDELETED;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_DATETIMEADDED] = LIBRARYTABLE_DATETIMEADDED;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_HEADERPARSED] = LIBRARYTABLE_HEADERPARSED;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_TIMESPLAYED] = LIBRARYTABLE_TIMESPLAYED;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_PLAYED] = LIBRARYTABLE_PLAYED;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_RATING] = LIBRARYTABLE_RATING;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_KEY] = LIBRARYTABLE_KEY;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_KEY_ID] = LIBRARYTABLE_KEY_ID;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_BPM_LOCK] = LIBRARYTABLE_BPM_LOCK;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_PREVIEW] = LIBRARYTABLE_PREVIEW;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_COLOR] = LIBRARYTABLE_COLOR;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_COVERART] = LIBRARYTABLE_COVERART;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_COVERART_SOURCE] = LIBRARYTABLE_COVERART_SOURCE;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_COVERART_TYPE] = LIBRARYTABLE_COVERART_TYPE;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_COVERART_LOCATION] = LIBRARYTABLE_COVERART_LOCATION;
    m_columnNameByEnum[COLUMN_LIBRARYTABLE_COVERART_HASH] = LIBRARYTABLE_COVERART_HASH;

    m_columnNameByEnum[COLUMN_TRACKLOCATIONSTABLE_FSDELETED] = TRACKLOCATIONSTABLE_FSDELETED;

    m_columnNameByEnum[COLUMN_PLAYLISTTRACKSTABLE_TRACKID] = PLAYLISTTRACKSTABLE_TRACKID;
    m_columnNameByEnum[COLUMN_PLAYLISTTRACKSTABLE_POSITION] = PLAYLISTTRACKSTABLE_POSITION;
    m_columnNameByEnum[COLUMN_PLAYLISTTRACKSTABLE_PLAYLISTID] = PLAYLISTTRACKSTABLE_PLAYLISTID;
    m_columnNameByEnum[COLUMN_PLAYLISTTRACKSTABLE_LOCATION] = PLAYLISTTRACKSTABLE_LOCATION;
    m_columnNameByEnum[COLUMN_PLAYLISTTRACKSTABLE_ARTIST] = PLAYLISTTRACKSTABLE_ARTIST;
    m_columnNameByEnum[COLUMN_PLAYLISTTRACKSTABLE_TITLE] = PLAYLISTTRACKSTABLE_TITLE;
    m_columnNameByEnum[COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED] =
            PLAYLISTTRACKSTABLE_DATETIMEADDED;

    m_columnNameByEnum[COLUMN_REKORDBOX_ANALYZE_PATH] = REKORDBOX_ANALYZE_PATH;

    for (int i = 0; i < NUM_COLUMNS; ++i) {
        m_columnIndexByEnum[i] = -1;
    }

    for (auto it = m_columnNameByEnum.constKeyValueBegin();
            it != m_columnNameByEnum.constKeyValueEnd();
            ++it) {
        m_columnIndexByEnum[(*it).first] = fieldIndex((*it).second);
    }

    const QString sortInt("cast(%1 as integer)");
    const QString sortNoCase("lower(%1)");

    m_columnSortByIndex.clear();
    // Add the columns that requires a special sort
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ARTIST], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_TITLE], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ALBUM], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_ALBUMARTIST], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_YEAR], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_GENRE], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COMPOSER], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_GROUPING], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_TRACKNUMBER], sortInt);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_FILETYPE], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_NATIVELOCATION], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_LIBRARYTABLE_COMMENT], sortNoCase);

    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_LOCATION], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_ARTIST], sortNoCase);
    m_columnSortByIndex.insert(m_columnIndexByEnum[COLUMN_PLAYLISTTRACKSTABLE_TITLE], sortNoCase);

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
